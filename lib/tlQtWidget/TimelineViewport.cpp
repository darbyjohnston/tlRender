// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineViewport.h>

#include <tlTimeline/GLRender.h>

#include <tlGL/Init.h>
#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>

#include <tlCore/Mesh.h>

#include <QGuiApplication>
#include <QMouseEvent>
#include <QSurfaceFormat>
#include <QWindow>

namespace tl
{
    namespace qtwidget
    {
        struct TimelineViewport::Private
        {
            std::weak_ptr<system::Context> context;
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            QVector<QSharedPointer<qt::TimelinePlayer> > timelinePlayers;
            std::vector<image::Size> timelineSizes;
            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = true;
            enum class MouseMode
            {
                None,
                View,
                Wipe
            };
            MouseMode mouseMode = MouseMode::None;
            math::Vector2i mousePos;
            math::Vector2i mousePress;
            math::Vector2i viewPosMousePress;
            std::vector<timeline::VideoData> videoData;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<tl::gl::Shader> shader;
            std::shared_ptr<tl::gl::OffscreenBuffer> buffer;
            std::shared_ptr<gl::VBO> vbo;
            std::shared_ptr<gl::VAO> vao;
        };

        TimelineViewport::TimelineViewport(
            const std::shared_ptr<system::Context>& context,
            QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            //QSurfaceFormat surfaceFormat;
            //surfaceFormat.setMajorVersion(4);
            //surfaceFormat.setMinorVersion(1);
            //surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            //surfaceFormat.setStencilBufferSize(8);
            //setFormat(surfaceFormat);

            setMouseTracking(true);
            setFocusPolicy(Qt::StrongFocus);
        }

        TimelineViewport::~TimelineViewport()
        {}

        void TimelineViewport::setColorConfigOptions(const timeline::ColorConfigOptions& value)
        {
            TLRENDER_P();
            if (value == p.colorConfigOptions)
                return;
            p.colorConfigOptions = value;
            update();
        }

        void TimelineViewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            update();
        }

        void TimelineViewport::setImageOptions(const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            update();
        }

