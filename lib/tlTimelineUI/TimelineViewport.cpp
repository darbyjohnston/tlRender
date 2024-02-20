// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineViewport.h>

#include <tlUI/DrawUtil.h>

#include <tlTimeline/RenderUtil.h>

#include <tlGL/OffscreenBuffer.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace timelineui
    {
        struct TimelineViewport::Private
        {
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            timeline::RenderOptions renderOptions;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            std::function<void(timeline::CompareOptions)> compareCallback;
            timeline::BackgroundOptions backgroundOptions;
            std::vector<std::shared_ptr<timeline::Player> > players;
            std::vector<image::Size> timelineSizes;
            std::vector<timeline::VideoData> videoData;
            math::Vector2i viewPos;
            double viewZoom = 1.0;
            std::shared_ptr<observer::Value<bool> > frameView;
            std::function<void(bool)> frameViewCallback;
            std::function<void(const math::Vector2i&, double)> viewPosAndZoomCallback;

            struct DroppedFrames
            {
                bool init = true;
                double frame = 0.0;
                size_t count = 0;
                std::function<void(size_t)> callback;
            };
            DroppedFrames droppedFrames;
            
            bool doRender = false;
            std::shared_ptr<gl::OffscreenBuffer> buffer;

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

            std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
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

            _setMouseHover(true);
            _setMousePress(true);

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

        void TimelineViewport::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            TLRENDER_P();
            if (value == p.ocioOptions)
                return;
            p.ocioOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setImageOptions(const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setCompareCallback(const std::function<void(timeline::CompareOptions)>& value)
        {
            _p->compareCallback = value;
        }

        void TimelineViewport::setBackgroundOptions(const timeline::BackgroundOptions& value)
        {
            TLRENDER_P();
            if (value == p.backgroundOptions)
                return;
            p.backgroundOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setPlayers(const std::vector<std::shared_ptr<timeline::Player> >& value)
        {
            TLRENDER_P();

            p.playbackObserver.reset();
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
            p.doRender = true;
            _updates |= ui::Update::Draw;
            for (size_t i = 0; i < p.players.size(); ++i)
            {
                if (p.players[i])
                {
                    if (0 == i)
                    {
                        p.playbackObserver = observer::ValueObserver<timeline::Playback>::create(
                            p.players[i]->observePlayback(),
                            [this](timeline::Playback value)
                            {
                                switch (value)
                                {
                                case timeline::Playback::Forward:
                                case timeline::Playback::Reverse:
                                    _p->droppedFrames.init = true;
                                    break;
                                default: break;
                                }
                            });
                    }
                    p.videoDataObservers.push_back(
                        observer::ValueObserver<timeline::VideoData>::create(
                            p.players[i]->observeCurrentVideo(),
                            [this, i](const timeline::VideoData& value)
                            {
                                _videoDataUpdate(value, i);
                            }));
                }
            }
        }

        const math::Vector2i& TimelineViewport::getViewPos() const
        {
            return _p->viewPos;
        }

        double TimelineViewport::getViewZoom() const
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
            p.doRender = true;
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
                if (p.frameViewCallback)
                {
                    p.frameViewCallback(value);
                }
                p.doRender = true;
                _updates |= ui::Update::Draw;
            }
        }

        void TimelineViewport::setFrameViewCallback(const std::function<void(bool)>& value)
        {
            _p->frameViewCallback = value;
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
                p.doRender = true;
            }
        }

        void TimelineViewport::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            const int sa = event.style->getSizeRole(ui::SizeRole::ScrollArea, _displayScale);
            _sizeHint.w = sa;
            _sizeHint.h = sa;
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
            if (p.doRender)
            {
                p.doRender = false;

                const timeline::ViewportState viewportState(event.render);
                const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
                const timeline::ClipRectState clipRectState(event.render);
                const timeline::TransformState transformState(event.render);
                const timeline::RenderSizeState renderSizeState(event.render);

                const math::Size2i size = g.getSize();
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = gl::offscreenColorDefault;
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = p.displayOptions[0].imageFilters;
                }
#if defined(TLRENDER_API_GL_4_1)
                offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
#elif defined(TLRENDER_API_GLES_2)
                offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
#endif // TLRENDER_API_GL_4_1
                if (gl::doCreate(p.buffer, size, offscreenBufferOptions))
                {
                    p.buffer = gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                }
                if (p.buffer)
                {
                    gl::OffscreenBufferBinding binding(p.buffer);
                    event.render->setRenderSize(size);
                    event.render->setViewport(math::Box2i(0, 0, g.w(), g.h()));
                    event.render->setClipRectEnabled(false);
                    event.render->clearViewport(image::Color4f(0.F, 0.F, 0.F));
                    event.render->setOCIOOptions(p.ocioOptions);
                    event.render->setLUTOptions(p.lutOptions);
                    if (!p.videoData.empty())
                    {
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
                        event.render->drawVideo(
                            p.videoData,
                            timeline::getBoxes(p.compareOptions.mode, p.timelineSizes),
                            p.imageOptions,
                            p.displayOptions,
                            p.compareOptions,
                            p.backgroundOptions);

                        _droppedFramesUpdate(p.videoData[0].time);
                    }
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
                p.doRender = true;
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
                        p.doRender = true;
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
                    event.value.y < 0 ?
                    p.viewZoom / (-event.value.y * mult) :
                    p.viewZoom * (event.value.y * mult);
                setViewZoom(zoom, event.pos - _geometry.min);
            }
            else if (event.modifiers & static_cast<int>(ui::KeyModifier::Control))
            {
                event.accept = true;
                if (!p.players.empty() && p.players[0])
                {
                    const otime::RationalTime t = p.players[0]->getCurrentTime();
                    p.players[0]->seek(t + otime::RationalTime(event.value.y, t.rate()));
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
                    break;
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

        math::Size2i TimelineViewport::_renderSize() const
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
            const math::Size2i viewportSize(_geometry.w(), _geometry.h());
            const math::Size2i renderSize = _renderSize();
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

        void TimelineViewport::_droppedFramesUpdate(const otime::RationalTime& value)
        {
            TLRENDER_P();
            if (value != time::invalidTime && p.droppedFrames.init)
            {
                p.droppedFrames.init = false;
                p.droppedFrames.count = 0;
                if (p.droppedFrames.callback)
                {
                    p.droppedFrames.callback(p.droppedFrames.count);
                }
            }
            else
            {
                const double frameDiff = value.value() - p.droppedFrames.frame;
                if (std::abs(frameDiff) > 1.0)
                {
                    ++p.droppedFrames.count;
                    if (p.droppedFrames.callback)
                    {
                        p.droppedFrames.callback(p.droppedFrames.count);
                    }
                }
            }
            p.droppedFrames.frame = value.value();
        }

        void TimelineViewport::_videoDataUpdate(const timeline::VideoData& value, size_t index)
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
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }
    }
}
