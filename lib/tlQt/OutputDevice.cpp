// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQt/OutputDevicePrivate.h>

#include <tlTimeline/GLRender.h>

#include <tlGL/Init.h>
#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>
#include <tlGL/Texture.h>

#include <tlDevice/IOutputDevice.h>

#include <tlCore/Context.h>
#include <tlCore/Mesh.h>

#include <QSurfaceFormat>

#include <array>
#include <atomic>
#include <iostream>
#include <mutex>

namespace tl
{
    namespace qt
    {
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

        int OutputDevice::getDeviceIndex() const
        {
            return _p->deviceIndex;
        }

        int OutputDevice::getDisplayModeIndex() const
        {
            return _p->displayModeIndex;
        }

        device::PixelType OutputDevice::getPixelType() const
        {
            return _p->pixelType;
        }

        void OutputDevice::setDevice(
            int deviceIndex,
            int displayModeIndex,
            device::PixelType pixelType)
        {
            TLRENDER_P();
            bool active = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.deviceIndex = deviceIndex;
                p.displayModeIndex = displayModeIndex;
                p.pixelType = pixelType;
                active = _isDeviceActive();
            }
            p.cv.notify_one();
            if (active != p.deviceActive)
            {
                p.deviceActive = active;
                Q_EMIT deviceActiveChanged(p.deviceActive);
            }
        }

