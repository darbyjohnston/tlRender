// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQWidget/TimelineViewport.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>

#include <tlCore/Mesh.h>

#include <QGuiApplication>
#include <QMouseEvent>
#include <QSurfaceFormat>

#include <glm/gtc/matrix_transform.hpp>

namespace tl
{
    namespace qwidget
    {
        struct TimelineViewport::Private
        {
            std::weak_ptr<core::Context> context;
            imaging::ColorConfig colorConfig;
            std::vector<render::ImageOptions> imageOptions;
            render::CompareOptions compareOptions;
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
            std::shared_ptr<render::IRender> render;
            std::shared_ptr<tl::gl::Shader> shader;
            std::shared_ptr<tl::gl::OffscreenBuffer> buffer;
        };

        TimelineViewport::TimelineViewport(
            const std::shared_ptr<core::Context>& context,
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

        void TimelineViewport::setColorConfig(const imaging::ColorConfig & colorConfig)
        {
            if (colorConfig == _p->colorConfig)
                return;
            _p->colorConfig = colorConfig;
            update();
        }

        void TimelineViewport::setImageOptions(const std::vector<render::ImageOptions>& options)
        {
            TLRENDER_P();
            if (options == p.imageOptions)
                return;
            p.imageOptions = options;
            update();
        }

        void TimelineViewport::setCompareOptions(const render::CompareOptions& options)
        {
            TLRENDER_P();
            if (options == p.compareOptions)
                return;
            p.compareOptions = options;
            update();
        }

        void TimelineViewport::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& timelinePlayers)
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
            p.timelinePlayers = timelinePlayers;
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

        void TimelineViewport::setViewPosAndZoom(const math::Vector2i& pos, float zoom)
        {
            TLRENDER_P();
            if (pos == p.viewPos && zoom == p.viewZoom)
                return;
            p.viewPos = pos;
            p.viewZoom = zoom;
            p.frameView = false;
            update();
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
            setViewZoom(1.F, p.mouseInside ? p.mousePos : _center());
        }

        void TimelineViewport::viewZoomIn()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom * 2.F, p.mouseInside ? p.mousePos : _center());
        }

        void TimelineViewport::viewZoomOut()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom / 2.F, p.mouseInside ? p.mousePos : _center());
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
                    "// Inputs\n"
                    "in vec3 vPos;\n"
                    "in vec2 vTexture;\n"
                    "\n"
                    "// Outputs\n"
                    "out vec2 fTexture;\n"
                    "\n"
                    "// Uniforms\n"
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
                    "// Inputs\n"
                    "in vec2 fTexture;\n"
                    "\n"
                    "// Outputs\n"
                    "out vec4 fColor;\n"
                    "\n"
                    "// Uniforms\n"
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
                        "tl::qwidget::TimelineViewport",
                        e.what(),
                        core::LogType::Error);
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
        }

        void TimelineViewport::paintGL()
        {
            TLRENDER_P();

            imaging::Info info;
            if (!p.timelinePlayers.empty())
            {
                const auto& avInfo = p.timelinePlayers[0]->avInfo();
                if (!avInfo.video.empty())
                {
                    info = avInfo.video[0];
                }
            }

            try
            {
                if (info.size.isValid())
                {
                    if (!p.buffer ||
                        (p.buffer && p.buffer->getSize() != info.size))
                    {
                        gl::OffscreenBufferOptions options;
                        options.colorType = imaging::PixelType::RGBA_F32;
                        options.depth = gl::OffscreenDepth::_24;
                        options.stencil = gl::OffscreenStencil::_8;
                        p.buffer = gl::OffscreenBuffer::create(info.size, options);
                    }
                }
                else
                {
                    p.buffer.reset();
                }

                p.render->setColorConfig(p.colorConfig);

                if (p.buffer)
                {
                    gl::OffscreenBufferBinding binding(p.buffer);
                    p.render->begin(info.size);
                    p.render->drawVideo(p.videoData, p.imageOptions, p.compareOptions);
                    p.render->end();
                }
            }
            catch (const std::exception& e)
            {
                if (auto context = p.context.lock())
                {
                    context->log(
                        "tl::qwidget::TimelineViewport",
                        e.what(),
                        core::LogType::Error);
                }
            }

            float devicePixelRatio = 1.F;
            if (auto app = qobject_cast<QGuiApplication*>(QGuiApplication::instance()))
            {
                devicePixelRatio = app->devicePixelRatio();
            }
            const auto size = imaging::Size(
                width() * devicePixelRatio,
                height() * devicePixelRatio);
            glViewport(
                0,
                0,
                GLsizei(size.w),
                GLsizei(size.h));
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
                    static_cast<float>(size.w),
                    0.F,
                    static_cast<float>(size.h),
                    -1.F,
                    1.F);
                const glm::mat4x4 vpm = pm * vm;
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
                mesh.v.push_back(math::Vector3f(info.size.w, 0.F, 0.F));
                mesh.t.push_back(math::Vector2f(1.F, 0.F));
                mesh.v.push_back(math::Vector3f(info.size.w, info.size.h, 0.F));
                mesh.t.push_back(math::Vector2f(1.F, 1.F));
                mesh.v.push_back(math::Vector3f(0.F, info.size.h, 0.F));
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
                auto vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos3_F32_UV_U16);
                vbo->copy(vboData);
                auto vao = gl::VAO::create(gl::VBOType::Pos3_F32_UV_U16, vbo->getID());
                vao->bind();
                vao->draw(GL_TRIANGLES, 0, mesh.triangles.size() * 3);
            }
        }

        void TimelineViewport::enterEvent(QEvent*)
        {
            TLRENDER_P();
            p.mouseInside = true;
        }

        void TimelineViewport::leaveEvent(QEvent*)
        {
            TLRENDER_P();
            p.mouseInside = false;
        }

        void TimelineViewport::mousePressEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            p.mousePressed = true;
            p.mousePress.x = event->x();
            p.mousePress.y = height() - 1 - event->y();
            p.viewPosMousePress = p.viewPos;
        }

        void TimelineViewport::mouseReleaseEvent(QMouseEvent*)
        {
            TLRENDER_P();
            p.mousePressed = false;
        }

        void TimelineViewport::mouseMoveEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            p.mousePos.x = event->x();
            p.mousePos.y = height() - 1 - event->y();
            if (p.mousePressed)
            {
                p.viewPos.x = p.viewPosMousePress.x + p.mousePos.x - p.mousePress.x;
                p.viewPos.y = p.viewPosMousePress.y + p.mousePos.y - p.mousePress.y;
                p.frameView = false;
                update();
            }
        }

        void TimelineViewport::_frameView()
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty())
            {
                const auto& avInfo = p.timelinePlayers[0]->avInfo();
                if (!avInfo.video.empty())
                {
                    const auto& info = avInfo.video[0];
                    float zoom = width() / static_cast<float>(info.size.w);
                    if (zoom * info.size.h > height())
                    {
                        zoom = height() / static_cast<float>(info.size.h);
                    }
                    math::Vector2i c(info.size.w / 2, info.size.h / 2);
                    p.viewPos.x = width() / 2.F - c.x * zoom;
                    p.viewPos.y = height() / 2.F - c.y * zoom;
                    p.viewZoom = zoom;
                    update();
                }
            }
        }

        math::Vector2i TimelineViewport::_center() const
        {
            return math::Vector2i(width() / 2, height() / 2);
        }
    }
}