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
            std::function<void(timeline::CompareOptions)> compareCallback;
            std::vector<std::shared_ptr<timeline::Player> > players;
            std::vector<image::Size> timelineSizes;
            math::Vector2i viewPos;
            double viewZoom = 1.0;
            std::shared_ptr<observer::Value<bool> > frameView;
            std::function<void(const math::Vector2i&, double)> viewPosAndZoomCallback;
            
            std::shared_ptr<gl::OffscreenBuffer> buffer;
            bool renderBuffer = false;

            enum class MouseMode
            {
                None,
                View,
                Wipe
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
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
            TLRENDER_P();

            setHStretch(ui::Stretch::Expanding);
            setVStretch(ui::Stretch::Expanding);

            _mouse.hoverEnabled = true;
            _mouse.pressEnabled = true;

            p.frameView = observer::Value<bool>::create(true);
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

        void TimelineViewport::setCompareCallback(const std::function<void(timeline::CompareOptions)>& value)
        {
            _p->compareCallback = value;
        }

        void TimelineViewport::setPlayers(const std::vector<std::shared_ptr<timeline::Player> >& value)
        {
            TLRENDER_P();
            p.videoDataObservers.clear();

            p.players = value;

            p.timelineSizes.clear();
            for (const auto& player : p.players)
            {
                if (player)
                {
                    const auto& ioInfo = player->getIOInfo();
                    if (!ioInfo.video.empty())
                    {
                        p.timelineSizes.push_back(ioInfo.video[0].size);
                    }
                }
            }

            p.videoData.clear();
            p.renderBuffer = true;
            _updates |= ui::Update::Draw;
            for (size_t i = 0; i < p.players.size(); ++i)
            {
                if (p.players[i])
                {
                    p.videoDataObservers.push_back(
                        observer::ValueObserver<timeline::VideoData>::create(
                            p.players[i]->observeCurrentVideo(),
                            [this, i](const timeline::VideoData& value)
                            {
                                _videoDataCallback(value, i);
                            }));
                }
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

        void TimelineViewport::setViewPosAndZoom(const math::Vector2i& pos, double zoom)
        {
            TLRENDER_P();
            if (pos == p.viewPos && zoom == p.viewZoom)
                return;
            p.viewPos = pos;
            p.viewZoom = zoom;
            p.renderBuffer = true;
            _updates |= ui::Update::Draw;
            if (p.viewPosAndZoomCallback)
            {
                p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
            }
            setFrameView(false);
        }

        void TimelineViewport::setViewZoom(double zoom, const math::Vector2i& focus)
        {
            TLRENDER_P();
            math::Vector2i pos;
            pos.x = focus.x + (p.viewPos.x - focus.x) * (zoom / p.viewZoom);
            pos.y = focus.y + (p.viewPos.y - focus.y) * (zoom / p.viewZoom);
            setViewPosAndZoom(pos, zoom);
        }

        bool TimelineViewport::hasFrameView() const
        {
            return _p->frameView->get();
        }

        std::shared_ptr<observer::IValue<bool> > TimelineViewport::observeFrameView() const
        {
            return _p->frameView;
        }

        void TimelineViewport::setFrameView(bool value)
        {
            TLRENDER_P();
            if (p.frameView->setIfChanged(value))
            {
                p.renderBuffer = true;
                _updates |= ui::Update::Draw;
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
            setViewZoom(p.viewZoom * 2.0, _viewportCenter());
        }

        void TimelineViewport::viewZoomOut()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom / 2.0, _viewportCenter());
        }

        void TimelineViewport::setViewPosAndZoomCallback(
            const std::function<void(const math::Vector2i&, double)>& value)
        {
            _p->viewPosAndZoomCallback = value;
        }

        void TimelineViewport::setGeometry(const math::Box2i& value)
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

        void TimelineViewport::drawEvent(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            if (p.frameView->get())
            {
                _frameView();
            }

            const math::Box2i& g = _geometry;

            event.render->drawRect(g, image::Color4f(0.F, 0.F, 0.F));

            if (p.renderBuffer)
            {
                p.renderBuffer = false;

                const timeline::ViewportState viewportState(event.render);
                const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
                const timeline::ClipRectState clipRectState(event.render);
                const timeline::TransformState transformState(event.render);
                const timeline::RenderSizeState renderSizeState(event.render);

                const image::Size size(g.w(), g.h());
                gl::OffscreenBufferOptions options;
                options.colorType = image::PixelType::RGB_F32;
                options.depth = gl::OffscreenDepth::_24;
                options.stencil = gl::OffscreenStencil::_8;
                if (gl::doCreate(p.buffer, size, options))
                {
                    p.buffer = gl::OffscreenBuffer::create(size, options);
                }
                if (p.buffer)
                {
                    gl::OffscreenBufferBinding binding(p.buffer);
                    event.render->setRenderSize(size);
                    event.render->setViewport(math::Box2i(0, 0, g.w(), g.h()));
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
                    event.render->clearViewport(image::Color4f(0.F, 0.F, 0.F));
                    event.render->drawVideo(
                        p.videoData,
                        timeline::getBoxes(p.compareOptions.mode, p.timelineSizes),
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
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::View:
                p.viewPos.x = p.mouse.viewPos.x + (event.pos.x - _mouse.pressPos.x);
                p.viewPos.y = p.mouse.viewPos.y + (event.pos.y - _mouse.pressPos.y);
                p.renderBuffer = true;
                _updates |= ui::Update::Draw;
                if (p.viewPosAndZoomCallback)
                {
                    p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
                }
                setFrameView(false);
                break;
            case Private::MouseMode::Wipe:
            {
                if (!p.players.empty() && p.players[0])
                {
                    const auto& ioInfo = p.players[0]->getIOInfo();
                    if (!ioInfo.video.empty())
                    {
                        const auto& imageInfo = ioInfo.video[0];
                        p.compareOptions.wipeCenter.x = (event.pos.x - _geometry.min.x - p.viewPos.x) / p.viewZoom /
                            static_cast<float>(imageInfo.size.w * imageInfo.size.pixelAspectRatio);
                        p.compareOptions.wipeCenter.y = (event.pos.y - _geometry.min.y - p.viewPos.y) / p.viewZoom /
                            static_cast<float>(imageInfo.size.h);
                        p.renderBuffer = true;
                        _updates |= ui::Update::Draw;
                        if (p.compareCallback)
                        {
                            p.compareCallback(p.compareOptions);
                        }
                    }
                }
                break;
            }
            default: break;
            }
        }

        void TimelineViewport::mousePressEvent(ui::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            takeKeyFocus();
            if (0 == event.button &&
                event.modifiers & static_cast<int>(ui::KeyModifier::Control))
            {
                p.mouse.mode = Private::MouseMode::View;
                p.mouse.viewPos = p.viewPos;
            }
            else if (0 == event.button &&
                event.modifiers & static_cast<int>(ui::KeyModifier::Alt))
            {
                p.mouse.mode = Private::MouseMode::Wipe;
            }
        }

        void TimelineViewport::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            TLRENDER_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineViewport::scrollEvent(ui::ScrollEvent& event)
        {
            TLRENDER_P();
            if (static_cast<int>(ui::KeyModifier::None) == event.modifiers)
            {
                event.accept = true;
                const double mult = 1.1;
                const double zoom =
                    event.dy < 0 ?
                    p.viewZoom / (-event.dy * mult) :
                    p.viewZoom * (event.dy * mult);
                setViewZoom(zoom, event.pos - _geometry.min);
            }
            else if (event.modifiers & static_cast<int>(ui::KeyModifier::Control))
            {
                event.accept = true;
                if (!p.players.empty() && p.players[0])
                {
                    const otime::RationalTime t = p.players[0]->getCurrentTime();
                    p.players[0]->seek(t + otime::RationalTime(event.dy, t.rate()));
                }
            }
        }

        void TimelineViewport::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            if (0 == event.modifiers)
            {
                switch (event.key)
                {
                case ui::Key::_0:
                    event.accept = true;
                    setViewZoom(1.0, event.pos - _geometry.min);
                    break;
                case ui::Key::Equal:
                    event.accept = true;
                    setViewZoom(p.viewZoom * 2.0, event.pos - _geometry.min);
                    break;
                case ui::Key::Minus:
                    event.accept = true;
                    setViewZoom(p.viewZoom / 2.0, event.pos - _geometry.min);
                    break;
                case ui::Key::Backspace:
                    event.accept = true;
                    setFrameView(true);
                default: break;
                }
            }
        }

        void TimelineViewport::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
        }

        void TimelineViewport::_releaseMouse()
        {
            IWidget::_releaseMouse();
            TLRENDER_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        image::Size TimelineViewport::_renderSize() const
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
            const image::Size viewportSize(_geometry.w(), _geometry.h());
            const image::Size renderSize = _renderSize();
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

        void TimelineViewport::_videoDataCallback(const timeline::VideoData& value, size_t index)
        {
            TLRENDER_P();
            if (p.videoData.size() != p.players.size())
            {
                p.videoData = std::vector<timeline::VideoData>(p.players.size());
            }
            for (size_t i = 0; i < p.videoData.size(); ++i)
            {
                if (!p.players[i]->getTimeRange().contains(p.videoData[i].time))
                {
                    p.videoData[i] = timeline::VideoData();
                }
            }
            p.videoData[index] = value;
            p.renderBuffer = true;
            _updates |= ui::Update::Draw;
        }
    }
}
