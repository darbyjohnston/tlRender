// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/Viewport.h>

#include <tlTimeline/IRender.h>

#include <dtk/ui/DrawUtil.h>
#include <dtk/gl/GL.h>
#include <dtk/gl/OffscreenBuffer.h>
#include <dtk/gl/Util.h>
#include <dtk/core/Context.h>
#include <dtk/core/LogSystem.h>
#include <dtk/core/RenderUtil.h>

#include <cmath>

namespace tl
{
    namespace timelineui
    {
        struct Viewport::Private
        {
            timeline::CompareOptions compareOptions;
            std::function<void(timeline::CompareOptions)> compareCallback;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            std::vector<dtk::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::BackgroundOptions bgOptions;
            timeline::ForegroundOptions fgOptions;
            std::shared_ptr<dtk::ObservableValue<dtk::ImageType> > colorBuffer;
            std::shared_ptr<timeline::Player> player;
            std::vector<timeline::VideoData> videoData;
            dtk::V2I viewPos;
            double viewZoom = 1.0;
            std::shared_ptr<dtk::ObservableValue<bool> > frameView;
            std::function<void(bool)> frameViewCallback;
            std::function<void(const dtk::V2I&, double)> viewPosAndZoomCallback;
            std::shared_ptr<dtk::ObservableValue<double> > fps;
            struct FpsData
            {
                std::chrono::steady_clock::time_point timer;
                size_t frameCount = 0;
            };
            FpsData fpsData;
            std::shared_ptr<dtk::ObservableValue<size_t> > droppedFrames;
            struct DroppedFramesData
            {
                bool init = true;
                double frame = 0.0;
            };
            DroppedFramesData droppedFramesData;
            dtk::KeyModifier panModifier = dtk::KeyModifier::Control;
            dtk::KeyModifier wipeModifier = dtk::KeyModifier::Alt;

            bool doRender = false;
            std::shared_ptr<dtk::gl::OffscreenBuffer> buffer;
            std::shared_ptr<dtk::gl::OffscreenBuffer> bgBuffer;
            std::shared_ptr<dtk::gl::OffscreenBuffer> fgBuffer;

            enum class MouseMode
            {
                None,
                View,
                Wipe
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                dtk::V2I viewPos;
            };
            MouseData mouse;

            std::shared_ptr<dtk::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<dtk::ListObserver<timeline::VideoData> > videoDataObserver;
        };

        void Viewport::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::timelineui::Viewport", parent);
            DTK_P();

            setHStretch(dtk::Stretch::Expanding);
            setVStretch(dtk::Stretch::Expanding);

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true);

