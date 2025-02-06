// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineViewport.h>

#include <tlUI/DrawUtil.h>

#include <dtk/gl/GL.h>
#include <dtk/gl/OffscreenBuffer.h>
#include <dtk/gl/Util.h>
#include <dtk/core/Context.h>
#include <dtk/core/LogSystem.h>
#include <dtk/core/RenderUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct TimelineViewport::Private
        {
            timeline::CompareOptions compareOptions;
            std::function<void(timeline::CompareOptions)> compareCallback;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            std::vector<dtk::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::BackgroundOptions backgroundOptions;
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
            std::vector<dtk::V2I> colorPickers;
            std::shared_ptr<dtk::ObservableList<dtk::Color4F> > colorPickerValues;

            bool doRender = false;
            std::shared_ptr<dtk::gl::OffscreenBuffer> buffer;

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

        void TimelineViewport::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimelineViewport", context, parent);
            DTK_P();

            setHStretch(ui::Stretch::Expanding);
            setVStretch(ui::Stretch::Expanding);

            _setMouseHover(true);
            _setMousePress(true);

            p.colorBuffer = dtk::ObservableValue<dtk::ImageType>::create(dtk::ImageType::RGBA_U8);
            p.frameView = dtk::ObservableValue<bool>::create(true);
            p.fps = dtk::ObservableValue<double>::create(0.0);
            p.droppedFrames = dtk::ObservableValue<size_t>::create(0);
            p.colorPickerValues = dtk::ObservableList<dtk::Color4F>::create();
        }

        TimelineViewport::TimelineViewport() :
            _p(new Private)
        {}

        TimelineViewport::~TimelineViewport()
        {}

        std::shared_ptr<TimelineViewport> TimelineViewport::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineViewport>(new TimelineViewport);
            out->_init(context, parent);
            return out;
        }

        void TimelineViewport::setCompareOptions(const timeline::CompareOptions& value)
        {
            DTK_P();
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

        void TimelineViewport::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            DTK_P();
            if (value == p.ocioOptions)
                return;
            p.ocioOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            DTK_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setImageOptions(const std::vector<dtk::ImageOptions>& value)
        {
            DTK_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            DTK_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setBackgroundOptions(const timeline::BackgroundOptions& value)
        {
            DTK_P();
            if (value == p.backgroundOptions)
                return;
            p.backgroundOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        dtk::ImageType TimelineViewport::getColorBuffer() const
        {
            return _p->colorBuffer->get();
        }

        std::shared_ptr<dtk::IObservableValue<dtk::ImageType> > TimelineViewport::observeColorBuffer() const
        {
            return _p->colorBuffer;
        }

        void TimelineViewport::setColorBuffer(dtk::ImageType value)
        {
            DTK_P();
            if (p.colorBuffer->setIfChanged(value))
            {
                p.doRender = true;
                _updates |= ui::Update::Draw;
            }
        }

        void TimelineViewport::setPlayer(const std::shared_ptr<timeline::Player>& value)
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
                        _updates |= ui::Update::Draw;
                    });
            }
            else if (!p.videoData.empty())
            {
                p.videoData.clear();
                p.doRender = true;
                _updates |= ui::Update::Draw;
            }
        }

        const dtk::V2I& TimelineViewport::getViewPos() const
        {
            return _p->viewPos;
        }

        double TimelineViewport::getViewZoom() const
        {
            return _p->viewZoom;
        }

        void TimelineViewport::setViewPosAndZoom(const dtk::V2I& pos, double zoom)
        {
            DTK_P();
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

        void TimelineViewport::setViewZoom(double zoom, const dtk::V2I& focus)
        {
            DTK_P();
            dtk::V2I pos;
            pos.x = focus.x + (p.viewPos.x - focus.x) * (zoom / p.viewZoom);
            pos.y = focus.y + (p.viewPos.y - focus.y) * (zoom / p.viewZoom);
            setViewPosAndZoom(pos, zoom);
        }

        bool TimelineViewport::hasFrameView() const
        {
            return _p->frameView->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > TimelineViewport::observeFrameView() const
        {
            return _p->frameView;
        }

        void TimelineViewport::setFrameView(bool value)
        {
            DTK_P();
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
            DTK_P();
            setViewZoom(1.F, _getViewportCenter());
        }

        void TimelineViewport::viewZoomIn()
        {
            DTK_P();
            setViewZoom(p.viewZoom * 2.0, _getViewportCenter());
        }

        void TimelineViewport::viewZoomOut()
        {
            DTK_P();
            setViewZoom(p.viewZoom / 2.0, _getViewportCenter());
        }

        void TimelineViewport::setViewPosAndZoomCallback(
            const std::function<void(const dtk::V2I&, double)>& value)
        {
            _p->viewPosAndZoomCallback = value;
        }

        double TimelineViewport::getFPS() const
        {
            return _p->fps->get();
        }

        std::shared_ptr<dtk::IObservableValue<double> > TimelineViewport::observeFPS() const
        {
            return _p->fps;
        }

        size_t TimelineViewport::getDroppedFrames() const
        {
            return _p->droppedFrames->get();
        }

        std::shared_ptr<dtk::IObservableValue<size_t> > TimelineViewport::observeDroppedFrames() const
        {
            return _p->droppedFrames;
        }

        void TimelineViewport::setColorPickers(const std::vector<dtk::V2I>& value)
        {
            DTK_P();
            if (value == p.colorPickers)
                return;
            p.colorPickers = value;
            _updates |= ui::Update::Draw;
        }

        std::shared_ptr<dtk::IObservableList<dtk::Color4F> > TimelineViewport::observeColorPickers() const
        {
            return _p->colorPickerValues;
        }

        void TimelineViewport::setGeometry(const dtk::Box2I& value)
        {
            const bool changed = value != _geometry;
            IWidget::setGeometry(value);
            DTK_P();
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
            const dtk::Box2I& drawRect,
            const ui::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            DTK_P();

            if (p.frameView->get())
            {
                _frameView();
            }

            const dtk::Box2I& g = _geometry;
            if (p.doRender)
            {
                p.doRender = false;

                const dtk::ViewportState viewportState(event.render);
                const dtk::ClipRectEnabledState clipRectEnabledState(event.render);
                const dtk::ClipRectState clipRectState(event.render);
                const dtk::TransformState transformState(event.render);
                const dtk::RenderSizeState renderSizeState(event.render);

                try
                {
                    const dtk::Size2I size = g.size();
                    dtk::gl::OffscreenBufferOptions offscreenBufferOptions;
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

                    if (p.buffer)
                    {
                        dtk::gl::OffscreenBufferBinding binding(p.buffer);
                        event.render->setRenderSize(size);
                        event.render->setViewport(dtk::Box2I(0, 0, g.w(), g.h()));
                        event.render->setClipRectEnabled(false);
                        event.render->clearViewport(dtk::Color4F(0.F, 0.F, 0.F));
                        event.render->setOCIOOptions(p.ocioOptions);
                        event.render->setLUTOptions(p.lutOptions);

                        const auto pm = dtk::ortho(
                            0.F,
                            static_cast<float>(g.w()),
                            0.F,
                            static_cast<float>(g.h()),
                            -1.F,
                            1.F);
                        event.render->setTransform(pm);
                        switch (p.backgroundOptions.type)
                        {
                        case timeline::Background::Solid:
                            event.render->drawRect(
                                dtk::Box2I(0, 0, g.w(), g.h()),
                                p.backgroundOptions.color0);
                            break;
                        case timeline::Background::Checkers:
                            event.render->drawColorMesh(
                                dtk::checkers(
                                    dtk::Box2I(0, 0, g.w(), g.h()),
                                    p.backgroundOptions.color0,
                                    p.backgroundOptions.color1,
                                    p.backgroundOptions.checkersSize),
                                dtk::Color4F(1.F, 1.F, 1.F));
                            break;
                        case timeline::Background::Gradient:
                        {
                            dtk::Box2I box(0, 0, g.w(), g.h());
                            dtk::TriMesh2F mesh;
                            mesh.v.push_back(dtk::V2F(box.min.x, box.min.y));
                            mesh.v.push_back(dtk::V2F(box.max.x, box.min.y));
                            mesh.v.push_back(dtk::V2F(box.max.x, box.max.y));
                            mesh.v.push_back(dtk::V2F(box.min.x, box.max.y));
                            mesh.c.push_back(dtk::V4F(
                                p.backgroundOptions.color0.r,
                                p.backgroundOptions.color0.g,
                                p.backgroundOptions.color0.b,
                                p.backgroundOptions.color0.a));
                            mesh.c.push_back(dtk::V4F(
                                p.backgroundOptions.color1.r,
                                p.backgroundOptions.color1.g,
                                p.backgroundOptions.color1.b,
                                p.backgroundOptions.color1.a));
                            mesh.triangles.push_back({
                                dtk::Vertex2(1, 0, 1),
                                dtk::Vertex2(2, 0, 1),
                                dtk::Vertex2(3, 0, 2), });
                            mesh.triangles.push_back({
                                dtk::Vertex2(3, 0, 2),
                                dtk::Vertex2(4, 0, 2),
                                dtk::Vertex2(1, 0, 1), });
                            event.render->drawColorMesh(
                                mesh,
                                dtk::Color4F(1.F, 1.F, 1.F));
                            break;
                        }
                        default: break;
                        }

                        if (!p.videoData.empty())
                        {
                            dtk::M44F vm;
                            vm = vm * dtk::translate(dtk::V3F(p.viewPos.x, p.viewPos.y, 0.F));
                            vm = vm * dtk::scale(dtk::V3F(p.viewZoom, p.viewZoom, 1.F));
                            event.render->setTransform(pm * vm);
                            timeline::BackgroundOptions backgroundOptions;
                            backgroundOptions.color0 = dtk::Color4F(0.F, 0.F, 0.F, 0.F);
                            event.render->drawVideo(
                                p.videoData,
                                timeline::getBoxes(p.compareOptions.mode, p.videoData),
                                p.imageOptions,
                                p.displayOptions,
                                p.compareOptions,
                                backgroundOptions);

                            _droppedFramesUpdate(p.videoData[0].time);
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    if (auto context = _context.lock())
                    {
                        context->log("tl::timelineui::TimelineViewport", e.what(), dtk::LogType::Error);
                    }
                }
            }

            if (p.buffer)
            {
                const unsigned int id = p.buffer->getColorID();
                event.render->drawTexture(id, g);
            }

            if (p.buffer && !p.colorPickers.empty())
            {
                dtk::gl::OffscreenBufferBinding binding(p.buffer);
                std::vector<dtk::Color4F> colors;
                for (const auto& colorPicker : p.colorPickers)
                {
                    const dtk::V2I pos = colorPicker - _geometry.min;
                    std::vector<float> sample(4);
                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                    glReadPixels(
                        pos.x,
                        pos.y,
                        1,
                        1,
                        GL_RGBA,
                        GL_FLOAT,
                        sample.data());
                    colors.push_back(dtk::Color4F(sample[0], sample[1], sample[2], sample[3]));
                }
                p.colorPickerValues->setIfChanged(colors);
            }
        }

        void TimelineViewport::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            DTK_P();
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
                if (p.player)
                {
                    const io::Info& ioInfo = p.player->getIOInfo();
                    if (!ioInfo.video.empty())
                    {
                        const auto& imageInfo = ioInfo.video[0];
                        p.compareOptions.wipeCenter.x = (event.pos.x - _geometry.min.x - p.viewPos.x) / p.viewZoom /
                            static_cast<float>(imageInfo.size.w * imageInfo.pixelAspectRatio);
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
            DTK_P();
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
            DTK_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineViewport::scrollEvent(ui::ScrollEvent& event)
        {
            DTK_P();
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
                if (p.player)
                {
                    const OTIO_NS::RationalTime t = p.player->getCurrentTime();
                    p.player->seek(t + OTIO_NS::RationalTime(event.value.y, t.rate()));
                }
            }
        }

        void TimelineViewport::keyPressEvent(ui::KeyEvent& event)
        {
            DTK_P();
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
            DTK_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        dtk::Size2I TimelineViewport::_getRenderSize() const
        {
            DTK_P();
            return timeline::getRenderSize(p.compareOptions.mode, p.videoData);
        }

        dtk::V2I TimelineViewport::_getViewportCenter() const
        {
            return dtk::V2I(_geometry.w() / 2, _geometry.h() / 2);
        }

        void TimelineViewport::_frameView()
        {
            DTK_P();
            const dtk::Size2I viewportSize(_geometry.w(), _geometry.h());
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

        void TimelineViewport::_droppedFramesUpdate(const OTIO_NS::RationalTime& value)
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
