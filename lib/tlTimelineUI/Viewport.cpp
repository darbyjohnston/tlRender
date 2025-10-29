// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/Viewport.h>

#include <tlTimeline/IRender.h>

#include <ftk/UI/DrawUtil.h>
#include <ftk/GL/GL.h>
#include <ftk/GL/OffscreenBuffer.h>
#include <ftk/GL/Util.h>
#include <ftk/Core/Context.h>
#include <ftk/Core/LogSystem.h>
#include <ftk/Core/RenderUtil.h>

#include <cmath>

namespace tl
{
    namespace timelineui
    {
        struct Viewport::Private
        {
            std::shared_ptr<ftk::ObservableValue<timeline::CompareOptions> > compareOptions;
            std::shared_ptr<ftk::ObservableValue<timeline::OCIOOptions> > ocioOptions;
            std::shared_ptr<ftk::ObservableValue<timeline::LUTOptions> > lutOptions;
            std::shared_ptr<ftk::ObservableList<ftk::ImageOptions> > imageOptions;
            std::shared_ptr<ftk::ObservableList<timeline::DisplayOptions> > displayOptions;
            std::shared_ptr<ftk::ObservableValue<timeline::BackgroundOptions> > bgOptions;
            std::shared_ptr<ftk::ObservableValue<timeline::ForegroundOptions> > fgOptions;
            std::shared_ptr<ftk::ObservableValue<ftk::ImageType> > colorBuffer;
            std::shared_ptr<timeline::Player> player;
            std::vector<timeline::VideoData> videoData;
            std::shared_ptr<ftk::ObservableValue<ftk::V2I> > viewPos;
            std::shared_ptr<ftk::ObservableValue<double> > viewZoom;
            std::shared_ptr<ftk::ObservableValue<std::pair<ftk::V2I, double> > > viewPosZoom;
            std::shared_ptr<ftk::ObservableValue<bool> > frameView;
            std::shared_ptr<ftk::ObservableValue<bool> > framed;
            std::shared_ptr<ftk::ObservableValue<double> > fps;
            struct FpsData
            {
                std::chrono::steady_clock::time_point timer;
                size_t frameCount = 0;
            };
            std::optional<FpsData> fpsData;
            std::shared_ptr<ftk::ObservableValue<size_t> > droppedFrames;
            struct DroppedFramesData
            {
                bool init = true;
                double frame = 0.0;
            };
            std::optional<DroppedFramesData> droppedFramesData;
            std::pair<int, ftk::KeyModifier> panBinding = std::make_pair(1, ftk::KeyModifier::Control);
            std::pair<int, ftk::KeyModifier> wipeBinding = std::make_pair(1, ftk::KeyModifier::Alt);
            float mouseWheelScale = 1.1F;

            bool doRender = false;
            std::shared_ptr<ftk::gl::OffscreenBuffer> buffer;
            std::shared_ptr<ftk::gl::OffscreenBuffer> bgBuffer;
            std::shared_ptr<ftk::gl::OffscreenBuffer> fgBuffer;

            enum class MouseMode
            {
                None,
                View,
                Wipe
            };
            struct MouseData
            {
                bool inside = false;
                ftk::V2I press;
                MouseMode mode = MouseMode::None;
                ftk::V2I viewPos;
            };
            MouseData mouse;

            std::shared_ptr<ftk::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<ftk::ListObserver<timeline::VideoData> > videoDataObserver;
        };

        void Viewport::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::timelineui::Viewport", parent);
            FTK_P();

            setHStretch(ftk::Stretch::Expanding);
            setVStretch(ftk::Stretch::Expanding);

