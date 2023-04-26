// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimelineViewport.h>

namespace tl
{
    namespace ui
    {
        struct TimelineViewport::Private
        {
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            std::vector<std::shared_ptr<timeline::TimelinePlayer> > timelinePlayers;
            std::vector<imaging::Size> timelineSizes;
            std::vector<imaging::Size> timelineSizesTmp;
            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = true;
            std::function<void(const math::Vector2i&, float)> viewPosAndZoomCallback;
            std::function<void(void)> frameViewCallback;
            bool mouseInside = false;
            bool mousePressed = false;
            math::Vector2i mousePos;
            math::Vector2i mousePress;
            math::Vector2i viewPosMousePress;
            std::vector<timeline::VideoData> videoData;
            std::vector<std::shared_ptr<observer::ValueObserver<timeline::VideoData> > > videoDataObservers;
        };

        void TimelineViewport::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimelineViewport", context, parent);
            setHStretch(Stretch::Expanding);
            setVStretch(Stretch::Expanding);
        }

        TimelineViewport::TimelineViewport() :
            _p(new Private)
        {}

        TimelineViewport::~TimelineViewport()
        {}

        std::shared_ptr<TimelineViewport> TimelineViewport::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineViewport>(new TimelineViewport);
            out->_init(context, parent);
            return out;
        }

        void TimelineViewport::setColorConfigOptions(const timeline::ColorConfigOptions& value)
        {
            TLRENDER_P();
            if (value == p.colorConfigOptions)
                return;
            p.colorConfigOptions = value;
            _updates |= Update::Draw;
        }

        void TimelineViewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            _updates |= Update::Draw;
        }

        void TimelineViewport::setImageOptions(const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            _updates |= Update::Draw;
        }

        void TimelineViewport::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            _updates |= Update::Draw;
        }

        void TimelineViewport::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            _updates |= Update::Draw;
        }

        void TimelineViewport::setTimelinePlayers(const std::vector<std::shared_ptr<timeline::TimelinePlayer> >& value)
        {
            TLRENDER_P();
            p.videoDataObservers.clear();
            p.timelinePlayers = value;
            p.timelineSizesTmp.clear();
            for (const auto& i : p.timelinePlayers)
            {
                const auto& ioInfo = i->getIOInfo();
                if (!ioInfo.video.empty())
                {
                    p.timelineSizesTmp.push_back(ioInfo.video[0].size);
                }
            }
            p.videoData.clear();
            _updates |= Update::Draw;
            for (size_t i = 0; i < p.timelinePlayers.size(); ++i)
            {
                p.videoDataObservers.push_back(
                    observer::ValueObserver<timeline::VideoData>::create(
                        p.timelinePlayers[i]->observeCurrentVideo(),
                        [this, i](const timeline::VideoData& value)
                        {
                            _p->timelineSizes = _p->timelineSizesTmp;
                            if (_p->videoData.size() != _p->timelinePlayers.size())
                            {
                                _p->videoData = std::vector<timeline::VideoData>(_p->timelinePlayers.size());
                            }
                            for (size_t i = 0; i < _p->videoData.size(); ++i)
                            {
                                if (!_p->timelinePlayers[i]->getTimeRange().contains(_p->videoData[i].time))
                                {
                                    _p->videoData[i] = timeline::VideoData();
                                }
                            }
                            _p->videoData[i] = value;
                            _updates |= Update::Draw;
                        }));
            }
        }

        const math::Vector2i& TimelineViewport::viewPos() const
        {
            return _p->viewPos;
        }

        float TimelineViewport::viewZoom() const
        {
            return _p->viewZoom;
        }

        bool TimelineViewport::hasFrameView() const
        {
            return _p->frameView;
        }

        void TimelineViewport::setViewPosAndZoom(const math::Vector2i& pos, float zoom)
        {
            TLRENDER_P();
            if (pos == p.viewPos && zoom == p.viewZoom)
                return;
            p.viewPos = pos;
            p.viewZoom = zoom;
            p.frameView = false;
            _updates |= Update::Draw;
            if (p.viewPosAndZoomCallback)
            {
                p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
            }
        }

        void TimelineViewport::setViewZoom(float zoom, const math::Vector2i& focus)
        {
            TLRENDER_P();
            math::Vector2i pos;
            pos.x = focus.x + (p.viewPos.x - focus.x) * (zoom / p.viewZoom);
            pos.y = focus.y + (p.viewPos.y - focus.y) * (zoom / p.viewZoom);
            setViewPosAndZoom(pos, zoom);
        }

        void TimelineViewport::frameView()
        {
            TLRENDER_P();
            p.frameView = true;
            _updates |= Update::Draw;
        }

        void TimelineViewport::viewZoom1To1()
        {
            TLRENDER_P();
            setViewZoom(1.F, p.mouseInside ? p.mousePos : _viewportCenter());
        }

        void TimelineViewport::viewZoomIn()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom * 2.F, p.mouseInside ? p.mousePos : _viewportCenter());
        }

        void TimelineViewport::viewZoomOut()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom / 2.F, p.mouseInside ? p.mousePos : _viewportCenter());
        }

        void TimelineViewport::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            const int sa = event.style->getSizeRole(SizeRole::ScrollArea);
            _sizeHint.x = sa;
            _sizeHint.y = sa;
        }

        void TimelineViewport::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            if (p.frameView)
            {
                _frameView();
            }

            const math::BBox2i g = _geometry;

            event.render->drawRect(g, imaging::Color4f(0.F, 0.F, 0.F));

            const math::BBox2i viewportPrev = event.render->getViewport();
            const math::Matrix4x4f transformPrev = event.render->getTransform();

            event.render->setViewport(math::BBox2i(0, 0, g.w(), g.h()));
            math::Matrix4x4f vm;
            vm = vm * math::translate(math::Vector3f(p.viewPos.x, p.viewPos.y, 0.F));
            vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));
            const auto pm = math::ortho(
                0.F,
                static_cast<float>(g.w()),
                static_cast<float>(g.h()),
                0.F,
                -1.F,
                1.F);
            event.render->setTransform(pm * vm);

            event.render->drawVideo(
                p.videoData,
                timeline::getBBoxes(p.compareOptions.mode, p.timelineSizes),
                p.imageOptions,
                p.displayOptions,
                p.compareOptions);

            event.render->setViewport(viewportPrev);
            event.render->setTransform(transformPrev);
        }

        imaging::Size TimelineViewport::_renderSize() const
        {
            TLRENDER_P();
            return timeline::getRenderSize(p.compareOptions.mode, p.timelineSizes);
        }

        math::Vector2i TimelineViewport::_viewportCenter() const
        {
            return math::Vector2i(_geometry.w() / 2, _geometry.h() / 2);
        }

        void TimelineViewport::_frameView()
        {
            TLRENDER_P();
            const imaging::Size viewportSize(_geometry.w(), _geometry.h());
            const imaging::Size renderSize = _renderSize();
            float zoom = viewportSize.w / static_cast<float>(renderSize.w);
            if (zoom * renderSize.h > viewportSize.h)
            {
                zoom = viewportSize.h / static_cast<float>(renderSize.h);
            }
            const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
            const math::Vector2i viewPos(
                viewportSize.w / 2.F - c.x * zoom,
                viewportSize.h / 2.F - c.y * zoom);
            if (viewPos != p.viewPos || zoom != p.viewZoom)
            {
                p.viewPos = viewPos;
                p.viewZoom = zoom;
                if (p.viewPosAndZoomCallback)
                {
                    p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
                }
                if (p.frameViewCallback)
                {
                    p.frameViewCallback();
                }
            }
        }
    }
}
