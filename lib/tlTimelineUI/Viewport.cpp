// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/Viewport.h>

#include <tlTimeline/IRender.h>

#include <feather-tk/ui/DrawUtil.h>
#include <feather-tk/gl/GL.h>
#include <feather-tk/gl/OffscreenBuffer.h>
#include <feather-tk/gl/Util.h>
#include <feather-tk/core/Context.h>
#include <feather-tk/core/LogSystem.h>
#include <feather-tk/core/RenderUtil.h>

#include <cmath>

namespace tl
{
    namespace timelineui
    {
        struct Viewport::Private
        {
            std::shared_ptr<feather_tk::ObservableValue<timeline::CompareOptions> > compareOptions;
            std::shared_ptr<feather_tk::ObservableValue<timeline::OCIOOptions> > ocioOptions;
            std::shared_ptr<feather_tk::ObservableValue<timeline::LUTOptions> > lutOptions;
            std::shared_ptr<feather_tk::ObservableList<feather_tk::ImageOptions> > imageOptions;
            std::shared_ptr<feather_tk::ObservableList<timeline::DisplayOptions> > displayOptions;
            std::shared_ptr<feather_tk::ObservableValue<timeline::BackgroundOptions> > bgOptions;
            std::shared_ptr<feather_tk::ObservableValue<timeline::ForegroundOptions> > fgOptions;
            std::shared_ptr<feather_tk::ObservableValue<feather_tk::ImageType> > colorBuffer;
            std::shared_ptr<timeline::Player> player;
            std::vector<timeline::VideoData> videoData;
            std::shared_ptr<feather_tk::ObservableValue<feather_tk::V2I> > viewPos;
            std::shared_ptr<feather_tk::ObservableValue<double> > viewZoom;
            std::shared_ptr<feather_tk::ObservableValue<std::pair<feather_tk::V2I, double> > > viewPosZoom;
            std::shared_ptr<feather_tk::ObservableValue<bool> > frameView;
            std::shared_ptr<feather_tk::ObservableValue<bool> > framed;
            std::shared_ptr<feather_tk::ObservableValue<double> > fps;
            struct FpsData
            {
                std::chrono::steady_clock::time_point timer;
                size_t frameCount = 0;
            };
            std::optional<FpsData> fpsData;
            std::shared_ptr<feather_tk::ObservableValue<size_t> > droppedFrames;
            struct DroppedFramesData
            {
                bool init = true;
                double frame = 0.0;
            };
            std::optional<DroppedFramesData> droppedFramesData;
            feather_tk::KeyModifier panModifier = feather_tk::KeyModifier::Control;
            feather_tk::KeyModifier wipeModifier = feather_tk::KeyModifier::Alt;

            bool doRender = false;
            std::shared_ptr<feather_tk::gl::OffscreenBuffer> buffer;
            std::shared_ptr<feather_tk::gl::OffscreenBuffer> bgBuffer;
            std::shared_ptr<feather_tk::gl::OffscreenBuffer> fgBuffer;

            enum class MouseMode
            {
                None,
                View,
                Wipe
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                feather_tk::V2I viewPos;
            };
            MouseData mouse;

            std::shared_ptr<feather_tk::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<feather_tk::ListObserver<timeline::VideoData> > videoDataObserver;
        };

        void Viewport::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::timelineui::Viewport", parent);
            FEATHER_TK_P();

            setHStretch(feather_tk::Stretch::Expanding);
            setVStretch(feather_tk::Stretch::Expanding);

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true);

