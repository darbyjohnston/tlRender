// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
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
            imaging::ColorConfig colorConfig;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            std::vector<qt::TimelinePlayer*> timelinePlayers;
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

        void TimelineViewport::setColorConfig(const imaging::ColorConfig& value)
        {
            TLRENDER_P();
            if (value == p.colorConfig)
                return;
            p.colorConfig = value;
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
            p.videoData.clear();
            for (const auto& i : p.timelinePlayers)
            {
                disconnect(
                    i,
                    SIGNAL(videoChanged(const tl::timeline::VideoData&)),
                    this,
                    SLOT(_videoCallback(const tl::timeline::VideoData&)));
            }
            p.timelinePlayers = value;
            for (const auto& i : p.timelinePlayers)
            {
                _p->videoData.push_back(i->video());
                connect(
                    i,
                    SIGNAL(videoChanged(const tl::timeline::VideoData&)),
                    SLOT(_videoCallback(const tl::timeline::VideoData&)));
            }
            if (p.frameView)
            {
                _frameView();
            }
            update();
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
            _frameView();
            update();
        }

        void TimelineViewport::viewZoom1To1()
        {
            TLRENDER_P();
            setViewZoom(1.F, p.mouseInside ? p.mousePos : _getViewportCenter());
        }

        void TimelineViewport::viewZoomIn()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom * 2.F, p.mouseInside ? p.mousePos : _getViewportCenter());
        }

        void TimelineViewport::viewZoomOut()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom / 2.F, p.mouseInside ? p.mousePos : _getViewportCenter());
        }

        void TimelineViewport::_videoCallback(const timeline::VideoData& value)
        {
            TLRENDER_P();
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
            try
            {
                gladLoaderLoadGL();

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

            if (p.frameView)
            {
                _frameView();
            }

            p.vao.reset();
            p.vbo.reset();
        }

        void TimelineViewport::paintGL()
        {
            TLRENDER_P();

            const auto renderSize = _getRenderSize();
            try
            {
                if (renderSize.isValid())
                {
                    gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
                    if (!p.displayOptions.empty())
                    {
                        offscreenBufferOptions.colorMinifyFilter = gl::getTextureFilter(p.displayOptions[0].imageFilters.minify);
                        offscreenBufferOptions.colorMagnifyFilter = gl::getTextureFilter(p.displayOptions[0].imageFilters.magnify);
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
                    p.render->setColorConfig(p.colorConfig);
                    p.render->begin(renderSize);
                    p.render->drawVideo(
                        p.videoData,
                        timeline::tiles(p.compareOptions.mode, _getTimelineSizes()),
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

            const auto viewportSize = _getViewportSize();
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

                geom::TriangleMesh3 mesh;
                mesh.v.push_back(math::Vector3f(0.F, 0.F, 0.F));
                mesh.t.push_back(math::Vector2f(0.F, 0.F));
                mesh.v.push_back(math::Vector3f(renderSize.w, 0.F, 0.F));
                mesh.t.push_back(math::Vector2f(1.F, 0.F));
                mesh.v.push_back(math::Vector3f(renderSize.w, renderSize.h, 0.F));
                mesh.t.push_back(math::Vector2f(1.F, 1.F));
                mesh.v.push_back(math::Vector3f(0.F, renderSize.h, 0.F));
                mesh.t.push_back(math::Vector2f(0.F, 1.F));
                mesh.triangles.push_back(geom::Triangle3({
                    geom::Vertex3({ 1, 1, 0 }),
                    geom::Vertex3({ 2, 2, 0 }),
                    geom::Vertex3({ 3, 3, 0 })
                    }));
                mesh.triangles.push_back(geom::Triangle3({
                    geom::Vertex3({ 3, 3, 0 }),
                    geom::Vertex3({ 4, 4, 0 }),
                    geom::Vertex3({ 1, 1, 0 })
                    }));
                auto vboData = convert(
                    mesh,
                    gl::VBOType::Pos3_F32_UV_U16,
                    math::SizeTRange(0, mesh.triangles.size() - 1));
                if (!p.vbo)
                {
                    p.vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos3_F32_UV_U16);
                }
                if (p.vbo)
                {
                    p.vbo->copy(vboData);
                }

                if (!p.vao && p.vbo)
                {
                    p.vao = gl::VAO::create(gl::VBOType::Pos3_F32_UV_U16, p.vbo->getID());
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
#endif
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
        
        imaging::Size TimelineViewport::_getViewportSize() const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return imaging::Size(
                width() * devicePixelRatio,
                height() * devicePixelRatio);
        }

        std::vector<imaging::Size> TimelineViewport::_getTimelineSizes() const
        {
            TLRENDER_P();
            std::vector<imaging::Size> sizes;
            for (const auto& i : p.timelinePlayers)
            {
                const auto& ioInfo = i->ioInfo();
                if (!ioInfo.video.empty())
                {
                    sizes.push_back(ioInfo.video[0].size);
                }
            }
            return sizes;
        }

        imaging::Size TimelineViewport::_getRenderSize() const
        {
            return timeline::getRenderSize(_p->compareOptions.mode, _getTimelineSizes());
        }

        math::Vector2i TimelineViewport::_getViewportCenter() const
        {
            const auto viewportSize = _getViewportSize();
            return math::Vector2i(viewportSize.w / 2, viewportSize.h / 2);
        }

        void TimelineViewport::_frameView()
        {
            TLRENDER_P();
            const auto viewportSize = _getViewportSize();
            const auto renderSize = _getRenderSize();
            float zoom = viewportSize.w / static_cast<float>(renderSize.w);
            if (zoom * renderSize.h > viewportSize.h)
            {
                zoom = viewportSize.h / static_cast<float>(renderSize.h);
            }
            const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
            p.viewPos.x = viewportSize.w / 2.F - c.x * zoom;
            p.viewPos.y = viewportSize.h / 2.F - c.y * zoom;
            p.viewZoom = zoom;
            update();
            Q_EMIT viewPosAndZoomChanged(p.viewPos, p.viewZoom);
            Q_EMIT frameViewActivated();
        }
    }
}
