// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlViewApp/SceneView.h>

#include <tlViewApp/TimelineScene.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include <QSurfaceFormat>
#include <QWindow>

#include <glm/gtc/matrix_transform.hpp>

namespace tl
{
    namespace view
    {
        struct SceneView::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<TimelineItem> scene;
            math::Vector2i viewPos = math::Vector2i(0, 0);
            float viewZoom = 1.F;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<tl::gl::Shader> shader;
            std::shared_ptr<tl::gl::OffscreenBuffer> buffer;
            std::shared_ptr<gl::VBO> vbo;
            std::shared_ptr<gl::VAO> vao;
        };

        SceneView::SceneView(
            const std::shared_ptr<system::Context>& context,
            QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;
            p.fontSystem = imaging::FontSystem::create(context);

            QSurfaceFormat surfaceFormat;
            surfaceFormat.setMajorVersion(4);
            surfaceFormat.setMinorVersion(1);
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            setFormat(surfaceFormat);

            setMouseTracking(true);
        }

        SceneView::~SceneView()
        {}

        void SceneView::setScene(const std::shared_ptr<TimelineItem>& scene)
        {
            TLRENDER_P();
            p.scene = scene;
            update();
        }

        void SceneView::initializeGL()
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

        void SceneView::resizeGL(int w, int h)
        {
            TLRENDER_P();
            p.vao.reset();
            p.vbo.reset();
        }

        void SceneView::paintGL()
        {
            TLRENDER_P();

            const imaging::Size renderSize = _viewportSize();
            try
            {
                if (renderSize.isValid())
                {
                    gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
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
                    p.render->begin(renderSize);
                    if (p.scene)
                    {
                        drawScene(p.scene, p.fontSystem, p.render);
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

            const imaging::Size viewportSize = _viewportSize();
            glViewport(
                0,
                0,
                GLsizei(viewportSize.w),
                GLsizei(viewportSize.h));
            glClearColor(.94F, .94F, .94F, 0.F);
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

        imaging::Size SceneView::_viewportSize() const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return imaging::Size(
                width() * devicePixelRatio,
                height() * devicePixelRatio);
        }
    }
}