            p.compareOptions = feather_tk::ObservableValue<timeline::CompareOptions>::create();
            p.ocioOptions = feather_tk::ObservableValue<timeline::OCIOOptions>::create();
            p.lutOptions = feather_tk::ObservableValue<timeline::LUTOptions>::create();
            p.imageOptions = feather_tk::ObservableList<feather_tk::ImageOptions>::create();
            p.displayOptions = feather_tk::ObservableList<timeline::DisplayOptions>::create();
            p.bgOptions = feather_tk::ObservableValue<timeline::BackgroundOptions>::create();
            p.fgOptions = feather_tk::ObservableValue<timeline::ForegroundOptions>::create();
            p.compareOptions = feather_tk::ObservableValue<timeline::CompareOptions>::create();
            p.colorBuffer = feather_tk::ObservableValue<feather_tk::ImageType>::create(
                feather_tk::ImageType::RGBA_U8);
            p.viewPos = feather_tk::ObservableValue<feather_tk::V2I>::create();
            p.viewZoom = feather_tk::ObservableValue<double>::create(1.0);
            p.viewPosZoom = feather_tk::ObservableValue<std::pair<feather_tk::V2I, double> >::create(
                std::make_pair(feather_tk::V2I(), 1.0));
            p.frameView = feather_tk::ObservableValue<bool>::create(true);
            p.framed = feather_tk::ObservableValue<bool>::create(false);
            p.fps = feather_tk::ObservableValue<double>::create(0.0);
            p.droppedFrames = feather_tk::ObservableValue<size_t>::create(0);
        }

        Viewport::Viewport() :
            _p(new Private)
        {}

        Viewport::~Viewport()
        {}

        std::shared_ptr<Viewport> Viewport::create(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Viewport>(new Viewport);
            out->_init(context, parent);
            return out;
        }

        const timeline::CompareOptions& Viewport::getCompareOptions() const
        {
            return _p->compareOptions->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<timeline::CompareOptions> > Viewport::observeCompareOptions() const
        {
            return _p->compareOptions;
        }

        void Viewport::setCompareOptions(const timeline::CompareOptions& value)
        {
            FEATHER_TK_P();
            if (p.compareOptions->setIfChanged(value))
            {
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        const timeline::OCIOOptions& Viewport::getOCIOOptions() const
        {
            return _p->ocioOptions->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<timeline::OCIOOptions> > Viewport::observeOCIOOptions() const
        {
            return _p->ocioOptions;
        }

        void Viewport::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            FEATHER_TK_P();
            if (p.ocioOptions->setIfChanged(value))
            {
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        const timeline::LUTOptions& Viewport::getLUTOptions() const
        {
            return _p->lutOptions->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<timeline::LUTOptions> > Viewport::observeLUTOptions() const
        {
            return _p->lutOptions;
        }

        void Viewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            FEATHER_TK_P();
            if (p.lutOptions->setIfChanged(value))
            {
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        const std::vector<feather_tk::ImageOptions>& Viewport::getImageOptions() const
        {
            return _p->imageOptions->get();
        }

        std::shared_ptr<feather_tk::IObservableList<feather_tk::ImageOptions> > Viewport::observeImageOptions() const
        {
            return _p->imageOptions;
        }

        void Viewport::setImageOptions(const std::vector<feather_tk::ImageOptions>& value)
        {
            FEATHER_TK_P();
            if (p.imageOptions->setIfChanged(value))
            {
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        const std::vector<timeline::DisplayOptions>& Viewport::getDisplayOptions() const
        {
            return _p->displayOptions->get();
        }

        std::shared_ptr<feather_tk::IObservableList<timeline::DisplayOptions> > Viewport::observeDisplayOptions() const
        {
            return _p->displayOptions;
        }

        void Viewport::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            FEATHER_TK_P();
            if (p.displayOptions->setIfChanged(value))
            {
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        const timeline::BackgroundOptions& Viewport::getBackgroundOptions() const
        {
            return _p->bgOptions->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<timeline::BackgroundOptions> > Viewport::observeBackgroundOptions() const
        {
            return _p->bgOptions;
        }

        void Viewport::setBackgroundOptions(const timeline::BackgroundOptions& value)
        {
            FEATHER_TK_P();
            if (p.bgOptions->setIfChanged(value))
            {
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        const timeline::ForegroundOptions& Viewport::getForegroundOptions() const
        {
            return _p->fgOptions->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<timeline::ForegroundOptions> > Viewport::observeForegroundOptions() const
        {
            return _p->fgOptions;
        }

        void Viewport::setForegroundOptions(const timeline::ForegroundOptions& value)
        {
            FEATHER_TK_P();
            if (p.fgOptions->setIfChanged(value))
            {
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        feather_tk::ImageType Viewport::getColorBuffer() const
        {
            return _p->colorBuffer->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<feather_tk::ImageType> > Viewport::observeColorBuffer() const
        {
            return _p->colorBuffer;
        }

        void Viewport::setColorBuffer(feather_tk::ImageType value)
        {
            FEATHER_TK_P();
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
            FEATHER_TK_P();

            p.fps->setIfChanged(0.0);
            p.fpsData.reset();
            p.droppedFrames->setIfChanged(0);
            p.droppedFramesData.reset();
            p.playbackObserver.reset();
            p.videoDataObserver.reset();

            p.player = value;

            if (p.player)
            {
                p.playbackObserver = feather_tk::ValueObserver<timeline::Playback>::create(
                    p.player->observePlayback(),
                    [this](timeline::Playback value)
                    {
                        FEATHER_TK_P();
                        switch (value)
                        {
                        case timeline::Playback::Forward:
                        case timeline::Playback::Reverse:
                            p.fpsData = Private::FpsData();
                            p.fpsData->timer = std::chrono::steady_clock::now();
                            p.fpsData->frameCount = 0;
                            break;
                        default:
                            p.fps->setIfChanged(0.0);
                            p.fpsData.reset();
                            p.droppedFrames->setIfChanged(0);
                            p.droppedFramesData.reset();
                            break;
                        }
                    });

                p.videoDataObserver = feather_tk::ListObserver<timeline::VideoData>::create(
                    p.player->observeCurrentVideo(),
                    [this](const std::vector<timeline::VideoData>& value)
                    {
                        FEATHER_TK_P();
                        p.videoData = value;

                        if (p.fpsData.has_value())
                        {
                            p.fpsData->frameCount = p.fpsData->frameCount + 1;
                            const auto now = std::chrono::steady_clock::now();
                            const std::chrono::duration<double> diff = now - p.fpsData->timer;
                            if (diff.count() > 1.0)
                            {
                                const double fps = p.fpsData->frameCount / diff.count();
                                //std::cout << "FPS: " << fps << std::endl;
                                p.fps->setIfChanged(fps);
                                p.fpsData->timer = now;
                                p.fpsData->frameCount = 0;
                            }
                        }

                        p.doRender = true;
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

        const feather_tk::V2I& Viewport::getViewPos() const
        {
            return _p->viewPos->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<feather_tk::V2I> > Viewport::observeViewPos() const
        {
            return _p->viewPos;
        }

        double Viewport::getViewZoom() const
        {
            return _p->viewZoom->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<double> > Viewport::observeViewZoom() const
        {
            return _p->viewZoom;
        }

        std::pair<feather_tk::V2I, double> Viewport::getViewPosAndZoom() const
        {
            return _p->viewPosZoom->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<std::pair<feather_tk::V2I, double> > > Viewport::observeViewPosAndZoom() const
        {
            return _p->viewPosZoom;
        }

        void Viewport::setViewPosAndZoom(const feather_tk::V2I& pos, double zoom)
        {
            FEATHER_TK_P();
            if (p.viewPosZoom->setIfChanged(std::make_pair(pos, zoom)))
            {
                p.viewPos->setIfChanged(pos);
                p.viewZoom->setIfChanged(zoom);
                p.doRender = true;
                _setDrawUpdate();
                setFrameView(false);
            }
        }

        void Viewport::setViewZoom(double zoom, const feather_tk::V2I& focus)
        {
            FEATHER_TK_P();
            feather_tk::V2I pos;
            const feather_tk::V2I& viewPos = p.viewPos->get();
            const double viewZoom = p.viewZoom->get();
            pos.x = focus.x + (viewPos.x - focus.x) * (zoom / viewZoom);
            pos.y = focus.y + (viewPos.y - focus.y) * (zoom / viewZoom);
            setViewPosAndZoom(pos, zoom);
        }

        bool Viewport::hasFrameView() const
        {
            return _p->frameView->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<bool> > Viewport::observeFrameView() const
        {
            return _p->frameView;
        }

        std::shared_ptr<feather_tk::IObservableValue<bool> > Viewport::observeFramed() const
        {
            return _p->framed;
        }

        void Viewport::setFrameView(bool value)
        {
            FEATHER_TK_P();
            if (p.frameView->setIfChanged(value))
            {
                p.framed->setAlways(true);
                p.doRender = true;
                _setDrawUpdate();
            }
        }

        void Viewport::viewZoomReset()
        {
            FEATHER_TK_P();
            setViewZoom(1.F, _getViewportCenter());
        }

        void Viewport::viewZoomIn()
        {
            FEATHER_TK_P();
            setViewZoom(p.viewZoom->get() * 2.0, _getViewportCenter());
        }

        void Viewport::viewZoomOut()
        {
            FEATHER_TK_P();
            setViewZoom(p.viewZoom->get() / 2.0, _getViewportCenter());
        }

        double Viewport::getFPS() const
        {
            return _p->fps->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<double> > Viewport::observeFPS() const
        {
            return _p->fps;
        }

        size_t Viewport::getDroppedFrames() const
        {
            return _p->droppedFrames->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<size_t> > Viewport::observeDroppedFrames() const
        {
            return _p->droppedFrames;
        }

        feather_tk::Color4F Viewport::getColorSample(const feather_tk::V2I& value)
        {
            FEATHER_TK_P();
            feather_tk::Color4F out;
            if (p.buffer)
            {
                const feather_tk::Box2I& g = getGeometry();
                const feather_tk::V2I pos = value - g.min;
                std::vector<float> sample(4);
                feather_tk::gl::OffscreenBufferBinding binding(p.buffer);
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
#if defined(FEATHER_TK_API_GL_4_1)
                glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
#endif // FEATHER_TK_API_GL_4_1
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

        void Viewport::setPanModifier(feather_tk::KeyModifier value)
        {
            _p->panModifier = value;
        }

        void Viewport::setWipeModifier(feather_tk::KeyModifier value)
        {
            _p->wipeModifier = value;
        }

        void Viewport::setGeometry(const feather_tk::Box2I& value)
        {
            const bool changed = value != getGeometry();
            IWidget::setGeometry(value);
            FEATHER_TK_P();
            if (changed)
            {
                p.doRender = true;
            }
        }

        void Viewport::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            const int sa = event.style->getSizeRole(feather_tk::SizeRole::ScrollArea, event.displayScale);
            _setSizeHint(feather_tk::Size2I(sa, sa));
        }

        void Viewport::drawEvent(const feather_tk::Box2I& drawRect, const feather_tk::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            FEATHER_TK_P();

            if (p.frameView->get())
            {
                _frameView();
            }

            auto render = std::dynamic_pointer_cast<timeline::IRender>(event.render);
            const feather_tk::Box2I& g = getGeometry();
            render->drawRect(g, feather_tk::Color4F(0.F, 0.F, 0.F));

            if (p.doRender)
            {
                p.doRender = false;
                try
                {
                    // Create the background and foreground buffers.
                    const feather_tk::Size2I size = g.size();
                    feather_tk::gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.color = feather_tk::ImageType::RGBA_U8;
                    offscreenBufferOptions.colorFilters.minify = feather_tk::ImageFilter::Nearest;
                    offscreenBufferOptions.colorFilters.magnify = feather_tk::ImageFilter::Nearest;
                    if (feather_tk::gl::doCreate(p.bgBuffer, size, offscreenBufferOptions))
                    {
                        p.bgBuffer = feather_tk::gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                    }
                    if (feather_tk::gl::doCreate(p.fgBuffer, size, offscreenBufferOptions))
                    {
                        p.fgBuffer = feather_tk::gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                    }

                    // Create the main buffer.
                    offscreenBufferOptions.colorFilters.minify = feather_tk::ImageFilter::Linear;
                    offscreenBufferOptions.colorFilters.magnify = feather_tk::ImageFilter::Linear;
                    offscreenBufferOptions.color = p.colorBuffer->get();
                    if (!p.displayOptions->isEmpty())
                    {
                        offscreenBufferOptions.colorFilters = p.displayOptions->getItem(0).imageFilters;
                    }
#if defined(FEATHER_TK_API_GL_4_1)
                    offscreenBufferOptions.depth = feather_tk::gl::OffscreenDepth::_24;
                    offscreenBufferOptions.stencil = feather_tk::gl::OffscreenStencil::_8;
#elif defined(FEATHER_TK_API_GLES_2)
                    offscreenBufferOptions.stencil = feather_tk::gl::OffscreenStencil::_8;
#endif // FEATHER_TK_API_GL_4_1
                    if (feather_tk::gl::doCreate(p.buffer, size, offscreenBufferOptions))
                    {
                        p.buffer = feather_tk::gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                    }

                    // Setup the transforms.
                    const auto pm = feather_tk::ortho(
                        0.F,
                        static_cast<float>(g.w()),
                        0.F,
                        static_cast<float>(g.h()),
                        -1.F,
                        1.F);
                    const timeline::CompareOptions& compareOptions = p.compareOptions->get();
                    const auto boxes = timeline::getBoxes(compareOptions.compare, p.videoData);
                    feather_tk::M44F vm;
                    const feather_tk::V2I& viewPos = p.viewPos->get();
                    const double viewZoom = p.viewZoom->get();
                    vm = vm * feather_tk::translate(feather_tk::V3F(viewPos.x, viewPos.y, 0.F));
                    vm = vm * feather_tk::scale(feather_tk::V3F(viewZoom, viewZoom, 1.F));

                    // Setup the state.
                    const feather_tk::ViewportState viewportState(render);
                    const feather_tk::ClipRectEnabledState clipRectEnabledState(render);
                    const feather_tk::ClipRectState clipRectState(render);
                    const feather_tk::TransformState transformState(render);
                    const feather_tk::RenderSizeState renderSizeState(render);
                    render->setRenderSize(size);
                    render->setViewport(feather_tk::Box2I(0, 0, g.w(), g.h()));
                    render->setClipRectEnabled(false);

                    // Draw the main buffer.
                    if (p.buffer)
                    {
                        feather_tk::gl::OffscreenBufferBinding binding(p.buffer);
                        render->clearViewport(feather_tk::Color4F(0.F, 0.F, 0.F, 0.F));
                        render->setOCIOOptions(p.ocioOptions->get());
                        render->setLUTOptions(p.lutOptions->get());
                        render->setTransform(pm * vm);
                        render->drawVideo(
                            p.videoData,
                            boxes,
                            p.imageOptions->get(),
                            p.displayOptions->get(),
                            compareOptions,
                            p.colorBuffer->get());

                        if (!p.videoData.empty() && p.fpsData.has_value())
                        {
                            _droppedFramesUpdate(p.videoData[0].time);
                        }
                    }

                    // Draw the background buffer.
                    if (p.bgBuffer)
                    {
                        feather_tk::gl::OffscreenBufferBinding binding(p.bgBuffer);
                        render->clearViewport(feather_tk::Color4F(0.F, 0.F, 0.F, 0.F));
                        render->setTransform(pm);
                        render->drawBackground(boxes, vm, p.bgOptions->get());
                    }

                    // Draw the foreground buffer.
                    if (p.fgBuffer)
                    {
                        feather_tk::gl::OffscreenBufferBinding binding(p.fgBuffer);
                        render->clearViewport(feather_tk::Color4F(0.F, 0.F, 0.F, 0.F));
                        render->setTransform(pm);
                        render->drawForeground(boxes, vm, p.fgOptions->get());
                    }
                }
                catch (const std::exception& e)
                {
                    if (auto context = getContext())
                    {
                        context->log("tl::timelineui::Viewport", e.what(), feather_tk::LogType::Error);
                    }
                }
            }

            if (p.bgBuffer)
            {
                render->drawTexture(p.bgBuffer->getColorID(), g);
            }
            if (p.buffer)
            {
                feather_tk::AlphaBlend alphaBlend = feather_tk::AlphaBlend::Straight;
                if (!p.imageOptions->isEmpty())
                {
                    alphaBlend = p.imageOptions->getItem(0).alphaBlend;
                }
                render->drawTexture(
                    p.buffer->getColorID(),
                    g,
                    feather_tk::Color4F(1.F, 1.F, 1.F),
                    alphaBlend);
            }
            if (p.fgBuffer)
            {
                render->drawTexture(p.fgBuffer->getColorID(), g);
            }
        }

        void Viewport::mouseMoveEvent(feather_tk::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            FEATHER_TK_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::View:
            {
                const feather_tk::V2I& mousePressPos = _getMousePressPos();
                const feather_tk::V2I viewPos(
                    p.mouse.viewPos.x + (event.pos.x - mousePressPos.x),
                    p.mouse.viewPos.y + (event.pos.y - mousePressPos.y));
                const double viewZoom = p.viewZoom->get();
                if (p.viewPosZoom->setIfChanged(std::make_pair(viewPos, viewZoom)))
                {
                    p.viewPos->setIfChanged(viewPos);
                    p.viewZoom->setIfChanged(viewZoom);
                    p.doRender = true;
                    _setDrawUpdate();
                    setFrameView(false);
                }
                break;
            }
            case Private::MouseMode::Wipe:
            {
                if (p.player)
                {
                    const io::Info& ioInfo = p.player->getIOInfo();
                    if (!ioInfo.video.empty())
                    {
                        const feather_tk::Box2I& g = getGeometry();
                        const feather_tk::V2I& viewPos = p.viewPos->get();
                        const double viewZoom = p.viewZoom->get();
                        const auto& imageInfo = ioInfo.video[0];
                        timeline::CompareOptions compareOptions = p.compareOptions->get();
                        compareOptions.wipeCenter.x = (event.pos.x - g.min.x - viewPos.x) / viewZoom /
                            static_cast<float>(imageInfo.size.w * imageInfo.pixelAspectRatio);
                        compareOptions.wipeCenter.y = (event.pos.y - g.min.y - viewPos.y) / viewZoom /
                            static_cast<float>(imageInfo.size.h);
                        if (p.compareOptions->setIfChanged(compareOptions))
                        {
                            p.doRender = true;
                            _setDrawUpdate();
                        }
                    }
                }
                break;
            }
            default: break;
            }
        }

        void Viewport::mousePressEvent(feather_tk::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            FEATHER_TK_P();
            takeKeyFocus();
            if (0 == event.button &&
                feather_tk::checkKeyModifier(p.panModifier, event.modifiers))
            {
                p.mouse.mode = Private::MouseMode::View;
                p.mouse.viewPos = p.viewPos->get();
            }
            else if (0 == event.button &&
                feather_tk::checkKeyModifier(p.wipeModifier, event.modifiers))
            {
                p.mouse.mode = Private::MouseMode::Wipe;
            }
        }

        void Viewport::mouseReleaseEvent(feather_tk::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            FEATHER_TK_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void Viewport::scrollEvent(feather_tk::ScrollEvent& event)
        {
            FEATHER_TK_P();
            if (static_cast<int>(feather_tk::KeyModifier::None) == event.modifiers)
            {
                event.accept = true;
                const double viewZoom = p.viewZoom->get();
                const double mult = 1.1;
                const double newZoom =
                    event.value.y < 0 ?
                    viewZoom / (-event.value.y * mult) :
                    viewZoom * (event.value.y * mult);
                setViewZoom(newZoom, event.pos - getGeometry().min);
            }
            else if (event.modifiers & static_cast<int>(feather_tk::KeyModifier::Control))
            {
                event.accept = true;
                if (p.player)
                {
                    const OTIO_NS::RationalTime t = p.player->getCurrentTime();
                    p.player->seek(t + OTIO_NS::RationalTime(event.value.y, t.rate()));
                }
            }
        }

        void Viewport::keyPressEvent(feather_tk::KeyEvent& event)
        {
            FEATHER_TK_P();
            if (0 == event.modifiers)
            {
                const feather_tk::Box2I& g = getGeometry();
                switch (event.key)
                {
                case feather_tk::Key::_0:
                    event.accept = true;
                    setViewZoom(1.0, event.pos - g.min);
                    break;
                case feather_tk::Key::Equal:
                    event.accept = true;
                    setViewZoom(p.viewZoom->get() * 2.0, event.pos - g.min);
                    break;
                case feather_tk::Key::Minus:
                    event.accept = true;
                    setViewZoom(p.viewZoom->get() / 2.0, event.pos - g.min);
                    break;
                case feather_tk::Key::Backspace:
                    event.accept = true;
                    setFrameView(true);
                    break;
                default: break;
                }
            }
        }

        void Viewport::keyReleaseEvent(feather_tk::KeyEvent& event)
        {
            event.accept = true;
        }

        void Viewport::_releaseMouse()
        {
            IWidget::_releaseMouse();
            FEATHER_TK_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        feather_tk::Size2I Viewport::_getRenderSize() const
        {
            FEATHER_TK_P();
            return timeline::getRenderSize(p.compareOptions->get().compare, p.videoData);
        }

        feather_tk::V2I Viewport::_getViewportCenter() const
        {
            const feather_tk::Box2I& g = getGeometry();
            return feather_tk::V2I(g.w() / 2, g.h() / 2);
        }

        void Viewport::_frameView()
        {
            FEATHER_TK_P();
            const feather_tk::Box2I& g = getGeometry();
            const feather_tk::Size2I viewportSize = g.size();
            const feather_tk::Size2I renderSize = _getRenderSize();
            double viewZoom = viewportSize.w / static_cast<double>(renderSize.w);
            if (viewZoom * renderSize.h > viewportSize.h)
            {
                viewZoom = viewportSize.h / static_cast<double>(renderSize.h);
            }
            const feather_tk::V2I c(renderSize.w / 2, renderSize.h / 2);
            const feather_tk::V2I viewPos(
                viewportSize.w / 2.F - c.x * viewZoom,
                viewportSize.h / 2.F - c.y * viewZoom);
            if (p.viewPosZoom->setIfChanged(std::make_pair(viewPos, viewZoom)))
            {
                p.viewPos->setIfChanged(viewPos);
                p.viewZoom->setIfChanged(viewZoom);
            }
        }

        void Viewport::_droppedFramesUpdate(const OTIO_NS::RationalTime& value)
        {
            FEATHER_TK_P();
            if (!p.droppedFramesData.has_value())
            {
                p.droppedFramesData = Private::DroppedFramesData();
            }
            else
            {
                const double frameDiff = value.value() - p.droppedFramesData->frame;
                if (std::abs(frameDiff) > 1.0)
                {
                    p.droppedFrames->setIfChanged(p.droppedFrames->get() + 1);
                }
            }
            p.droppedFramesData->frame = value.value();
        }
    }
}