            p.compareOptions = ftk::ObservableValue<timeline::CompareOptions>::create();
            p.ocioOptions = ftk::ObservableValue<timeline::OCIOOptions>::create();
            p.lutOptions = ftk::ObservableValue<timeline::LUTOptions>::create();
            p.imageOptions = ftk::ObservableList<ftk::ImageOptions>::create();
            p.displayOptions = ftk::ObservableList<timeline::DisplayOptions>::create();
            p.bgOptions = ftk::ObservableValue<timeline::BackgroundOptions>::create();
            p.fgOptions = ftk::ObservableValue<timeline::ForegroundOptions>::create();
            p.compareOptions = ftk::ObservableValue<timeline::CompareOptions>::create();
            p.colorBuffer = ftk::ObservableValue<ftk::ImageType>::create(
                ftk::ImageType::RGBA_U8);
            p.viewPos = ftk::ObservableValue<ftk::V2I>::create();
            p.viewZoom = ftk::ObservableValue<double>::create(1.0);
            p.viewPosZoom = ftk::ObservableValue<std::pair<ftk::V2I, double> >::create(
                std::make_pair(ftk::V2I(), 1.0));
            p.frameView = ftk::ObservableValue<bool>::create(true);
            p.framed = ftk::ObservableValue<bool>::create(false);
            p.fps = ftk::ObservableValue<double>::create(0.0);
            p.droppedFrames = ftk::ObservableValue<size_t>::create(0);
        }

        Viewport::Viewport() :
            _p(new Private)
        {}

        Viewport::~Viewport()
        {}

        std::shared_ptr<Viewport> Viewport::create(
            const std::shared_ptr<ftk::Context>& context,
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

        std::shared_ptr<ftk::IObservableValue<timeline::CompareOptions> > Viewport::observeCompareOptions() const
        {
            return _p->compareOptions;
        }

        void Viewport::setCompareOptions(const timeline::CompareOptions& value)
        {
            FTK_P();
            if (p.compareOptions->setIfChanged(value))
            {
                p.doRender = true;
                setDrawUpdate();
            }
        }

        const timeline::OCIOOptions& Viewport::getOCIOOptions() const
        {
            return _p->ocioOptions->get();
        }

        std::shared_ptr<ftk::IObservableValue<timeline::OCIOOptions> > Viewport::observeOCIOOptions() const
        {
            return _p->ocioOptions;
        }

        void Viewport::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            FTK_P();
            if (p.ocioOptions->setIfChanged(value))
            {
                p.doRender = true;
                setDrawUpdate();
            }
        }

        const timeline::LUTOptions& Viewport::getLUTOptions() const
        {
            return _p->lutOptions->get();
        }

        std::shared_ptr<ftk::IObservableValue<timeline::LUTOptions> > Viewport::observeLUTOptions() const
        {
            return _p->lutOptions;
        }

