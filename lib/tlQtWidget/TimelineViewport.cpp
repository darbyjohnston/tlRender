// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineViewport.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include <tlCore/Mesh.h>

#include <QGuiApplication>
#include <QMouseEvent>
#include <QSurfaceFormat>
#include <QWindow>

#include <glm/gtc/matrix_transform.hpp>

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
            std::vector<qt::TimelinePlayer*> timelinePlayers;
            std::vector<imaging::Size> timelineSizes;
            std::vector<imaging::Size> timelineSizesTmp;
            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = true;
            bool mouseInside = false;
            bool mousePressed = false;
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

            QSurfaceFormat surfaceFormat;
            surfaceFormat.setMajorVersion(4);
            surfaceFormat.setMinorVersion(1);
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            surfaceFormat.setStencilBufferSize(8);
            setFormat(surfaceFormat);

            setMouseTracking(true);
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

        void TimelineViewport::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& value)
        {
            TLRENDER_P();
            for (const auto& i : p.timelinePlayers)
            {
                disconnect(
                    i,
                    SIGNAL(currentVideoChanged(const tl::timeline::VideoData&)),
                    this,
                    SLOT(_currentVideoCallback(const tl::timeline::VideoData&)));
            }
            p.timelinePlayers = value;
            p.timelineSizesTmp.clear();
            for (const auto& i : p.timelinePlayers)
            {
                const auto& ioInfo = i->ioInfo();
                if (!ioInfo.video.empty())
                {
                    p.timelineSizesTmp.push_back(ioInfo.video[0].size);
                }
                connect(
                    i,
                    SIGNAL(currentVideoChanged(const tl::timeline::VideoData&)),
                    SLOT(_currentVideoCallback(const tl::timeline::VideoData&)));
            }
            if (p.timelinePlayers.empty())
            {
                p.videoData.clear();
                update();
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
            update();
            Q_EMIT viewPosAndZoomChanged(p.viewPos, p.viewZoom);
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
            update();
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

        void TimelineViewport::_currentVideoCallback(const timeline::VideoData& value)
        {
            TLRENDER_P();
            p.timelineSizes = p.timelineSizesTmp;
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
            const auto i = std::find(p.timelinePlayers.begin(), p.timelinePlayers.end(), sender());
            if (i != p.timelinePlayers.end())
            {
                const size_t index = i - p.timelinePlayers.begin();
                _p->videoData[index] = value;
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
                    p.render = gl::Render::create(context);
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
                    offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
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
                    p.render->setColorConfig(p.colorConfigOptions);
                    p.render->setLUT(p.lutOptions);
                    p.render->begin(renderSize);
                    p.render->drawVideo(
                        p.videoData,
                        timeline::tiles(p.compareOptions.mode, p.timelineSizes),
                        p.imageOptions,
                        p.displayOptions,
                        p.compareOptions);
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
                glm::mat4x4 vm(1.F);
                vm = glm::translate(vm, glm::vec3(p.viewPos.x, p.viewPos.y, 0.F));
                vm = glm::scale(vm, glm::vec3(p.viewZoom, p.viewZoom, 1.F));
                const glm::mat4x4 pm = glm::ortho(
                    0.F,
                    static_cast<float>(viewportSize.w),
                    0.F,
                    static_cast<float>(viewportSize.h),
                    -1.F,
                    1.F);
                glm::mat4x4 vpm = pm * vm;
                p.shader->setUniform(
                    "transform.mvp",
                    math::Matrix4x4f(
                        vpm[0][0], vpm[0][1], vpm[0][2], vpm[0][3],
                        vpm[1][0], vpm[1][1], vpm[1][2], vpm[1][3],
                        vpm[2][0], vpm[2][1], vpm[2][2], vpm[2][3],
                        vpm[3][0], vpm[3][1], vpm[3][2], vpm[3][3]));
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, p.buffer->getColorID());

                const auto mesh = geom::bbox(math::BBox2i(0, 0, renderSize.w, renderSize.h));
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
            p.mouseInside = true;
            p.mousePressed = false;
        }

        void TimelineViewport::leaveEvent(QEvent* event)
        {
            TLRENDER_P();
            event->accept();
            p.mouseInside = false;
            p.mousePressed = false;
        }

        void TimelineViewport::mousePressEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            if (Qt::LeftButton == event->button() && event->modifiers() & Qt::ControlModifier)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                p.mousePressed = true;
                p.mousePress.x = event->x() * devicePixelRatio;
                p.mousePress.y = height() * devicePixelRatio - 1 -
                    event->y() * devicePixelRatio;
                p.viewPosMousePress = p.viewPos;
            }
        }

        void TimelineViewport::mouseReleaseEvent(QMouseEvent*)
        {
            TLRENDER_P();
            p.mousePressed = false;
        }

        void TimelineViewport::mouseMoveEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            const float devicePixelRatio = window()->devicePixelRatio();
            p.mousePos.x = event->x() * devicePixelRatio;
            p.mousePos.y = height() * devicePixelRatio - 1 -
                event->y() * devicePixelRatio;
            if (p.mousePressed)
            {
                p.viewPos.x = p.viewPosMousePress.x + (p.mousePos.x - p.mousePress.x);
                p.viewPos.y = p.viewPosMousePress.y + (p.mousePos.y - p.mousePress.y);
                p.frameView = false;
                update();
                Q_EMIT viewPosAndZoomChanged(p.viewPos, p.viewZoom);
            }
        }

        void TimelineViewport::wheelEvent(QWheelEvent* event)
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty())
            {
                const auto t = p.timelinePlayers[0]->currentTime();
                const float delta = event->angleDelta().y() / 8.F / 15.F;
                p.timelinePlayers[0]->seek(t + otime::RationalTime(delta, t.rate()));
            }
        }

        imaging::Size TimelineViewport::_viewportSize() const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return imaging::Size(
                width() * devicePixelRatio,
                height() * devicePixelRatio);
        }

        imaging::Size TimelineViewport::_renderSize() const
        {
            TLRENDER_P();
            return timeline::getRenderSize(p.compareOptions.mode, p.timelineSizes);
        }

        math::Vector2i TimelineViewport::_viewportCenter() const
        {
            const auto viewportSize = _viewportSize();
            return math::Vector2i(viewportSize.w / 2, viewportSize.h / 2);
        }

        void TimelineViewport::_frameView()
        {
            TLRENDER_P();
            const auto viewportSize = _viewportSize();
            const auto renderSize = _renderSize();
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
                Q_EMIT frameViewActivated();
            }
        }
    }
}
