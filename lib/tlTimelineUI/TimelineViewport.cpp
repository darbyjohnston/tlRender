// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineViewport.h>

#include <tlGL/OffscreenBuffer.h>

#include <tlTimeline/RenderUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct TimelineViewport::Private
        {
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            std::vector<std::shared_ptr<timeline::Player> > players;
            std::vector<imaging::Size> timelineSizes;
            math::Vector2i viewPos;
            double viewZoom = 1.0;
            bool frameView = true;
            std::function<void(const math::Vector2i&, double)> viewPosAndZoomCallback;
            std::function<void(bool)> frameViewCallback;
            
            std::shared_ptr<gl::OffscreenBuffer> buffer;
            bool renderBuffer = false;

            struct MouseData
            {
                bool pressed = false;
                math::Vector2i pressPos;
                math::Vector2i viewPos;
            };
            MouseData mouse;

            std::vector<timeline::VideoData> videoData;
            std::vector<std::shared_ptr<observer::ValueObserver<timeline::VideoData> > > videoDataObservers;
        };

        void TimelineViewport::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimelineViewport", context, parent);
            setHStretch(ui::Stretch::Expanding);
            setVStretch(ui::Stretch::Expanding);
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
            p.renderBuffer = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            p.renderBuffer = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setImageOptions(const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            p.renderBuffer = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            p.renderBuffer = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            p.renderBuffer = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setPlayers(const std::vector<std::shared_ptr<timeline::Player> >& value)
        {
            TLRENDER_P();
            p.videoDataObservers.clear();

            p.players = value;

            p.timelineSizes.clear();
            for (const auto& i : p.players)
            {
                const auto& ioInfo = i->getIOInfo();
                if (!ioInfo.video.empty())
                {
                    p.timelineSizes.push_back(ioInfo.video[0].size);
                }
            }

            p.videoData.clear();
            p.renderBuffer = true;
            _updates |= ui::Update::Draw;
            for (size_t i = 0; i < p.players.size(); ++i)
            {
                p.videoDataObservers.push_back(
                    observer::ValueObserver<timeline::VideoData>::create(
                        p.players[i]->observeCurrentVideo(),
                        [this, i](const timeline::VideoData& value)
                        {
                            if (_p->videoData.size() != _p->players.size())
                            {
                                _p->videoData = std::vector<timeline::VideoData>(_p->players.size());
                            }
                            for (size_t i = 0; i < _p->videoData.size(); ++i)
                            {
                                if (!_p->players[i]->getTimeRange().contains(_p->videoData[i].time))
                                {
                                    _p->videoData[i] = timeline::VideoData();
                                }
                            }
                            _p->videoData[i] = value;
                            _p->renderBuffer = true;
                            _updates |= ui::Update::Draw;
                        }));
            }
        }

        const math::Vector2i& TimelineViewport::viewPos() const
        {
            return _p->viewPos;
        }

        double TimelineViewport::viewZoom() const
        {
            return _p->viewZoom;
        }

        bool TimelineViewport::hasFrameView() const
        {
            return _p->frameView;
        }

        void TimelineViewport::setViewPosAndZoom(const math::Vector2i& pos, double zoom)
        {
            TLRENDER_P();
            if (pos == p.viewPos && zoom == p.viewZoom)
                return;
            p.viewPos = pos;
            p.viewZoom = zoom;
            p.frameView = false;
            p.renderBuffer = true;
            _updates |= ui::Update::Draw;
            if (p.viewPosAndZoomCallback)
            {
                p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
            }
            if (p.frameViewCallback)
            {
                p.frameViewCallback(p.frameView);
            }
        }

        void TimelineViewport::setViewZoom(double zoom, const math::Vector2i& focus)
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
            if (!p.frameView)
            {
                p.frameView = true;
                p.renderBuffer = true;
                _updates |= ui::Update::Draw;
                if (p.frameViewCallback)
                {
                    p.frameViewCallback(p.frameView);
                }
            }
        }

        void TimelineViewport::viewZoom1To1()
        {
            TLRENDER_P();
            setViewZoom(1.F, _viewportCenter());
        }

        void TimelineViewport::viewZoomIn()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom * 2.F, _viewportCenter());
        }

        void TimelineViewport::viewZoomOut()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom / 2.F, _viewportCenter());
        }

        void TimelineViewport::setVisible(bool value)
        {
            const bool changed = value != _visible;
            IWidget::setVisible(value);
            if (changed && !_visible)
            {
                _resetMouse();
            }
        }

        void TimelineViewport::setEnabled(bool value)
        {
            const bool changed = value != _enabled;
            IWidget::setEnabled(value);
            if (changed && !_enabled)
            {
                _resetMouse();
            }
        }

        void TimelineViewport::setGeometry(const math::BBox2i& value)
        {
            const bool changed = value != _geometry;
            IWidget::setGeometry(value);
            TLRENDER_P();
            if (changed)
            {
                p.renderBuffer = true;
            }
        }

        void TimelineViewport::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            const int sa = event.style->getSizeRole(ui::SizeRole::ScrollArea, event.displayScale);
            _sizeHint.x = sa;
            _sizeHint.y = sa;
        }

        void TimelineViewport::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ui::ClipEvent& event)
        {
            const bool changed = clipped != _clipped;
            IWidget::clipEvent(clipRect, clipped, event);
            if (changed && clipped)
            {
                _resetMouse();
            }
        }

        void TimelineViewport::drawEvent(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            if (p.frameView)
            {
                _frameView();
            }

            const math::BBox2i& g = _geometry;

            event.render->drawRect(g, imaging::Color4f(0.F, 0.F, 0.F));

            if (p.renderBuffer &&
                !p.videoData.empty())
            {
                p.renderBuffer = false;

                const timeline::ViewportState viewportState(event.render);
                const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
                const timeline::ClipRectState clipRectState(event.render);
                const timeline::TransformState transformState(event.render);
                const timeline::RenderSizeState renderSizeState(event.render);

                const imaging::Size size(g.w(), g.h());
                gl::OffscreenBufferOptions options;
                options.colorType = imaging::PixelType::RGB_F32;
                if (gl::doCreate(p.buffer, size, options))
                {
                    p.buffer = gl::OffscreenBuffer::create(size, options);
                }
                if (p.buffer)
                {
                    gl::OffscreenBufferBinding binding(p.buffer);
                    event.render->setRenderSize(size);
                    event.render->setViewport(math::BBox2i(0, 0, g.w(), g.h()));
                    event.render->setClipRectEnabled(false);
                    math::Matrix4x4f vm;
                    vm = vm * math::translate(math::Vector3f(p.viewPos.x, p.viewPos.y, 0.F));
                    vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));
                    const auto pm = math::ortho(
                        0.F,
                        static_cast<float>(g.w()),
                        0.F,
                        static_cast<float>(g.h()),
                        -1.F,
                        1.F);
                    event.render->setTransform(pm * vm);
                    event.render->clearViewport(imaging::Color4f(0.F, 0.F, 0.F));
                    event.render->drawVideo(
                        p.videoData,
                        timeline::getBBoxes(p.compareOptions.mode, p.timelineSizes),
                        p.imageOptions,
                        p.displayOptions,
                        p.compareOptions);
                }
            }

            if (p.buffer)
            {
                const unsigned int id = p.buffer->getColorID();
                event.render->drawTexture(id, g);
            }
        }

        void TimelineViewport::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            if (p.mouse.pressed)
            {
                p.viewPos.x = p.mouse.viewPos.x + (event.pos.x - p.mouse.pressPos.x);
                p.viewPos.y = p.mouse.viewPos.y + (event.pos.y - p.mouse.pressPos.y);
                p.frameView = false;
                p.renderBuffer = true;
                _updates |= ui::Update::Draw;
                if (p.viewPosAndZoomCallback)
                {
                    p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
                }
                if (p.frameViewCallback)
                {
                    p.frameViewCallback(p.frameView);
                }
            }
        }

        void TimelineViewport::mousePressEvent(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            takeKeyFocus();
            if (0 == event.button &&
                event.modifiers & static_cast<int>(ui::KeyModifier::Control))
            {
                p.mouse.pressed = true;
                p.mouse.pressPos = event.pos;
                p.mouse.viewPos = p.viewPos;
            }
        }

        void TimelineViewport::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = false;
        }

        void TimelineViewport::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            switch (event.key)
            {
            case ui::Key::_0:
                event.accept = true;
                setViewZoom(1.F, event.pos);
                break;
            case ui::Key::Equal:
                event.accept = true;
                setViewZoom(p.viewZoom * 2.F, event.pos);
                break;
            case ui::Key::Minus:
                event.accept = true;
                setViewZoom(p.viewZoom / 2.F, event.pos);
                break;
            case ui::Key::Backspace:
                event.accept = true;
                frameView();
            default: break;
            }
        }

        void TimelineViewport::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
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
            double zoom = viewportSize.w / static_cast<double>(renderSize.w);
            if (zoom * renderSize.h > viewportSize.h)
            {
                zoom = viewportSize.h / static_cast<double>(renderSize.h);
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
            }
        }

        void TimelineViewport::_resetMouse()
        {
            TLRENDER_P();
            p.mouse.pressed = false;
        }
    }
}
