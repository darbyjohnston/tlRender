// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQt/OutputDevice.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include <tlDevice/IDeviceSystem.h>
#include <tlDevice/IOutputDevice.h>

#include <tlCore/Context.h>
#include <tlCore/Mesh.h>

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <atomic>
#include <iostream>
#include <mutex>

namespace tl
{
    namespace qt
    {
        struct OutputDevice::Private
        {
            std::weak_ptr<system::Context> context;
            std::weak_ptr<device::IDeviceSystem> deviceSystem;

            int deviceIndex = -1;
            int displayModeIndex = -1;
            device::PixelType pixelType = device::PixelType::_8BitBGRA;
            device::HDRMode hdrMode = device::HDRMode::FromFile;
            imaging::HDRData hdrData;

            imaging::ColorConfig colorConfig;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            std::vector<qt::TimelinePlayer*> timelinePlayers;
            std::vector<imaging::Size> sizes;
            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = true;
            std::vector<timeline::VideoData> videoData;
            std::chrono::milliseconds timeout = std::chrono::milliseconds(5);
            QScopedPointer<QOffscreenSurface> offscreenSurface;
            QScopedPointer<QOpenGLContext> glContext;
            std::condition_variable cv;
            std::mutex mutex;
            std::atomic<bool> running;
        };

        OutputDevice::OutputDevice(
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            QThread(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;
            p.deviceSystem = context->getSystem<device::IDeviceSystem>();

            p.glContext.reset(new QOpenGLContext);
            QSurfaceFormat surfaceFormat;
            surfaceFormat.setMajorVersion(4);
            surfaceFormat.setMinorVersion(1);
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            p.glContext->setFormat(surfaceFormat);
            p.glContext->create();

            p.offscreenSurface.reset(new QOffscreenSurface);
            p.offscreenSurface->setFormat(p.glContext->format());
            p.offscreenSurface->create();

            p.glContext->moveToThread(this);

            p.running = true;
            start();
        }

        OutputDevice::~OutputDevice()
        {
            TLRENDER_P();
            p.running = false;
            wait();
        }

        void OutputDevice::setDevice(
            int deviceIndex,
            int displayModeIndex,
            device::PixelType pixelType)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.deviceIndex = deviceIndex;
                p.displayModeIndex = displayModeIndex;
                p.pixelType = pixelType;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setColorConfig(const imaging::ColorConfig& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.colorConfig = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setImageOptions(const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.imageOptions = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.displayOptions = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setHDR(device::HDRMode hdrMode, const imaging::HDRData& hdrData)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.hdrMode = hdrMode;
                p.hdrData = hdrData;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.compareOptions = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& value)
        {
            TLRENDER_P();
            if (value == p.timelinePlayers)
                return;
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
                connect(
                    i,
                    SIGNAL(videoChanged(const tl::timeline::VideoData&)),
                    SLOT(_videoCallback(const tl::timeline::VideoData&)));
            }
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.sizes.clear();
                p.videoData.clear();
                for (const auto& i : p.timelinePlayers)
                {
                    const auto& ioInfo = i->ioInfo();
                    if (!ioInfo.video.empty())
                    {
                        p.sizes.push_back(ioInfo.video[0].size);
                    }
                    p.videoData.push_back(i->video());
                }
            }
        }