        bool OutputDevice::isDeviceEnabled() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.deviceEnabled;
        }

        bool OutputDevice::isDeviceActive() const
        {
            return _p->deviceActive;
        }

        void OutputDevice::setColorConfigOptions(const timeline::ColorConfigOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.colorConfigOptions = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.lutOptions = value;
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
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                disconnect(
                    i,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
                disconnect(
                    i,
                    SIGNAL(currentVideoChanged(const tl::timeline::VideoData&)),
                    this,
                    SLOT(_currentVideoCallback(const tl::timeline::VideoData&)));
                disconnect(
                    i,
                    SIGNAL(currentAudioChanged(const std::vector<tl::timeline::AudioData>&)),
                    this,
                    SLOT(_currentAudioCallback(const std::vector<tl::timeline::AudioData>&)));
            }
            p.timelinePlayers = value;
            for (const auto& i : p.timelinePlayers)
            {
                connect(
                    i,
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                connect(
                    i,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
                connect(
                    i,
                    SIGNAL(currentVideoChanged(const tl::timeline::VideoData&)),
                    SLOT(_currentVideoCallback(const tl::timeline::VideoData&)));
                connect(
                    i,
                    SIGNAL(currentAudioChanged(const std::vector<tl::timeline::AudioData>&)),
                    SLOT(_currentAudioCallback(const std::vector<tl::timeline::AudioData>&)));
            }
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.timelinePlayers.empty())
                {
                    p.playback = p.timelinePlayers.front()->playback();
                    p.currentTime = p.timelinePlayers.front()->currentTime();
                }
                else
                {
                    p.playback = timeline::Playback::Stop;
                    p.currentTime = time::invalidTime;
                }
                p.sizes.clear();
                p.videoData.clear();
                for (const auto& i : p.timelinePlayers)
                {
                    const auto& ioInfo = i->ioInfo();
                    if (!ioInfo.video.empty())
                    {
                        p.sizes.push_back(ioInfo.video[0].size);
                    }
                    p.videoData.push_back(i->currentVideo());
                }
                p.audioData.clear();
                if (!p.timelinePlayers.empty())
                {
                    p.audioData = p.timelinePlayers.front()->currentAudio();
                }
            }
        }

        void OutputDevice::setOverlay(QImage* qImage)
        {
            TLRENDER_P();
            std::shared_ptr<QImage> tmp;
            if (qImage)
            {
                switch (qImage->format())
                {
                case QImage::Format_RGBA8888:
                case QImage::Format_ARGB4444_Premultiplied:
                    tmp = std::shared_ptr<QImage>(qImage);
                    break;
                default: break;
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.overlay = tmp;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setDeviceEnabled(bool value)
        {
            TLRENDER_P();
            bool active = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.deviceEnabled = value;
                active = _isDeviceActive();
            }
            p.cv.notify_one();
            if (active != p.deviceActive)
            {
                p.deviceActive = active;
                Q_EMIT deviceActiveChanged(p.deviceActive);
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

        void OutputDevice::setVolume(float value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.volume = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setMute(bool value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.mute = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setAudioOffset(double value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.audioOffset = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::_playbackCallback(tl::timeline::Playback value)
        {
            TLRENDER_P();
            if (qobject_cast<qt::TimelinePlayer*>(sender()) == p.timelinePlayers.front())
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.playback = value;
                }
                p.cv.notify_one();
            }
        }

        void OutputDevice::_currentTimeCallback(const otime::RationalTime& value)
        {
            TLRENDER_P();
            if (qobject_cast<qt::TimelinePlayer*>(sender()) == p.timelinePlayers.front())
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.currentTime = value;
                }
                p.cv.notify_one();
            }
        }

        void OutputDevice::_currentVideoCallback(const tl::timeline::VideoData& value)
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

        void OutputDevice::_currentAudioCallback(const std::vector<tl::timeline::AudioData>& value)
        {
            TLRENDER_P();
            if (qobject_cast<qt::TimelinePlayer*>(sender()) == p.timelinePlayers.front())
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.audioData = value;
                }
                p.cv.notify_one();
            }
        }

        void OutputDevice::run()
        {
            TLRENDER_P();

            p.glContext->makeCurrent(p.offscreenSurface.get());
            gl::initGLAD();

            std::shared_ptr<timeline::IRender> render;
            if (auto context = p.context.lock())
            {
                render = timeline::GLRender::create(context);
            }

            int deviceIndex = -1;
            int displayModeIndex = -1;
            device::PixelType pixelType = device::PixelType::None;
            bool deviceEnabled = true;
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            device::HDRMode hdrMode = device::HDRMode::FromFile;
            imaging::HDRData hdrData;
            timeline::CompareOptions compareOptions;
            timeline::Playback playback = timeline::Playback::Stop;
            otime::RationalTime currentTime = time::invalidTime;
            std::vector<imaging::Size> sizes;
            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = true;
            std::vector<timeline::VideoData> videoData;
            std::shared_ptr<QImage> overlay;
            float volume = 1.F;
            bool mute = false;
            double audioOffset = 0.0;
            std::vector<timeline::AudioData> audioData;

            std::shared_ptr<device::IOutputDevice> device;
            std::shared_ptr<tl::gl::Shader> shader;
            std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;
            std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer2;
            std::shared_ptr<gl::VBO> vbo;
            std::shared_ptr<gl::VAO> vao;
            std::array<GLuint, 1> pbo;
            std::array<otime::RationalTime, 1> pboTime;
            size_t pboIndex = 0;
            std::shared_ptr<OverlayTexture> overlayTexture;
            std::shared_ptr<gl::VBO> overlayVbo;
            std::shared_ptr<gl::VAO> overlayVao;

            while (p.running)
            {
                bool createDevice = false;
                bool doRender = false;
                bool overlayChanged = false;
                bool audioChanged = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    if (p.cv.wait_for(
                        lock,
                        p.timeout,
                        [this, deviceIndex, displayModeIndex, pixelType,
                        deviceEnabled, colorConfigOptions, lutOptions, imageOptions,
                        displayOptions, hdrMode, hdrData, compareOptions,
                        playback, currentTime, sizes, viewPos, viewZoom, frameView,
                        videoData, overlay, volume, mute, audioOffset, audioData]
                        {
                            return
                                deviceIndex != _p->deviceIndex ||
                                displayModeIndex != _p->displayModeIndex ||
                                pixelType != _p->pixelType ||
                                deviceEnabled != _p->deviceEnabled ||
                                colorConfigOptions != _p->colorConfigOptions ||
                                lutOptions != _p->lutOptions ||
                                imageOptions != _p->imageOptions ||
                                displayOptions != _p->displayOptions ||
                                hdrMode != _p->hdrMode ||
                                hdrData != _p->hdrData ||
                                compareOptions != _p->compareOptions ||
                                playback != _p->playback ||
                                currentTime != _p->currentTime ||
                                sizes != _p->sizes ||
                                viewPos != _p->viewPos ||
                                viewZoom != _p->viewZoom ||
                                frameView != _p->frameView ||
                                videoData != _p->videoData ||
                                overlay != _p->overlay ||
                                volume != _p->volume ||
                                mute != _p->mute ||
                                audioOffset != _p->audioOffset ||
                                audioData != _p->audioData;
                        }))
                    {
                        createDevice =
                            p.deviceIndex != deviceIndex ||
                            p.displayModeIndex != displayModeIndex ||
                            p.pixelType != pixelType ||
                            p.deviceEnabled != deviceEnabled;
                        deviceIndex = p.deviceIndex;
                        displayModeIndex = p.displayModeIndex;
                        pixelType = p.pixelType;
                        deviceEnabled = p.deviceEnabled;

                        playback = p.playback;
                        currentTime = p.currentTime;

                        doRender =
                            createDevice ||
                            colorConfigOptions != p.colorConfigOptions ||
                            lutOptions != p.lutOptions ||
                            imageOptions != p.imageOptions ||
                            displayOptions != p.displayOptions ||
                            hdrMode != p.hdrMode ||
                            hdrData != p.hdrData ||
                            compareOptions != p.compareOptions ||
                            sizes != p.sizes ||
                            viewPos != p.viewPos ||
                            viewZoom != p.viewZoom ||
                            frameView != p.frameView ||
                            videoData != p.videoData ||
                            overlay != p.overlay;
                        colorConfigOptions = p.colorConfigOptions;
                        lutOptions = p.lutOptions;
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
                        overlayChanged = overlay != p.overlay;
                        overlay = p.overlay;

                        volume = p.volume;
                        mute = p.mute;
                        audioOffset = p.audioOffset;
                        audioChanged = audioData != p.audioData;
                        audioData = p.audioData;
                    }
                }

                if (createDevice)
                {
                    offscreenBuffer2.reset();
                    offscreenBuffer.reset();
                    device.reset();
                    imaging::Size deviceSize;
                    otime::RationalTime deviceFrameRate = time::invalidTime;
                    if (deviceIndex != -1 &&
                        displayModeIndex != -1 &&
                        pixelType != device::PixelType::None &&
                        deviceEnabled)
                    {
                        if (auto deviceSystem = p.deviceSystem.lock())
                        {
                            try
                            {
                                device = deviceSystem->createDevice(deviceIndex, displayModeIndex, pixelType);
                                deviceSize = device->getSize();
                                deviceFrameRate = device->getFrameRate();
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
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        p.deviceEnabled = device.get();
                    }
                    Q_EMIT sizeChanged(deviceSize);
                    Q_EMIT frameRateChanged(deviceFrameRate);

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
                        gl::OffscreenBufferOptions offscreenBufferOptions;
                        offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
                        if (!displayOptions.empty())
                        {
                            offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                        }
                        offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                        offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
                        if (gl::doCreate(offscreenBuffer, renderSize, offscreenBufferOptions))
                        {
                            offscreenBuffer = gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
                        }

                        if (offscreenBuffer)
                        {
                            gl::OffscreenBufferBinding binding(offscreenBuffer);

                            render->begin(
                                renderSize,
                                colorConfigOptions,
                                lutOptions);
                            render->drawVideo(
                                videoData,
                                timeline::getBBoxes(compareOptions.mode, sizes),
                                imageOptions,
                                displayOptions,
                                compareOptions);
                            render->end();
                        }

                        const imaging::Size viewportSize = device->getSize();
                        offscreenBufferOptions = gl::OffscreenBufferOptions();
                        offscreenBufferOptions.colorType = getOffscreenType(pixelType);
                        if (!displayOptions.empty())
                        {
                            offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                        }
                        if (gl::doCreate(offscreenBuffer2, viewportSize, offscreenBufferOptions))
                        {
                            offscreenBuffer2 = gl::OffscreenBuffer::create(viewportSize, offscreenBufferOptions);
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
                                "uniform int       mirrorY;\n"
                                "uniform sampler2D textureSampler;\n"
                                "\n"
                                "void main()\n"
                                "{\n"
                                "    vec2 t = fTexture;\n"
                                "    if (1 == mirrorY)\n"
                                "    {\n"
                                "        t.y = 1.0 - t.y;\n"
                                "    }\n"
                                "    fColor = texture(textureSampler, t);\n"
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
                            math::Matrix4x4f vm;
                            vm = vm * math::translate(math::Vector3f(viewPosTmp.x, viewPosTmp.y, 0.F));
                            vm = vm * math::scale(math::Vector3f(viewZoomTmp, viewZoomTmp, 1.F));
                            auto pm = math::ortho(
                                0.F,
                                static_cast<float>(viewportSize.w),
                                0.F,
                                static_cast<float>(viewportSize.h),
                                -1.F,
                                1.F);
                            shader->setUniform("transform.mvp", pm * vm);
                            shader->setUniform("mirrorY", false);

                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, offscreenBuffer->getColorID());

                            auto mesh = geom::bbox(math::BBox2i(0, 0, renderSize.w, renderSize.h));
                            if (!vbo)
                            {
                                vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos2_F32_UV_U16);
                            }
                            if (vbo)
                            {
                                vbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
                            }

                            if (!vao && vbo)
                            {
                                vao = gl::VAO::create(gl::VBOType::Pos2_F32_UV_U16, vbo->getID());
                            }
                            if (vao && vbo)
                            {
                                vao->bind();
                                vao->draw(GL_TRIANGLES, 0, vbo->getSize());
                            }

                            if ((overlay && !overlayTexture) ||
                                (overlay && overlayTexture &&
                                    (overlay->size() != overlayTexture->getSize() ||
                                        overlay->format() != overlayTexture->getFormat())))
                            {
                                overlayTexture = OverlayTexture::create(overlay->size(), overlay->format());
                            }
                            if (overlay && overlayTexture && overlayChanged)
                            {
                                overlayTexture->copy(*overlay);
                            }
                            if (!overlay && overlayTexture)
                            {
                                overlayTexture.reset();
                            }
                            if (overlay && overlayTexture)
                            {
                                switch (overlay->format())
                                {
                                case QImage::Format_RGBA8888:
                                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
                                    break;
                                case QImage::Format_ARGB4444_Premultiplied:
                                    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
                                    break;
                                default: break;
                                }

                                shader->setUniform("transform.mvp", pm);
                                shader->setUniform("mirrorY", true);

                                glBindTexture(GL_TEXTURE_2D, overlayTexture->getID());

                                mesh = geom::bbox(math::BBox2i(0, 0, viewportSize.w, viewportSize.h));
                                if (!overlayVbo)
                                {
                                    overlayVbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos2_F32_UV_U16);
                                }
                                if (overlayVbo)
                                {
                                    overlayVbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
                                }

                                if (!overlayVao && overlayVbo)
                                {
                                    overlayVao = gl::VAO::create(gl::VBOType::Pos2_F32_UV_U16, overlayVbo->getID());
                                }
                                if (overlayVao && overlayVbo)
                                {
                                    overlayVao->bind();
                                    overlayVao->draw(GL_TRIANGLES, 0, overlayVbo->getSize());
                                }
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
                                device->setPixelData(pixelData);
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

                if (device)
                {
                    device->setPlayback(playback, currentTime);
                    device->setVolume(volume);
                    device->setMute(mute);
                    device->setAudioOffset(audioOffset);
                }
                if (device && audioChanged)
                {
                    device->setAudioData(audioData);
                }
            }
            glDeleteBuffers(pbo.size(), pbo.data());
        }

        bool OutputDevice::_isDeviceActive() const
        {
            TLRENDER_P();
            return
                p.deviceIndex != -1 &&
                p.displayModeIndex != -1 &&
                p.pixelType != device::PixelType::None &&
                p.deviceEnabled;
        }
    }
}