            p.colorBuffer = dtk::ObservableValue<dtk::ImageType>::create(dtk::ImageType::RGBA_U8);
            p.frameView = dtk::ObservableValue<bool>::create(true);
            p.fps = dtk::ObservableValue<double>::create(0.0);
            p.droppedFrames = dtk::ObservableValue<size_t>::create(0);
        }

        Viewport::Viewport() :
            _p(new Private)
        {}

        Viewport::~Viewport()
        {}

        std::shared_ptr<Viewport> Viewport::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Viewport>(new Viewport);
            out->_init(context, parent);
            return out;
        }

        void Viewport::setCompareOptions(const timeline::CompareOptions& value)
        {
            DTK_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            p.doRender = true;
            _setDrawUpdate();
        }

        void Viewport::setCompareCallback(const std::function<void(timeline::CompareOptions)>& value)
        {
            _p->compareCallback = value;
        }

        void Viewport::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            DTK_P();
            if (value == p.ocioOptions)
                return;
            p.ocioOptions = value;
            p.doRender = true;
            _setDrawUpdate();
        }

        void Viewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            DTK_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            p.doRender = true;
            _setDrawUpdate();
        }

        void Viewport::setImageOptions(const std::vector<dtk::ImageOptions>& value)
        {
            DTK_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            p.doRender = true;
            _setDrawUpdate();
        }

        void Viewport::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            DTK_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            p.doRender = true;
            _setDrawUpdate();
        }

        void Viewport::setBackgroundOptions(const timeline::BackgroundOptions& value)
        {
            DTK_P();
            if (value == p.bgOptions)
                return;
            p.bgOptions = value;
            p.doRender = true;
            _setDrawUpdate();
        }

        void Viewport::setForegroundOptions(const timeline::ForegroundOptions& value)
        {
            DTK_P();
            if (value == p.fgOptions)
                return;
            p.fgOptions = value;
            p.doRender = true;
            _setDrawUpdate();
        }

        dtk::ImageType Viewport::getColorBuffer() const
        {
            return _p->colorBuffer->get();
        }

        std::shared_ptr<dtk::IObservableValue<dtk::ImageType> > Viewport::observeColorBuffer() const
        {
            return _p->colorBuffer;
        }

        void Viewport::setColorBuffer(dtk::ImageType value)
        {
            DTK_P();
            if (p.colorBuffer->setIfChanged(value))
            {
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        const std::shared_ptr<timeline::Player>& Viewport::getPlayer() const
        {
            return _p->player;
        }

        void Viewport::setPlayer(const std::shared_ptr<timeline::Player>& value)
        {
            DTK_P();

            p.fpsData.timer = std::chrono::steady_clock::now();
            p.fpsData.frameCount = 0;
            p.fps->setIfChanged(0.0);
            p.droppedFramesData.init = true;
            p.droppedFrames->setIfChanged(0);
            p.playbackObserver.reset();
            p.videoDataObserver.reset();

            p.player = value;

            if (p.player)
            {
                p.playbackObserver = dtk::ValueObserver<timeline::Playback>::create(
                    p.player->observePlayback(),
                    [this](timeline::Playback value)
                    {
                        switch (value)
                        {
                        case timeline::Playback::Forward:
                        case timeline::Playback::Reverse:
                            _p->fpsData.timer = std::chrono::steady_clock::now();
                            _p->fpsData.frameCount = 0;
                            _p->droppedFramesData.init = true;
                            break;
                        default: break;
                        }
                    });
                p.videoDataObserver = dtk::ListObserver<timeline::VideoData>::create(
                    p.player->observeCurrentVideo(),
                    [this](const std::vector<timeline::VideoData>& value)
                    {
                        _p->videoData = value;

                        _p->fpsData.frameCount = _p->fpsData.frameCount + 1;
                        const auto now = std::chrono::steady_clock::now();
                        const std::chrono::duration<double> diff = now - _p->fpsData.timer;
                        if (diff.count() > 1.0)
                        {
                            const double fps = _p->fpsData.frameCount / diff.count();
                            //std::cout << "FPS: " << fps << std::endl;
                            _p->fps->setIfChanged(fps);
                            _p->fpsData.timer = now;
                            _p->fpsData.frameCount = 0;
                        }

                        _p->doRender = true;
                        _setDrawUpdate();
                    });
            }
            else if (!p.videoData.empty())
            {
                p.videoData.clear();
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        const dtk::V2I& Viewport::getViewPos() const
        {
            return _p->viewPos;
        }

        double Viewport::getViewZoom() const
        {
            return _p->viewZoom;
        }

        void Viewport::setViewPosAndZoom(const dtk::V2I& pos, double zoom)
        {
            DTK_P();
            if (pos == p.viewPos && zoom == p.viewZoom)
                return;
            p.viewPos = pos;
            p.viewZoom = zoom;
            p.doRender = true;
            _setDrawUpdate();
            if (p.viewPosAndZoomCallback)
            {
                p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
            }
            setFrameView(false);
        }

        void Viewport::setViewZoom(double zoom, const dtk::V2I& focus)
        {
            DTK_P();
            dtk::V2I pos;
            pos.x = focus.x + (p.viewPos.x - focus.x) * (zoom / p.viewZoom);
            pos.y = focus.y + (p.viewPos.y - focus.y) * (zoom / p.viewZoom);
            setViewPosAndZoom(pos, zoom);
        }

        bool Viewport::hasFrameView() const
        {
            return _p->frameView->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > Viewport::observeFrameView() const
        {
            return _p->frameView;
        }

        void Viewport::setFrameView(bool value)
        {
            DTK_P();
            if (p.frameView->setIfChanged(value))
            {
                if (p.frameViewCallback)
                {
                    p.frameViewCallback(value);
                }
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        void Viewport::setFrameViewCallback(const std::function<void(bool)>& value)
        {
            _p->frameViewCallback = value;
        }

        void Viewport::viewZoomReset()
        {
            DTK_P();
            setViewZoom(1.F, _getViewportCenter());
        }

        void Viewport::viewZoomIn()
        {
            DTK_P();
            setViewZoom(p.viewZoom * 2.0, _getViewportCenter());
        }

        void Viewport::viewZoomOut()
        {
            DTK_P();
            setViewZoom(p.viewZoom / 2.0, _getViewportCenter());
        }

        void Viewport::setViewPosAndZoomCallback(
            const std::function<void(const dtk::V2I&, double)>& value)
        {
            _p->viewPosAndZoomCallback = value;
        }

        double Viewport::getFPS() const
        {
            return _p->fps->get();
        }

        std::shared_ptr<dtk::IObservableValue<double> > Viewport::observeFPS() const
        {
            return _p->fps;
        }

        size_t Viewport::getDroppedFrames() const
        {
            return _p->droppedFrames->get();
        }

        std::shared_ptr<dtk::IObservableValue<size_t> > Viewport::observeDroppedFrames() const
        {
            return _p->droppedFrames;
        }

        dtk::Color4F Viewport::getColorSample(const dtk::V2I& value)
        {
            DTK_P();
            dtk::Color4F out;
            if (p.buffer)
            {
                const dtk::Box2I& g = getGeometry();
                const dtk::V2I pos = value - g.min;
                std::vector<float> sample(4);
                dtk::gl::OffscreenBufferBinding binding(p.buffer);
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
                glReadPixels(
                    pos.x,
                    pos.y,
                    1,
                    1,
                    GL_RGBA,
                    GL_FLOAT,
                    sample.data());
                out.r = std::isnan(sample[0]) || std::isinf(sample[0]) ? 0.F : sample[0];
                out.g = std::isnan(sample[1]) || std::isinf(sample[1]) ? 0.F : sample[1];
                out.b = std::isnan(sample[2]) || std::isinf(sample[2]) ? 0.F : sample[2];
                out.a = std::isnan(sample[3]) || std::isinf(sample[3]) ? 0.F : sample[3];
            }
            return out;
        }

        void Viewport::setPanModifier(dtk::KeyModifier value)
        {
            _p->panModifier = value;
        }

        void Viewport::setWipeModifier(dtk::KeyModifier value)
        {
            _p->wipeModifier = value;
        }

        void Viewport::setGeometry(const dtk::Box2I& value)
        {
            const bool changed = value != getGeometry();
            IWidget::setGeometry(value);
            DTK_P();
            if (changed)
            {
                p.doRender = true;
            }
        }

        void Viewport::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            const int sa = event.style->getSizeRole(dtk::SizeRole::ScrollArea, event.displayScale);
            _setSizeHint(dtk::Size2I(sa, sa));
        }

        void Viewport::drawEvent(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            DTK_P();

            if (p.frameView->get())
            {
                _frameView();
            }

            auto render = std::dynamic_pointer_cast<timeline::IRender>(event.render);
            const dtk::Box2I& g = getGeometry();
            render->drawRect(g, dtk::Color4F(0.F, 0.F, 0.F));

            if (p.doRender)
            {
                p.doRender = false;
                try
                {
                    // Create the background and foreground buffers.
                    const dtk::Size2I size = g.size();
                    dtk::gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.color = dtk::ImageType::RGBA_U8;
                    offscreenBufferOptions.colorFilters.minify = dtk::ImageFilter::Nearest;
                    offscreenBufferOptions.colorFilters.magnify = dtk::ImageFilter::Nearest;
                    if (dtk::gl::doCreate(p.bgBuffer, size, offscreenBufferOptions))
                    {
                        p.bgBuffer = dtk::gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                    }
                    if (dtk::gl::doCreate(p.fgBuffer, size, offscreenBufferOptions))
                    {
                        p.fgBuffer = dtk::gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                    }

                    // Create the main buffer.
                    offscreenBufferOptions.colorFilters.minify = dtk::ImageFilter::Linear;
                    offscreenBufferOptions.colorFilters.magnify = dtk::ImageFilter::Linear;
                    offscreenBufferOptions.color = p.colorBuffer->get();
                    if (!p.displayOptions.empty())
                    {
                        offscreenBufferOptions.colorFilters = p.displayOptions[0].imageFilters;
                    }
#if defined(dtk_API_GL_4_1)
                    offscreenBufferOptions.depth = dtk::gl::OffscreenDepth::_24;
                    offscreenBufferOptions.stencil = dtk::gl::OffscreenStencil::_8;
#elif defined(dtk_API_GLES_2)
                    offscreenBufferOptions.stencil = dtk::gl::OffscreenStencil::_8;
#endif // dtk_API_GL_4_1
                    if (dtk::gl::doCreate(p.buffer, size, offscreenBufferOptions))
                    {
                        p.buffer = dtk::gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                    }

                    // Setup the transforms.
                    const auto pm = dtk::ortho(
                        0.F,
                        static_cast<float>(g.w()),
                        0.F,
                        static_cast<float>(g.h()),
                        -1.F,
                        1.F);
                    const auto boxes = timeline::getBoxes(p.compareOptions.compare, p.videoData);
                    dtk::M44F vm;
                    vm = vm * dtk::translate(dtk::V3F(p.viewPos.x, p.viewPos.y, 0.F));
                    vm = vm * dtk::scale(dtk::V3F(p.viewZoom, p.viewZoom, 1.F));

                    // Setup the state.
                    const dtk::ViewportState viewportState(render);
                    const dtk::ClipRectEnabledState clipRectEnabledState(render);
                    const dtk::ClipRectState clipRectState(render);
                    const dtk::TransformState transformState(render);
                    const dtk::RenderSizeState renderSizeState(render);
                    render->setRenderSize(size);
                    render->setViewport(dtk::Box2I(0, 0, g.w(), g.h()));
                    render->setClipRectEnabled(false);

                    // Draw the main buffer.
                    if (p.buffer)
                    {
                        dtk::gl::OffscreenBufferBinding binding(p.buffer);
                        render->clearViewport(dtk::Color4F(0.F, 0.F, 0.F, 0.F));
                        render->setOCIOOptions(p.ocioOptions);
                        render->setLUTOptions(p.lutOptions);
                        render->setTransform(pm * vm);
                        render->drawVideo(
                            p.videoData,
                            boxes,
                            p.imageOptions,
                            p.displayOptions,
                            p.compareOptions,
                            p.colorBuffer->get());

                        if (!p.videoData.empty())
                        {
                            _droppedFramesUpdate(p.videoData[0].time);
                        }
                    }

                    // Draw the background buffer.
                    if (p.bgBuffer)
                    {
                        dtk::gl::OffscreenBufferBinding binding(p.bgBuffer);
                        render->clearViewport(dtk::Color4F(0.F, 0.F, 0.F, 0.F));
                        render->setTransform(pm);
                        render->drawBackground(boxes, vm, p.bgOptions);
                    }

                    // Draw the foreground buffer.
                    if (p.fgBuffer)
                    {
                        dtk::gl::OffscreenBufferBinding binding(p.fgBuffer);
                        render->clearViewport(dtk::Color4F(0.F, 0.F, 0.F, 0.F));
                        render->setTransform(pm);
                        render->drawForeground(boxes, vm, p.fgOptions);
                    }
                }
                catch (const std::exception& e)
                {
                    if (auto context = getContext())
                    {
                        context->log("tl::timelineui::Viewport", e.what(), dtk::LogType::Error);
                    }
                }
            }

            if (p.bgBuffer)
            {
                render->drawTexture(p.bgBuffer->getColorID(), g);
            }
            if (p.buffer)
            {
                dtk::AlphaBlend alphaBlend = dtk::AlphaBlend::Straight;
                if (!p.imageOptions.empty())
                {
                    alphaBlend = p.imageOptions.front().alphaBlend;
                }
                render->drawTexture(
                    p.buffer->getColorID(),
                    g,
                    dtk::Color4F(1.F, 1.F, 1.F),
                    alphaBlend);
            }
            if (p.fgBuffer)
            {
                render->drawTexture(p.fgBuffer->getColorID(), g);
            }
        }

        void Viewport::mouseMoveEvent(dtk::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            DTK_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::View:
            {
                const dtk::V2I& mousePressPos = _getMousePressPos();
                p.viewPos.x = p.mouse.viewPos.x + (event.pos.x - mousePressPos.x);
                p.viewPos.y = p.mouse.viewPos.y + (event.pos.y - mousePressPos.y);
                p.doRender = true;
                _setDrawUpdate();
                if (p.viewPosAndZoomCallback)
                {
                    p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
                }
                setFrameView(false);
                break;
            }
            case Private::MouseMode::Wipe:
            {
                if (p.player)
                {
                    const io::Info& ioInfo = p.player->getIOInfo();
                    if (!ioInfo.video.empty())
                    {
                        const dtk::Box2I& g = getGeometry();
                        const auto& imageInfo = ioInfo.video[0];
                        p.compareOptions.wipeCenter.x = (event.pos.x - g.min.x - p.viewPos.x) / p.viewZoom /
                            static_cast<float>(imageInfo.size.w * imageInfo.pixelAspectRatio);
                        p.compareOptions.wipeCenter.y = (event.pos.y - g.min.y - p.viewPos.y) / p.viewZoom /
                            static_cast<float>(imageInfo.size.h);
                        p.doRender = true;
                        _setDrawUpdate();
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

        void Viewport::mousePressEvent(dtk::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            DTK_P();
            takeKeyFocus();
            if (0 == event.button &&
                dtk::checkKeyModifier(p.panModifier, event.modifiers))
            {
                p.mouse.mode = Private::MouseMode::View;
                p.mouse.viewPos = p.viewPos;
            }
            else if (0 == event.button &&
                dtk::checkKeyModifier(p.wipeModifier, event.modifiers))
            {
                p.mouse.mode = Private::MouseMode::Wipe;
            }
        }

        void Viewport::mouseReleaseEvent(dtk::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            DTK_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void Viewport::scrollEvent(dtk::ScrollEvent& event)
        {
            DTK_P();
            if (static_cast<int>(dtk::KeyModifier::None) == event.modifiers)
            {
                event.accept = true;
                const double mult = 1.1;
                const double zoom =
                    event.value.y < 0 ?
                    p.viewZoom / (-event.value.y * mult) :
                    p.viewZoom * (event.value.y * mult);
                setViewZoom(zoom, event.pos - getGeometry().min);
            }
            else if (event.modifiers & static_cast<int>(dtk::KeyModifier::Control))
            {
                event.accept = true;
                if (p.player)
                {
                    const OTIO_NS::RationalTime t = p.player->getCurrentTime();
                    p.player->seek(t + OTIO_NS::RationalTime(event.value.y, t.rate()));
                }
            }
        }

        void Viewport::keyPressEvent(dtk::KeyEvent& event)
        {
            DTK_P();
            if (0 == event.modifiers)
            {
                const dtk::Box2I& g = getGeometry();
                switch (event.key)
                {
                case dtk::Key::_0:
                    event.accept = true;
                    setViewZoom(1.0, event.pos - g.min);
                    break;
                case dtk::Key::Equal:
                    event.accept = true;
                    setViewZoom(p.viewZoom * 2.0, event.pos - g.min);
                    break;
                case dtk::Key::Minus:
                    event.accept = true;
                    setViewZoom(p.viewZoom / 2.0, event.pos - g.min);
                    break;
                case dtk::Key::Backspace:
                    event.accept = true;
                    setFrameView(true);
                    break;
                default: break;
                }
            }
        }

        void Viewport::keyReleaseEvent(dtk::KeyEvent& event)
        {
            event.accept = true;
        }

        void Viewport::_releaseMouse()
        {
            IWidget::_releaseMouse();
            DTK_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        dtk::Size2I Viewport::_getRenderSize() const
        {
            DTK_P();
            return timeline::getRenderSize(p.compareOptions.compare, p.videoData);
        }

        dtk::V2I Viewport::_getViewportCenter() const
        {
            const dtk::Box2I& g = getGeometry();
            return dtk::V2I(g.w() / 2, g.h() / 2);
        }

        void Viewport::_frameView()
        {
            DTK_P();
            const dtk::Box2I& g = getGeometry();
            const dtk::Size2I viewportSize = g.size();
            const dtk::Size2I renderSize = _getRenderSize();
            double zoom = viewportSize.w / static_cast<double>(renderSize.w);
            if (zoom * renderSize.h > viewportSize.h)
            {
                zoom = viewportSize.h / static_cast<double>(renderSize.h);
            }
            const dtk::V2I c(renderSize.w / 2, renderSize.h / 2);
            const dtk::V2I viewPos(
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

        void Viewport::_droppedFramesUpdate(const OTIO_NS::RationalTime& value)
        {
            DTK_P();
            if (value != time::invalidTime && p.droppedFramesData.init)
            {
                p.droppedFramesData.init = false;
                p.droppedFrames->setIfChanged(0);
            }
            else
            {
                const double frameDiff = value.value() - p.droppedFramesData.frame;
                if (std::abs(frameDiff) > 1.0)
                {
                    p.droppedFrames->setIfChanged(p.droppedFrames->get() + 1);
                }
            }
            p.droppedFramesData.frame = value.value();
        }
    }
}