        void OutputDevice::setView(
            const tl::math::Vector2i& pos,
            float                     zoom,
            bool                      frame)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.viewPos = pos;
                p.viewZoom = zoom;
                p.frameView = frame;
            }
            p.cv.notify_one();
        }

        void OutputDevice::_videoCallback(const tl::timeline::VideoData& value)
        {
            TLRENDER_P();
            const auto i = std::find(p.timelinePlayers.begin(), p.timelinePlayers.end(), sender());
            if (i != p.timelinePlayers.end())
            {
                const size_t index = i - p.timelinePlayers.begin();
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.videoData[index] = value;
                }
                p.cv.notify_one();
            }
        }

        namespace
        {
            imaging::PixelType getOffscreenType(device::PixelType value)
            {
                const std::array<imaging::PixelType, static_cast<size_t>(device::PixelType::Count)> data =
                {
                    imaging::PixelType::None,
                    imaging::PixelType::RGBA_U8,
                    imaging::PixelType::RGB_U10
                };
                return data[static_cast<size_t>(value)];
            }

            GLenum getReadPixelsFormat(device::PixelType value)
            {
                const std::array<GLenum, static_cast<size_t>(device::PixelType::Count)> data =
                {
                    GL_NONE,
                    GL_BGRA,
                    GL_RGBA
                };
                return data[static_cast<size_t>(value)];
            }

            GLenum getReadPixelsType(device::PixelType value)
            {
                const std::array<GLenum, static_cast<size_t>(device::PixelType::Count)> data =
                {
                    GL_NONE,
                    GL_UNSIGNED_BYTE,
                    GL_UNSIGNED_INT_10_10_10_2
                };
                return data[static_cast<size_t>(value)];
            }

            GLint getReadPixelsAlign(device::PixelType value)
            {
                const std::array<GLint, static_cast<size_t>(device::PixelType::Count)> data =
                {
                    0,
                    4,
                    256
                };
                return data[static_cast<size_t>(value)];
            }

            GLint getReadPixelsSwap(device::PixelType value)
            {
                const std::array<GLint, static_cast<size_t>(device::PixelType::Count)> data =
                {
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE
                };
                return data[static_cast<size_t>(value)];
            }
        }

        void OutputDevice::run()
        {
            TLRENDER_P();

            p.glContext->makeCurrent(p.offscreenSurface.get());
            gladLoaderLoadGL();

            std::shared_ptr<timeline::IRender> render;
            if (auto context = p.context.lock())
            {
                render = gl::Render::create(context);
            }

            int deviceIndex = -1;
            int displayModeIndex = -1;
            device::PixelType pixelType = device::PixelType::None;
            imaging::ColorConfig colorConfig;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            device::HDRMode hdrMode = device::HDRMode::FromFile;
            imaging::HDRData hdrData;
            timeline::CompareOptions compareOptions;
            std::vector<imaging::Size> sizes;
            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = true;
            std::vector<timeline::VideoData> videoData;

            std::shared_ptr<device::IOutputDevice> device;
            std::shared_ptr<tl::gl::Shader> shader;
            std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;
            std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer2;
            std::shared_ptr<gl::VBO> vbo;
            std::shared_ptr<gl::VAO> vao;
            std::array<GLuint, 1> pbo;
            std::array<otime::RationalTime, 1> pboTime;
            size_t pboIndex = 0;
            while (p.running)
            {
                bool doCreateDevice = false;
                bool doRender = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    if (p.cv.wait_for(
                        lock,
                        p.timeout,
                        [this, deviceIndex, displayModeIndex, pixelType,
                        colorConfig, imageOptions, displayOptions, hdrMode, hdrData,
                        compareOptions, sizes, viewPos, viewZoom, frameView,
                        videoData]
                        {
                            return
                                deviceIndex != _p->deviceIndex ||
                                displayModeIndex != _p->displayModeIndex ||
                                pixelType != _p->pixelType ||
                                colorConfig != _p->colorConfig ||
                                imageOptions != _p->imageOptions ||
                                displayOptions != _p->displayOptions ||
                                hdrMode != _p->hdrMode ||
                                hdrData != _p->hdrData ||
                                compareOptions != _p->compareOptions ||
                                sizes != _p->sizes ||
                                viewPos != _p->viewPos ||
                                viewZoom != _p->viewZoom ||
                                frameView != _p->frameView ||
                                videoData != _p->videoData;
                        }))
                    {
                        if (p.deviceIndex != deviceIndex ||
                            p.displayModeIndex != displayModeIndex ||
                            p.pixelType != pixelType)
                        {
                            doCreateDevice = true;
                        }
                        deviceIndex = p.deviceIndex;
                        displayModeIndex = p.displayModeIndex;
                        pixelType = p.pixelType;

                        doRender = true;
                        colorConfig = p.colorConfig;
                        imageOptions = p.imageOptions;
                        displayOptions = p.displayOptions;
                        hdrMode = p.hdrMode;
                        hdrData = p.hdrData;
                        compareOptions = p.compareOptions;
                        sizes = p.sizes;
                        viewPos = p.viewPos;
                        viewZoom = p.viewZoom;
                        frameView = p.frameView;
                        videoData = p.videoData;
                    }
                }

                if (doCreateDevice)
                {
                    offscreenBuffer2.reset();
                    offscreenBuffer.reset();
                    device.reset();
                    if (deviceIndex != -1 && displayModeIndex != -1 && pixelType != device::PixelType::None)
                    {
                        if (auto deviceSystem = p.deviceSystem.lock())
                        {
                            try
                            {
                                device = deviceSystem->createDevice(deviceIndex, displayModeIndex, pixelType);
                            }
                            catch (const std::exception& e)
                            {
                                if (auto context = p.context.lock())
                                {
                                    context->log("tl::qt::OutputDevice", e.what(), log::Type::Error);
                                }
                            }
                        }
                    }

                    vao.reset();
                    vbo.reset();

                    glDeleteBuffers(pbo.size(), pbo.data());
                    glGenBuffers(pbo.size(), pbo.data());
                    if (device)
                    {
                        const imaging::Size viewportSize = device->getSize();
                        for (size_t i = 0; i < pbo.size(); ++i)
                        {
                            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
                            glBufferData(
                                GL_PIXEL_PACK_BUFFER,
                                device::getDataByteCount(viewportSize, pixelType),
                                NULL,
                                GL_STREAM_COPY);
                        }
                    }
                }

                if (doRender && render && device)
                {
                    try
                    {
                        const imaging::Size renderSize = timeline::getRenderSize(_p->compareOptions.mode, sizes);
                        if (gl::doCreate(offscreenBuffer, renderSize))
                        {
                            gl::OffscreenBufferOptions options;
                            options.colorType = imaging::PixelType::RGBA_F32;
                            options.depth = gl::OffscreenDepth::_24;
                            options.stencil = gl::OffscreenStencil::_8;
                            offscreenBuffer = gl::OffscreenBuffer::create(renderSize, options);
                        }

                        if (offscreenBuffer)
                        {
                            gl::OffscreenBufferBinding binding(offscreenBuffer);

                            render->setColorConfig(colorConfig);
                            render->begin(renderSize);
                            render->drawVideo(
                                videoData,
                                timeline::tiles(compareOptions.mode, sizes),
                                { imageOptions },
                                { displayOptions },
                                compareOptions);
                            render->end();
                        }

                        const imaging::Size viewportSize = device->getSize();
                        if (gl::doCreate(offscreenBuffer2, viewportSize))
                        {
                            gl::OffscreenBufferOptions options;
                            options.colorType = getOffscreenType(pixelType);
                            offscreenBuffer2 = gl::OffscreenBuffer::create(viewportSize, options);
                        }

                        math::Vector2i viewPosTmp = viewPos;
                        float viewZoomTmp = viewZoom;
                        if (frameView)
                        {
                            float zoom = viewportSize.w / static_cast<float>(renderSize.w);
                            if (zoom * renderSize.h > viewportSize.h)
                            {
                                zoom = viewportSize.h / static_cast<float>(renderSize.h);
                            }
                            const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
                            viewPosTmp.x = viewportSize.w / 2.F - c.x * zoom;
                            viewPosTmp.y = viewportSize.h / 2.F - c.y * zoom;
                            viewZoomTmp = zoom;
                        }

                        if (!shader)
                        {
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
                            shader = gl::Shader::create(vertexSource, fragmentSource);
                        }

                        if (offscreenBuffer && offscreenBuffer2)
                        {
                            gl::OffscreenBufferBinding binding(offscreenBuffer2);

                            glViewport(
                                0,
                                0,
                                GLsizei(viewportSize.w),
                                GLsizei(viewportSize.h));
                            glClearColor(0.F, 0.F, 0.F, 0.F);
                            glClear(GL_COLOR_BUFFER_BIT);

                            shader->bind();
                            glm::mat4x4 vm(1.F);
                            vm = glm::translate(vm, glm::vec3(viewPosTmp.x, viewPosTmp.y, 0.F));
                            vm = glm::scale(vm, glm::vec3(viewZoomTmp, viewZoomTmp, 1.F));
                            const glm::mat4x4 pm = glm::ortho(
                                0.F,
                                static_cast<float>(viewportSize.w),
                                0.F,
                                static_cast<float>(viewportSize.h),
                                -1.F,
                                1.F);
                            const glm::mat4x4 vpm = pm * vm;
                            shader->setUniform(
                                "transform.mvp",
                                math::Matrix4x4f(
                                    vpm[0][0], vpm[0][1], vpm[0][2], vpm[0][3],
                                    vpm[1][0], vpm[1][1], vpm[1][2], vpm[1][3],
                                    vpm[2][0], vpm[2][1], vpm[2][2], vpm[2][3],
                                    vpm[3][0], vpm[3][1], vpm[3][2], vpm[3][3]));
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, offscreenBuffer->getColorID());

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
                            if (!vbo)
                            {
                                vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos3_F32_UV_U16);
                            }
                            if (vbo)
                            {
                                vbo->copy(vboData);
                            }

                            if (!vao && vbo)
                            {
                                vao = gl::VAO::create(gl::VBOType::Pos3_F32_UV_U16, vbo->getID());
                            }
                            if (vao && vbo)
                            {
                                vao->bind();
                                vao->draw(GL_TRIANGLES, 0, vbo->getSize());
                            }

                            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[pboIndex % pbo.size()]);
                            pboTime[pboIndex % pbo.size()] = !videoData.empty() ? videoData[0].time : time::invalidTime;
                            if (0 == viewportSize.w % getReadPixelsAlign(pixelType) &&
                                !getReadPixelsSwap(pixelType))
                            {
                                glBindTexture(GL_TEXTURE_2D, offscreenBuffer2->getColorID());
                                glGetTexImage(
                                    GL_TEXTURE_2D,
                                    0,
                                    getReadPixelsFormat(pixelType),
                                    getReadPixelsType(pixelType),
                                    NULL);
                            }
                            else
                            {
                                glPixelStorei(GL_PACK_ALIGNMENT, getReadPixelsAlign(pixelType));
                                glPixelStorei(GL_PACK_SWAP_BYTES, getReadPixelsSwap(pixelType));
                                glReadPixels(
                                    0,
                                    0,
                                    viewportSize.w,
                                    viewportSize.h,
                                    getReadPixelsFormat(pixelType),
                                    getReadPixelsType(pixelType),
                                    NULL);
                            }

                            ++pboIndex;
                            if (pbo[pboIndex % pbo.size()])
                            {
                                auto pixelData = device::PixelData::create(
                                    viewportSize,
                                    pixelType,
                                    pboTime[pboIndex % pbo.size()]);
                                //std::cout << "time: " << pixelData->getTime() << std::endl;

                                std::shared_ptr<imaging::HDRData> hdrDataP;
                                switch (hdrMode)
                                {
                                case device::HDRMode::FromFile:
                                    if (!videoData.empty())
                                    {
                                        hdrDataP = device::getHDRData(videoData[0]);
                                    }
                                    break;
                                case device::HDRMode::Custom:
                                    hdrDataP.reset(new imaging::HDRData(hdrData));
                                    break;
                                default: break;
                                }
                                pixelData->setHDRData(hdrDataP);

                                glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[pboIndex % pbo.size()]);
                                if (void* buffer = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY))
                                {
                                    memcpy(
                                        pixelData->getData(),
                                        buffer,
                                        pixelData->getDataByteCount());
                                    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                                }
                                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                                device->display(pixelData);
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        if (auto context = p.context.lock())
                        {
                            context->log("tl::qt::OutputDevice", e.what(), log::Type::Error);
                        }
                    }
                }
            }
            glDeleteBuffers(pbo.size(), pbo.data());
        }
    }
}