        void Viewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            FTK_P();
            if (p.lutOptions->setIfChanged(value))
            {
                p.doRender = true;
                setDrawUpdate();
            }
        }

        const std::vector<ftk::ImageOptions>& Viewport::getImageOptions() const
        {
            return _p->imageOptions->get();
        }

        std::shared_ptr<ftk::IObservableList<ftk::ImageOptions> > Viewport::observeImageOptions() const
        {
            return _p->imageOptions;
        }

        void Viewport::setImageOptions(const std::vector<ftk::ImageOptions>& value)
        {
            FTK_P();
            if (p.imageOptions->setIfChanged(value))
            {
                p.doRender = true;
                setDrawUpdate();
            }
        }

        const std::vector<timeline::DisplayOptions>& Viewport::getDisplayOptions() const
        {
            return _p->displayOptions->get();
        }

        std::shared_ptr<ftk::IObservableList<timeline::DisplayOptions> > Viewport::observeDisplayOptions() const
        {
            return _p->displayOptions;
        }

        void Viewport::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            FTK_P();
            if (p.displayOptions->setIfChanged(value))
            {
                p.doRender = true;
                setDrawUpdate();
            }
        }

        const timeline::BackgroundOptions& Viewport::getBackgroundOptions() const
        {
            return _p->bgOptions->get();
        }

        std::shared_ptr<ftk::IObservableValue<timeline::BackgroundOptions> > Viewport::observeBackgroundOptions() const
        {
            return _p->bgOptions;
        }

        void Viewport::setBackgroundOptions(const timeline::BackgroundOptions& value)
        {
            FTK_P();
            if (p.bgOptions->setIfChanged(value))
            {
                p.doRender = true;
                setDrawUpdate();
            }
        }

        const timeline::ForegroundOptions& Viewport::getForegroundOptions() const
        {
            return _p->fgOptions->get();
        }

        std::shared_ptr<ftk::IObservableValue<timeline::ForegroundOptions> > Viewport::observeForegroundOptions() const
        {
            return _p->fgOptions;
        }

        void Viewport::setForegroundOptions(const timeline::ForegroundOptions& value)
        {
            FTK_P();
            if (p.fgOptions->setIfChanged(value))
            {
                p.doRender = true;
                setDrawUpdate();
            }
        }

        ftk::ImageType Viewport::getColorBuffer() const
        {
            return _p->colorBuffer->get();
        }

        std::shared_ptr<ftk::IObservableValue<ftk::ImageType> > Viewport::observeColorBuffer() const
        {
            return _p->colorBuffer;
        }

        void Viewport::setColorBuffer(ftk::ImageType value)
        {
            FTK_P();
            if (p.colorBuffer->setIfChanged(value))
            {
                p.doRender = true;
                setDrawUpdate();
            }
        }

        const std::shared_ptr<timeline::Player>& Viewport::getPlayer() const
        {
            return _p->player;
        }

        void Viewport::setPlayer(const std::shared_ptr<timeline::Player>& value)
        {
            FTK_P();

            p.fps->setIfChanged(0.0);
            p.fpsData.reset();
            p.droppedFrames->setIfChanged(0);
            p.droppedFramesData.reset();
            p.playbackObserver.reset();
            p.videoDataObserver.reset();

            p.player = value;

            if (p.player)
            {
                p.playbackObserver = ftk::ValueObserver<timeline::Playback>::create(
                    p.player->observePlayback(),
                    [this](timeline::Playback value)
                    {
                        FTK_P();
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

                p.videoDataObserver = ftk::ListObserver<timeline::VideoData>::create(
                    p.player->observeCurrentVideo(),
                    [this](const std::vector<timeline::VideoData>& value)
                    {
                        FTK_P();
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
                        setDrawUpdate();
                    });
            }
            else if (!p.videoData.empty())
            {
                p.videoData.clear();
                p.doRender = true;
                setDrawUpdate();
            }
        }

        const ftk::V2I& Viewport::getViewPos() const
        {
            return _p->viewPos->get();
        }

        std::shared_ptr<ftk::IObservableValue<ftk::V2I> > Viewport::observeViewPos() const
        {
            return _p->viewPos;
        }

        double Viewport::getViewZoom() const
        {
            return _p->viewZoom->get();
        }

        std::shared_ptr<ftk::IObservableValue<double> > Viewport::observeViewZoom() const
        {
            return _p->viewZoom;
        }

        std::pair<ftk::V2I, double> Viewport::getViewPosAndZoom() const
        {
            return _p->viewPosZoom->get();
        }

        std::shared_ptr<ftk::IObservableValue<std::pair<ftk::V2I, double> > > Viewport::observeViewPosAndZoom() const
        {
            return _p->viewPosZoom;
        }

        void Viewport::setViewPosAndZoom(const ftk::V2I& pos, double zoom)
        {
            FTK_P();
            if (p.viewPosZoom->setIfChanged(std::make_pair(pos, zoom)))
            {
                p.viewPos->setIfChanged(pos);
                p.viewZoom->setIfChanged(zoom);
                p.doRender = true;
                setDrawUpdate();
                setFrameView(false);
            }
        }

        void Viewport::setViewZoom(double zoom, const ftk::V2I& focus)
        {
            FTK_P();
            ftk::V2I pos;
            const ftk::V2I& viewPos = p.viewPos->get();
            const double viewZoom = p.viewZoom->get();
            pos.x = focus.x + (viewPos.x - focus.x) * (zoom / viewZoom);
            pos.y = focus.y + (viewPos.y - focus.y) * (zoom / viewZoom);
            setViewPosAndZoom(pos, zoom);
        }

        bool Viewport::hasFrameView() const
        {
            return _p->frameView->get();
        }

        std::shared_ptr<ftk::IObservableValue<bool> > Viewport::observeFrameView() const
        {
            return _p->frameView;
        }

        std::shared_ptr<ftk::IObservableValue<bool> > Viewport::observeFramed() const
        {
            return _p->framed;
        }

        void Viewport::setFrameView(bool value)
        {
            FTK_P();
            if (p.frameView->setIfChanged(value))
            {
                p.framed->setAlways(true);
                p.doRender = true;
                setDrawUpdate();
            }
        }

        void Viewport::viewZoomReset()
        {
            FTK_P();
            setViewZoom(1.F, _getViewportCenter());
        }

        void Viewport::viewZoomIn()
        {
            FTK_P();
            setViewZoom(p.viewZoom->get() * 2.0, _getViewportCenter());
        }

        void Viewport::viewZoomOut()
        {
            FTK_P();
            setViewZoom(p.viewZoom->get() / 2.0, _getViewportCenter());
        }

        double Viewport::getFPS() const
        {
            return _p->fps->get();
        }

        std::shared_ptr<ftk::IObservableValue<double> > Viewport::observeFPS() const
        {
            return _p->fps;
        }

        size_t Viewport::getDroppedFrames() const
        {
            return _p->droppedFrames->get();
        }

        std::shared_ptr<ftk::IObservableValue<size_t> > Viewport::observeDroppedFrames() const
        {
            return _p->droppedFrames;
        }

        ftk::Color4F Viewport::getColorSample(const ftk::V2I& value)
        {
            FTK_P();
            ftk::Color4F out;
            if (p.buffer)
            {
                const ftk::Box2I& g = getGeometry();
                const ftk::V2I pos = value - g.min;
                std::vector<float> sample(4);
                ftk::gl::OffscreenBufferBinding binding(p.buffer);
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
#if defined(FTK_API_GL_4_1)
                glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
#endif // FTK_API_GL_4_1
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

        void Viewport::setPanBinding(int button, ftk::KeyModifier modifier)
        {
            FTK_P();
            p.panBinding = std::make_pair(button, modifier);
        }

        void Viewport::setWipeBinding(int button, ftk::KeyModifier modifier)
        {
            FTK_P();
            p.wipeBinding = std::make_pair(button, modifier);
        }

        void Viewport::setMouseWheelScale(float value)
        {
            _p->mouseWheelScale = value;
        }

        void Viewport::setGeometry(const ftk::Box2I& value)
        {
            const bool changed = value != getGeometry();
            IWidget::setGeometry(value);
            FTK_P();
            if (changed)
            {
                p.doRender = true;
            }
        }

        void Viewport::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            const int sa = event.style->getSizeRole(ftk::SizeRole::ScrollArea, event.displayScale);
            _setSizeHint(ftk::Size2I(sa, sa));
        }

        void Viewport::drawEvent(const ftk::Box2I& drawRect, const ftk::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            FTK_P();

            if (p.frameView->get())
            {
                _frameView();
            }

            auto render = std::dynamic_pointer_cast<timeline::IRender>(event.render);
            const ftk::Box2I& g = getGeometry();
            render->drawRect(g, ftk::Color4F(0.F, 0.F, 0.F));

            if (p.doRender)
            {
                p.doRender = false;
                try
                {
                    // Create the background and foreground buffers.
                    const ftk::Size2I size = g.size();
                    ftk::gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.color = ftk::ImageType::RGBA_U8;
                    offscreenBufferOptions.colorFilters.minify = ftk::ImageFilter::Nearest;
                    offscreenBufferOptions.colorFilters.magnify = ftk::ImageFilter::Nearest;
                    if (ftk::gl::doCreate(p.bgBuffer, size, offscreenBufferOptions))
                    {
                        p.bgBuffer = ftk::gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                    }
                    if (ftk::gl::doCreate(p.fgBuffer, size, offscreenBufferOptions))
                    {
                        p.fgBuffer = ftk::gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                    }

                    // Create the main buffer.
                    offscreenBufferOptions.colorFilters.minify = ftk::ImageFilter::Linear;
                    offscreenBufferOptions.colorFilters.magnify = ftk::ImageFilter::Linear;
                    offscreenBufferOptions.color = p.colorBuffer->get();
                    if (!p.displayOptions->isEmpty())
                    {
                        offscreenBufferOptions.colorFilters = p.displayOptions->getItem(0).imageFilters;
                    }
#if defined(FTK_API_GL_4_1)
                    offscreenBufferOptions.depth = ftk::gl::OffscreenDepth::_24;
                    offscreenBufferOptions.stencil = ftk::gl::OffscreenStencil::_8;
#elif defined(FTK_API_GLES_2)
                    offscreenBufferOptions.stencil = ftk::gl::OffscreenStencil::_8;
#endif // FTK_API_GL_4_1
                    if (ftk::gl::doCreate(p.buffer, size, offscreenBufferOptions))
                    {
                        p.buffer = ftk::gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                    }

                    // Setup the transforms.
                    const auto pm = ftk::ortho(
                        0.F,
                        static_cast<float>(g.w()),
                        static_cast<float>(g.h()),
                        0.F,
                        -1.F,
                        1.F);
                    const timeline::CompareOptions& compareOptions = p.compareOptions->get();
                    const auto boxes = timeline::getBoxes(compareOptions.compare, p.videoData);
                    const ftk::V2I& viewPos = p.viewPos->get();
                    const double viewZoom = p.viewZoom->get();
                    const ftk::M44F vm =
                        ftk::translate(ftk::V3F(viewPos.x, viewPos.y, 0.F)) *
                        ftk::scale(ftk::V3F(viewZoom, viewZoom, 1.F));

                    // Setup the state.
                    const ftk::ViewportState viewportState(render);
                    const ftk::ClipRectEnabledState clipRectEnabledState(render);
                    const ftk::ClipRectState clipRectState(render);
                    const ftk::TransformState transformState(render);
                    const ftk::RenderSizeState renderSizeState(render);
                    render->setRenderSize(size);
                    render->setViewport(ftk::Box2I(0, 0, g.w(), g.h()));
                    render->setClipRectEnabled(false);

                    // Draw the main buffer.
                    if (p.buffer)
                    {
                        ftk::gl::OffscreenBufferBinding binding(p.buffer);
                        render->clearViewport(ftk::Color4F(0.F, 0.F, 0.F, 0.F));
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
                        ftk::gl::OffscreenBufferBinding binding(p.bgBuffer);
                        render->clearViewport(ftk::Color4F(0.F, 0.F, 0.F, 0.F));
                        render->setTransform(pm);
                        render->drawBackground(boxes, vm, p.bgOptions->get());
                    }

                    // Draw the foreground buffer.
                    if (p.fgBuffer)
                    {
                        ftk::gl::OffscreenBufferBinding binding(p.fgBuffer);
                        render->clearViewport(ftk::Color4F(0.F, 0.F, 0.F, 0.F));
                        render->setTransform(pm);
                        render->drawForeground(boxes, vm, p.fgOptions->get());
                    }
                }
                catch (const std::exception& e)
                {
                    if (auto context = getContext())
                    {
                        context->log("tl::timelineui::Viewport", e.what(), ftk::LogType::Error);
                    }
                }
            }

            if (p.bgBuffer)
            {
                render->drawTexture(p.bgBuffer->getColorID(), g);
            }
            if (p.buffer)
            {
                ftk::AlphaBlend alphaBlend = ftk::AlphaBlend::Straight;
                if (!p.imageOptions->isEmpty())
                {
                    alphaBlend = p.imageOptions->getItem(0).alphaBlend;
                }
                render->drawTexture(
                    p.buffer->getColorID(),
                    g,
                    false,
                    ftk::Color4F(1.F, 1.F, 1.F),
                    alphaBlend);
            }
            if (p.fgBuffer)
            {
                render->drawTexture(p.fgBuffer->getColorID(), g);
            }
        }
        void Viewport::mouseEnterEvent(ftk::MouseEnterEvent& event)
        {
            FTK_P();
            event.accept = true;
            p.mouse.inside = true;
        }

        void Viewport::mouseLeaveEvent()
        {
            FTK_P();
            p.mouse.inside = false;
        }

        void Viewport::mouseMoveEvent(ftk::MouseMoveEvent& event)
        {
            FTK_P();
            event.accept = true;

            const ftk::Box2I& g = getGeometry();
            const ftk::V2I pos(
                event.pos.x - g.min.x,
                (g.h() - 1) - (event.pos.y - g.min.y));

            switch (p.mouse.mode)
            {
            case Private::MouseMode::View:
            {
                const ftk::V2I viewPos(
                    p.mouse.viewPos.x + (pos.x - p.mouse.press.x),
                    p.mouse.viewPos.y + (pos.y - p.mouse.press.y));
                const double viewZoom = p.viewZoom->get();
                if (p.viewPosZoom->setIfChanged(std::make_pair(viewPos, viewZoom)))
                {
                    p.viewPos->setIfChanged(viewPos);
                    p.viewZoom->setIfChanged(viewZoom);
                    p.doRender = true;
                    setDrawUpdate();
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
                        const ftk::V2I& viewPos = p.viewPos->get();
                        const double viewZoom = p.viewZoom->get();
                        const auto& imageInfo = ioInfo.video[0];
                        timeline::CompareOptions compareOptions = p.compareOptions->get();
                        compareOptions.wipeCenter.x = (pos.x - viewPos.x) / viewZoom /
                            static_cast<float>(imageInfo.size.w * imageInfo.pixelAspectRatio);
                        compareOptions.wipeCenter.y = (pos.y - viewPos.y) / viewZoom /
                            static_cast<float>(imageInfo.size.h);
                        if (p.compareOptions->setIfChanged(compareOptions))
                        {
                            p.doRender = true;
                            setDrawUpdate();
                        }
                    }
                }
                break;
            }
            default: break;
            }
        }

        void Viewport::mousePressEvent(ftk::MouseClickEvent& event)
        {
            FTK_P();
            event.accept = true;
            takeKeyFocus();

            const ftk::Box2I& g = getGeometry();
            const ftk::V2I pos(
                event.pos.x - g.min.x,
                (g.h() - 1) - (event.pos.y - g.min.y));
            p.mouse.press = pos;

            if (p.panBinding.first == event.button &&
                ftk::checkKeyModifier(p.panBinding.second, event.modifiers))
            {
                p.mouse.mode = Private::MouseMode::View;
                p.mouse.viewPos = p.viewPos->get();
            }
            else if (p.wipeBinding.first == event.button &&
                ftk::checkKeyModifier(p.wipeBinding.second, event.modifiers))
            {
                p.mouse.mode = Private::MouseMode::Wipe;
            }
            else
            {
                p.mouse.mode = Private::MouseMode::None;
            }
        }

        void Viewport::mouseReleaseEvent(ftk::MouseClickEvent& event)
        {
            FTK_P();
            event.accept = true;
            p.mouse.mode = Private::MouseMode::None;
        }

        void Viewport::scrollEvent(ftk::ScrollEvent& event)
        {
            FTK_P();

            if (static_cast<int>(ftk::KeyModifier::None) == event.modifiers)
            {
                event.accept = true;

                const ftk::Box2I& g = getGeometry();
                const ftk::V2I pos(
                    event.pos.x - g.min.x,
                    (g.h() - 1) - (event.pos.y - g.min.y));

                const double viewZoom = p.viewZoom->get();
                const double newZoom =
                    event.value.y > 0 ?
                    viewZoom * p.mouseWheelScale :
                    viewZoom / p.mouseWheelScale;
                setViewZoom(newZoom, pos);
            }
            else if (event.modifiers & static_cast<int>(ftk::KeyModifier::Control))
            {
                event.accept = true;

                if (p.player)
                {
                    const OTIO_NS::RationalTime t = p.player->getCurrentTime();
                    p.player->seek(t + OTIO_NS::RationalTime(event.value.y, t.rate()));
                }
            }
        }

        void Viewport::keyPressEvent(ftk::KeyEvent& event)
        {
            FTK_P();

            const ftk::Box2I& g = getGeometry();
            const ftk::V2I pos(
                event.pos.x - g.min.x,
                (g.h() - 1) - (event.pos.y - g.min.y));

            if (0 == event.modifiers)
            {
                switch (event.key)
                {
                case ftk::Key::_0:
                    event.accept = true;
                    setViewZoom(1.0, pos);
                    break;

                case ftk::Key::Equals:
                    event.accept = true;
                    setViewZoom(p.viewZoom->get() * 2.0, pos);
                    break;

                case ftk::Key::Minus:
                    event.accept = true;
                    setViewZoom(p.viewZoom->get() / 2.0, pos);
                    break;

                case ftk::Key::Backspace:
                    event.accept = true;
                    setFrameView(true);
                    break;

                default: break;
                }
            }
        }

        void Viewport::keyReleaseEvent(ftk::KeyEvent& event)
        {
            event.accept = true;
        }

        bool Viewport::_isMouseInside() const
        {
            return _p->mouse.inside;
        }

        const ftk::V2I& Viewport::_getMousePressPos() const
        {
            return _p->mouse.press;
        }

        ftk::Size2I Viewport::_getRenderSize() const
        {
            FTK_P();
            return timeline::getRenderSize(p.compareOptions->get().compare, p.videoData);
        }

        ftk::V2I Viewport::_getViewportCenter() const
        {
            const ftk::Box2I& g = getGeometry();
            return ftk::V2I(g.w() / 2, g.h() / 2);
        }

        void Viewport::_frameView()
        {
            FTK_P();
            ftk::V2I viewPos;
            double viewZoom = 1.0;
            const ftk::Box2I& g = getGeometry();
            const ftk::Size2I viewportSize = g.size();
            const ftk::Size2I renderSize = _getRenderSize();
            if (renderSize.h > 0 && renderSize.h > 0)
            {
                viewZoom = viewportSize.w / static_cast<double>(renderSize.w);
                if (viewZoom * renderSize.h > viewportSize.h)
                {
                    viewZoom = viewportSize.h / static_cast<double>(renderSize.h);
                }
                const ftk::V2I c(renderSize.w / 2, renderSize.h / 2);
                viewPos = ftk::V2I(
                    viewportSize.w / 2.F - c.x * viewZoom,
                    viewportSize.h / 2.F - c.y * viewZoom);
            }
            if (p.viewPosZoom->setIfChanged(std::make_pair(viewPos, viewZoom)))
            {
                p.viewPos->setIfChanged(viewPos);
                p.viewZoom->setIfChanged(viewZoom);
            }
        }

        void Viewport::_droppedFramesUpdate(const OTIO_NS::RationalTime& value)
        {
            FTK_P();
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