        void TimelineViewport::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            update();
        }

        void TimelineViewport::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            update();
        }

        void TimelineViewport::setTimelinePlayers(const QVector<QSharedPointer<qt::TimelinePlayer> >& value)
        {
            TLRENDER_P();

            for (const auto& player : p.timelinePlayers)
            {
                if (player)
                {
                    disconnect(
                        player.get(),
                        SIGNAL(currentVideoChanged(const tl::timeline::VideoData&)),
                        this,
                        SLOT(_currentVideoCallback(const tl::timeline::VideoData&)));
                }
            }

            p.timelinePlayers = value;

            p.timelineSizes.clear();
            for (const auto& player : p.timelinePlayers)
            {
                if (player)
                {
                    const auto& ioInfo = player->ioInfo();
                    if (!ioInfo.video.empty())
                    {
                        p.timelineSizes.push_back(ioInfo.video[0].size);
                    }
                }
            }

            p.videoData.clear();
            for (const auto& player : p.timelinePlayers)
            {
                if (player)
                {
                    p.videoData.push_back(player->currentVideo());
                }
            }

            update();

            for (const auto& player : p.timelinePlayers)
            {
                if (player)
                {
                    connect(
                        player.get(),
                        SIGNAL(currentVideoChanged(const tl::timeline::VideoData&)),
                        SLOT(_currentVideoCallback(const tl::timeline::VideoData&)));
                }
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
            update();
            Q_EMIT viewPosAndZoomChanged(p.viewPos, p.viewZoom);
            setFrameView(false);
        }

        void TimelineViewport::setViewZoom(float zoom, const math::Vector2i& focus)
        {
            TLRENDER_P();
            math::Vector2i pos;
            pos.x = focus.x + (p.viewPos.x - focus.x) * (zoom / p.viewZoom);
            pos.y = focus.y + (p.viewPos.y - focus.y) * (zoom / p.viewZoom);
            setViewPosAndZoom(pos, zoom);
        }

        void TimelineViewport::setFrameView(bool value)
        {
            TLRENDER_P();
            if (value == p.frameView)
                return;
            p.frameView = value;
            update();
            Q_EMIT frameViewChanged(p.frameView);
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

        void TimelineViewport::_currentVideoCallback(const timeline::VideoData& value)
        {
            TLRENDER_P();
            if (p.videoData.size() != p.timelinePlayers.size())
            {
                p.videoData = std::vector<timeline::VideoData>(p.timelinePlayers.size());
            }
            for (size_t i = 0; i < p.videoData.size(); ++i)
            {
                if (!p.timelinePlayers[i]->timeRange().contains(p.videoData[i].time))
                {
                    p.videoData[i] = timeline::VideoData();
                }
            }            
            for (size_t i = 0; i < p.timelinePlayers.size(); ++i)
            {
                if (p.timelinePlayers[i] == sender())
                {
                    p.videoData[i] = value;
                }
            }
            update();
        }

        void TimelineViewport::initializeGL()
        {
            TLRENDER_P();

            initializeOpenGLFunctions();
            gl::initGLAD();

            try
            {
                if (auto context = p.context.lock())
                {
                    p.render = timeline::GLRender::create(context);
                }

                const std::string vertexSource =
                    "#version 410\n"
                    "\n"
                    "in vec3 vPos;\n"
                    "in vec2 vTexture;\n"
                    "out vec2 fTexture;\n"
                    "\n"
                    "uniform struct Transform\n"
                    "{\n"
                    "    mat4 mvp;\n"
                    "} transform;\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                    "    fTexture = vTexture;\n"
                    "}\n";
                const std::string fragmentSource =
                    "#version 410\n"
                    "\n"
                    "in vec2 fTexture;\n"
                    "out vec4 fColor;\n"
                    "\n"
                    "uniform sampler2D textureSampler;\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "    fColor = texture(textureSampler, fTexture);\n"
                    "}\n";
                p.shader = gl::Shader::create(vertexSource, fragmentSource);
            }
            catch (const std::exception& e)
            {
                if (auto context = p.context.lock())
                {
                    context->log(
                        "tl::qt::widget::TimelineViewport",
                        e.what(),
                        log::Type::Error);
                }
            }
        }

        void TimelineViewport::resizeGL(int w, int h)
        {
            TLRENDER_P();
            p.vao.reset();
            p.vbo.reset();
        }

        void TimelineViewport::paintGL()
        {
            TLRENDER_P();

            if (p.frameView)
            {
                _frameView();
            }

            const auto renderSize = _renderSize();
            try
            {
                if (renderSize.isValid())
                {
                    gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
                    if (!p.displayOptions.empty())
                    {
                        offscreenBufferOptions.colorFilters = p.displayOptions[0].imageFilters;
                    }
                    offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                    offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
                    if (gl::doCreate(p.buffer, renderSize, offscreenBufferOptions))
                    {
                        p.buffer = gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
                    }
                }
                else
                {
                    p.buffer.reset();
                }

                if (p.buffer)
                {
                    gl::OffscreenBufferBinding binding(p.buffer);
                    p.render->begin(
                        renderSize,
                        p.colorConfigOptions,
                        p.lutOptions); 
                    if (!p.videoData.empty())
                    {
                        p.render->drawVideo(
                            p.videoData,
                            timeline::getBoxes(p.compareOptions.mode, p.timelineSizes),
                            p.imageOptions,
                            p.displayOptions,
                            p.compareOptions);
                    }
                    p.render->end();
                }
            }
            catch (const std::exception& e)
            {
                if (auto context = p.context.lock())
                {
                    context->log(
                        "tl::qt::widget::TimelineViewport",
                        e.what(),
                        log::Type::Error);
                }
            }

            const auto viewportSize = _viewportSize();
            glViewport(
                0,
                0,
                GLsizei(viewportSize.w),
                GLsizei(viewportSize.h));
            glClearColor(0.F, 0.F, 0.F, 0.F);
            glClear(GL_COLOR_BUFFER_BIT);

            if (p.buffer)
            {
                p.shader->bind();
                math::Matrix4x4f vm;
                vm = vm * math::translate(math::Vector3f(p.viewPos.x, p.viewPos.y, 0.F));
                vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));
                const auto pm = math::ortho(
                    0.F,
                    static_cast<float>(viewportSize.w),
                    0.F,
                    static_cast<float>(viewportSize.h),
                    -1.F,
                    1.F);
                p.shader->setUniform("transform.mvp", pm * vm);
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, p.buffer->getColorID());

                const auto mesh = geom::box(math::Box2i(0, 0, renderSize.w, renderSize.h));
                if (!p.vbo)
                {
                    p.vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos2_F32_UV_U16);
                }
                if (p.vbo)
                {
                    p.vbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
                }

                if (!p.vao && p.vbo)
                {
                    p.vao = gl::VAO::create(gl::VBOType::Pos2_F32_UV_U16, p.vbo->getID());
                }
                if (p.vao && p.vbo)
                {
                    p.vao->bind();
                    p.vao->draw(GL_TRIANGLES, 0, p.vbo->getSize());
                }
            }
        }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        void TimelineViewport::enterEvent(QEvent* event)
#else
        void TimelineViewport::enterEvent(QEnterEvent* event)
#endif // QT_VERSION
        {
            TLRENDER_P();
            event->accept();
            p.mouseMode = Private::MouseMode::None;
        }

        void TimelineViewport::leaveEvent(QEvent* event)
        {
            TLRENDER_P();
            event->accept();
            p.mouseMode = Private::MouseMode::None;
        }

        void TimelineViewport::mousePressEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            if (Qt::LeftButton == event->button() && event->modifiers() & Qt::ControlModifier)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                p.mouseMode = Private::MouseMode::View;
                p.mousePress.x = event->x() * devicePixelRatio;
                p.mousePress.y = height() * devicePixelRatio - 1 -
                    event->y() * devicePixelRatio;
                p.viewPosMousePress = p.viewPos;
            }
            else if (Qt::LeftButton == event->button() && event->modifiers() & Qt::AltModifier)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                p.mouseMode = Private::MouseMode::Wipe;
                p.mousePress.x = event->x() * devicePixelRatio;
                p.mousePress.y = height() * devicePixelRatio - 1 -
                    event->y() * devicePixelRatio;
            }
        }

        void TimelineViewport::mouseReleaseEvent(QMouseEvent*)
        {
            TLRENDER_P();
            p.mouseMode = Private::MouseMode::None;
        }

        void TimelineViewport::mouseMoveEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            const float devicePixelRatio = window()->devicePixelRatio();
            p.mousePos.x = event->x() * devicePixelRatio;
            p.mousePos.y = height() * devicePixelRatio - 1 -
                event->y() * devicePixelRatio;
            switch (p.mouseMode)
            {
            case Private::MouseMode::View:
                p.viewPos.x = p.viewPosMousePress.x + (p.mousePos.x - p.mousePress.x);
                p.viewPos.y = p.viewPosMousePress.y + (p.mousePos.y - p.mousePress.y);
                update();
                Q_EMIT viewPosAndZoomChanged(p.viewPos, p.viewZoom);
                setFrameView(false);
                break;
            case Private::MouseMode::Wipe:
                if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
                {
                    const auto& ioInfo = p.timelinePlayers[0]->ioInfo();
                    if (!ioInfo.video.empty())
                    {
                        const auto& imageInfo = ioInfo.video[0];
                        p.compareOptions.wipeCenter.x = (p.mousePos.x - p.viewPos.x) / p.viewZoom /
                            static_cast<float>(imageInfo.size.w * imageInfo.size.pixelAspectRatio);
                        p.compareOptions.wipeCenter.y = 1.F - (p.mousePos.y - p.viewPos.y) / p.viewZoom /
                            static_cast<float>(imageInfo.size.h);
                        update();
                        Q_EMIT compareOptionsChanged(p.compareOptions);
                    }
                }
                break;
            default: break;
            }
        }

        void TimelineViewport::wheelEvent(QWheelEvent* event)
        {
            TLRENDER_P();
            if (Qt::NoModifier == event->modifiers())
            {
                event->accept();
                const float delta = event->angleDelta().y() / 8.F / 15.F;
                const double mult = 1.1;
                const double zoom =
                    delta < 0 ?
                    p.viewZoom / (-delta * mult) :
                    p.viewZoom * (delta * mult);
                setViewZoom(zoom, p.mousePos);
            }
            else if (event->modifiers() & Qt::ControlModifier)
            {
                event->accept();
                if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
                {
                    const auto t = p.timelinePlayers[0]->currentTime();
                    const float delta = event->angleDelta().y() / 8.F / 15.F;
                    p.timelinePlayers[0]->seek(t + otime::RationalTime(delta, t.rate()));
                }
            }
        }

        void TimelineViewport::keyPressEvent(QKeyEvent* event)
        {
            TLRENDER_P();
            switch (event->key())
            {
            case Qt::Key::Key_0:
                event->accept();
                setViewZoom(1.F, p.mousePos);
                break;
            case Qt::Key::Key_Minus:
                event->accept();
                setViewZoom(p.viewZoom / 2.F, p.mousePos);
                break;
            case Qt::Key::Key_Equal:
            case Qt::Key::Key_Plus:
                event->accept();
                setViewZoom(p.viewZoom * 2.F, p.mousePos);
                break;
            case Qt::Key::Key_Backspace:
                event->accept();
                setFrameView(true);
                break;
            }
        }

        math::Size2i TimelineViewport::_viewportSize() const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return math::Size2i(
                width() * devicePixelRatio,
                height() * devicePixelRatio);
        }
        
        math::Vector2i TimelineViewport::_viewportCenter() const
        {
            const math::Size2i viewportSize = _viewportSize();
            return math::Vector2i(viewportSize.w / 2, viewportSize.h / 2);
        }

        math::Size2i TimelineViewport::_renderSize() const
        {
            TLRENDER_P();
            return timeline::getRenderSize(p.compareOptions.mode, p.timelineSizes);
        }

        void TimelineViewport::_frameView()
        {
            TLRENDER_P();
            const math::Size2i viewportSize = _viewportSize();
            const math::Size2i renderSize = _renderSize();
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
                Q_EMIT viewPosAndZoomChanged(p.viewPos, p.viewZoom);
            }
        }
    }
}
