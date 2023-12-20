// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputDevicePrivate.h>

#include <tlDevice/BMDUtil.h>

#include <tlTimeline/GLRender.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>
#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>
#include <tlGL/Texture.h>

#include <tlCore/AudioResample.h>
#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <list>
#include <mutex>
#include <tuple>

#if defined(_WINDOWS)
#include <atlbase.h>
#endif // _WINDOWS

namespace tl
{
    namespace device
    {
        namespace
        {
            const std::chrono::milliseconds timeout = std::chrono::milliseconds(5);

            struct DL
            {
                ~DL()
                {
                    if (dlOutput.p)
                    {
                        dlOutput.p->StopScheduledPlayback(0, nullptr, 0);
                        dlOutput.p->DisableVideoOutput();
                        dlOutput.p->DisableAudioOutput();
                    }
                }

                DLWrapper dl;
                DLConfigWrapper dlConfig;
                DLOutputWrapper dlOutput;
                audio::Info audioInfo;
                DLOutputCallbackWrapper dlOutputCallback;
            };
        }

        struct BMDOutputDevice::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<observer::Value<DeviceConfig> > config;
            std::shared_ptr<observer::Value<bool> > enabled;
            std::shared_ptr<observer::Value<bool> > active;
            std::shared_ptr<observer::Value<math::Size2i> > size;
            std::shared_ptr<observer::Value<otime::RationalTime> > frameRate;

            std::vector<std::shared_ptr<timeline::Player> > players;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            std::vector<std::shared_ptr<observer::ValueObserver<timeline::VideoData> > > videoObservers;
            std::shared_ptr<observer::ListObserver<timeline::AudioData> > audioObserver;

            std::shared_ptr<gl::GLFWWindow> window;

            struct Mutex
            {
                DeviceConfig config;
                bool enabled = false;
                bool active = false;
                math::Size2i size;
                otime::RationalTime frameRate = time::invalidTime;
                timeline::OCIOOptions ocioOptions;
                timeline::LUTOptions lutOptions;
                std::vector<timeline::ImageOptions> imageOptions;
                std::vector<timeline::DisplayOptions> displayOptions;
                device::HDRMode hdrMode = device::HDRMode::FromFile;
                image::HDRData hdrData;
                timeline::CompareOptions compareOptions;
                math::Vector2i viewPos;
                float viewZoom = 1.F;
                bool frameView = true;
                timeline::Playback playback = timeline::Playback::Stop;
                otime::RationalTime currentTime = time::invalidTime;
                std::vector<image::Size> sizes;
                std::vector<timeline::VideoData> videoData;
                std::vector<timeline::AudioData> audioData;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                std::unique_ptr<DL> dl;
                math::Size2i size;
                PixelType pixelType = PixelType::None;
                device::HDRMode hdrMode = device::HDRMode::FromFile;
                image::HDRData hdrData;
                math::Vector2i viewPos;
                float viewZoom = 1.F;
                bool frameView = true;
                std::vector<image::Size> sizes;
                std::vector<timeline::VideoData> videoData;
                std::shared_ptr<timeline::IRender> render;
                std::shared_ptr<tl::gl::Shader> shader;
                std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;
                std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer2;
                std::shared_ptr<gl::VBO> vbo;
                std::shared_ptr<gl::VAO> vao;
                std::array<GLuint, 1> pbo;
                std::array<otime::RationalTime, 1> pboTime;
                size_t pboIndex = 0;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void BMDOutputDevice::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;
            p.config = observer::Value<DeviceConfig>::create();
            p.enabled = observer::Value<bool>::create(false);
            p.active = observer::Value<bool>::create(false);
            p.size = observer::Value<math::Size2i>::create();
            p.frameRate = observer::Value<otime::RationalTime>::create(time::invalidTime);

            p.window = gl::GLFWWindow::create(
                "tl::device::BMDOutputDevice",
                math::Size2i(1, 1),
                context,
                static_cast<int>(gl::GLFWWindowOptions::None));
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    try
                    {
                        p.window->makeCurrent();
                    }
                    catch (const std::exception& e)
                    {
                        if (auto context = p.context.lock())
                        {
                            context->log(
                                "tl::device::BMDOutputDevice",
                                string::Format("Cannot make the OpenGL context current: {0}").
                                arg(e.what()),
                                log::Type::Error);
                        }
                    }
                    _run();
                    p.window->doneCurrent();
                });
        }

        BMDOutputDevice::BMDOutputDevice() :
            _p(new Private)
        {}

        BMDOutputDevice::~BMDOutputDevice()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<BMDOutputDevice> BMDOutputDevice::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<BMDOutputDevice>(new BMDOutputDevice);
            out->_init(context);
            return out;
        }

        DeviceConfig BMDOutputDevice::getConfig() const
        {
            return _p->config->get();
        }

        std::shared_ptr<observer::IValue<DeviceConfig> > BMDOutputDevice::observeConfig() const
        {
            return _p->config;
        }

        void BMDOutputDevice::setConfig(const DeviceConfig& value)
        {
            TLRENDER_P();
            if (p.config->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.config = value;
                }
                p.thread.cv.notify_one();
            }
        }

        bool BMDOutputDevice::isEnabled() const
        {
            return _p->enabled->get();
        }

        std::shared_ptr<observer::IValue<bool> > BMDOutputDevice::observeEnabled() const
        {
            return _p->enabled;
        }

        void BMDOutputDevice::setEnabled(bool value)
        {
            TLRENDER_P();
            if (p.enabled->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.enabled = value;
                }
                p.thread.cv.notify_one();
            }
        }

        bool BMDOutputDevice::isActive() const
        {
            return _p->active->get();
        }

        std::shared_ptr<observer::IValue<bool> > BMDOutputDevice::observeActive() const
        {
            return _p->active;
        }

        const math::Size2i& BMDOutputDevice::getSize() const
        {
            return _p->size->get();
        }

        std::shared_ptr<observer::IValue<math::Size2i> > BMDOutputDevice::observeSize() const
        {
            return _p->size;
        }

        const otime::RationalTime& BMDOutputDevice::getFrameRate() const
        {
            return _p->frameRate->get();
        }

        std::shared_ptr<observer::IValue<otime::RationalTime> > BMDOutputDevice::observeFrameRate() const
        {
            return _p->frameRate;
        }

        void BMDOutputDevice::setView(
            const tl::math::Vector2i& position,
            float                     zoom,
            bool                      frame)
        {}

        void BMDOutputDevice::setOCIOOptions(const timeline::OCIOOptions&)
        {}

        void BMDOutputDevice::setLUTOptions(const timeline::LUTOptions&)
        {}

        void BMDOutputDevice::setImageOptions(const std::vector<timeline::ImageOptions>&)
        {}

        void BMDOutputDevice::setDisplayOptions(const std::vector<timeline::DisplayOptions>&)
        {}

        void BMDOutputDevice::setHDR(device::HDRMode, const image::HDRData&)
        {}

        void BMDOutputDevice::setCompareOptions(const timeline::CompareOptions&)
        {}

        void BMDOutputDevice::setPlayers(const std::vector<std::shared_ptr<timeline::Player> >& value)
        {
            TLRENDER_P();
            if (value == p.players)
                return;

            p.playbackObserver.reset();
            p.currentTimeObserver.reset();
            p.videoObservers.clear();
            p.audioObserver.reset();

            p.players = value;

            if (!p.players.empty() && p.players.front())
            {
                auto weak = std::weak_ptr<BMDOutputDevice>(shared_from_this());
                p.playbackObserver = observer::ValueObserver<timeline::Playback>::create(
                    p.players.front()->observePlayback(),
                    [weak](timeline::Playback value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.playback = value;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    });
                p.currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                    p.players.front()->observeCurrentTime(),
                    [weak](const otime::RationalTime& value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.currentTime = value;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    });
                for (size_t i = 0; i < p.players.size(); ++i)
                {
                    if (p.players[i])
                    {
                        p.videoObservers.push_back(observer::ValueObserver<timeline::VideoData>::create(
                            p.players[i]->observeCurrentVideo(),
                            [weak, i](const timeline::VideoData& value)
                            {
                                if (auto device = weak.lock())
                                {
                                    {
                                        std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                        if (i < device->_p->mutex.videoData.size())
                                        {
                                            device->_p->mutex.videoData[i] = value;
                                        }
                                    }
                                    device->_p->thread.cv.notify_one();
                                }
                            }));
                    }
                }
                p.audioObserver = observer::ListObserver<timeline::AudioData>::create(
                    p.players.front()->observeCurrentAudio(),
                    [weak](const std::vector<timeline::AudioData>& value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.audioData = value;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    });
            }

            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.players.empty() && p.players.front())
                {
                    p.mutex.playback = p.players.front()->getPlayback();
                    p.mutex.currentTime = p.players.front()->getCurrentTime();
                }
                else
                {
                    p.mutex.playback = timeline::Playback::Stop;
                    p.mutex.currentTime = time::invalidTime;
                }
                p.mutex.sizes.clear();
                p.mutex.videoData.clear();
                for (const auto& player : p.players)
                {
                    if (player)
                    {
                        const auto& ioInfo = player->getIOInfo();
                        if (!ioInfo.video.empty())
                        {
                            p.mutex.sizes.push_back(ioInfo.video[0].size);
                        }
                        p.mutex.videoData.push_back(player->getCurrentVideo());
                    }
                }
                p.mutex.audioData.clear();
                if (!p.players.empty() && p.players.front())
                {
                    p.mutex.audioData = p.players.front()->getCurrentAudio();
                }
            }
        }

        void BMDOutputDevice::tick()
        {
            TLRENDER_P();
            bool active = false;
            math::Size2i size = p.size->get();
            otime::RationalTime frameRate = p.frameRate->get();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                active = p.mutex.active;
                size = p.mutex.size;
                frameRate = p.mutex.frameRate;
            }
            p.active->setIfChanged(active);
            p.size->setIfChanged(size);
            p.frameRate->setIfChanged(frameRate);
        }

        void BMDOutputDevice::_run()
        {
            TLRENDER_P();

            DeviceConfig config;
            bool enabled = false;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            timeline::Playback playback = timeline::Playback::Stop;
            otime::RationalTime currentTime = time::invalidTime;
            std::vector<timeline::AudioData> audioData;

            if (auto context = p.context.lock())
            {
                p.thread.render = timeline::GLRender::create(context);
            }

            while (p.thread.running)
            {
                bool createDevice = false;
                bool doRender = false;
                bool overlayChanged = false;
                bool audioChanged = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        timeout,
                        [this, config, enabled,
                        ocioOptions, lutOptions, imageOptions,
                        displayOptions, compareOptions,
                        playback, currentTime,
                        audioData]
                        {
                            return
                                config != _p->mutex.config ||
                                enabled != _p->mutex.enabled ||
                                ocioOptions != _p->mutex.ocioOptions ||
                                lutOptions != _p->mutex.lutOptions ||
                                imageOptions != _p->mutex.imageOptions ||
                                displayOptions != _p->mutex.displayOptions ||
                                _p->thread.hdrMode != _p->mutex.hdrMode ||
                                _p->thread.hdrData != _p->mutex.hdrData ||
                                compareOptions != _p->mutex.compareOptions ||
                                _p->thread.viewPos != _p->mutex.viewPos ||
                                _p->thread.viewZoom != _p->mutex.viewZoom ||
                                _p->thread.frameView != _p->mutex.frameView ||
                                playback != _p->mutex.playback ||
                                currentTime != _p->mutex.currentTime ||
                                _p->thread.sizes != _p->mutex.sizes ||
                                _p->thread.videoData != _p->mutex.videoData ||
                                audioData != _p->mutex.audioData;
                        }))
                    {
                        createDevice =
                            p.mutex.config != config ||
                            p.mutex.enabled != enabled;
                        config = p.mutex.config;
                        enabled = p.mutex.enabled;

                        playback = p.mutex.playback;
                        currentTime = p.mutex.currentTime;

                        doRender =
                            createDevice ||
                            ocioOptions != p.mutex.ocioOptions ||
                            lutOptions != p.mutex.lutOptions ||
                            imageOptions != p.mutex.imageOptions ||
                            displayOptions != p.mutex.displayOptions ||
                            p.thread.hdrMode != p.mutex.hdrMode ||
                            p.thread.hdrData != p.mutex.hdrData ||
                            compareOptions != p.mutex.compareOptions ||
                            p.thread.viewPos != p.mutex.viewPos ||
                            p.thread.viewZoom != p.mutex.viewZoom ||
                            p.thread.frameView != p.mutex.frameView ||
                            p.thread.sizes != p.mutex.sizes ||
                            p.thread.videoData != p.mutex.videoData;
                        ocioOptions = p.mutex.ocioOptions;
                        lutOptions = p.mutex.lutOptions;
                        imageOptions = p.mutex.imageOptions;
                        displayOptions = p.mutex.displayOptions;
                        p.thread.hdrMode = p.mutex.hdrMode;
                        p.thread.hdrData = p.mutex.hdrData;
                        compareOptions = p.mutex.compareOptions;
                        p.thread.viewPos = p.mutex.viewPos;
                        p.thread.viewZoom = p.mutex.viewZoom;
                        p.thread.frameView = p.mutex.frameView;
                        p.thread.sizes = p.mutex.sizes;
                        p.thread.videoData = p.mutex.videoData;

                        audioChanged = audioData != p.mutex.audioData;
                        audioData = p.mutex.audioData;
                    }
                }

                if (createDevice)
                {
                    glDeleteBuffers(p.thread.pbo.size(), p.thread.pbo.data());
                    p.thread.vao.reset();
                    p.thread.vbo.reset();
                    p.thread.offscreenBuffer2.reset();
                    p.thread.offscreenBuffer.reset();
                    p.thread.dl.reset();
                    if (enabled)
                    {
                        p.thread.dl.reset(new DL);
                        _createDevice();
                        if (p.thread.dl->dlOutputCallback.p)
                        {
                            glGenBuffers(p.thread.pbo.size(), p.thread.pbo.data());
                            for (size_t i = 0; i < p.thread.pbo.size(); ++i)
                            {
                                glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo[i]);
                                glBufferData(
                                    GL_PIXEL_PACK_BUFFER,
                                    device::getDataByteCount(p.thread.size, p.thread.pixelType),
                                    NULL,
                                    GL_STREAM_COPY);
                            }
                        }
                    }
                }

                if (p.thread.dl && p.thread.dl->dlOutputCallback.p && doRender && p.thread.render)
                {
                    _render(
                        ocioOptions,
                        lutOptions,
                        imageOptions,
                        displayOptions,
                        compareOptions);
                }

                if (p.thread.dl && p.thread.dl->dlOutputCallback.p)
                {
                    p.thread.dl->dlOutputCallback.p->setPlayback(playback, currentTime);
                }
                if (p.thread.dl && p.thread.dl->dlOutputCallback.p && audioChanged)
                {
                    p.thread.dl->dlOutputCallback.p->setAudioData(audioData);
                }
            }

            p.thread.vao.reset();
            p.thread.vbo.reset();
            p.thread.offscreenBuffer2.reset();
            p.thread.offscreenBuffer.reset();
            p.thread.shader.reset();
            p.thread.render.reset();
            p.thread.dl.reset();
        }

        void BMDOutputDevice::_createDevice()
        {
            TLRENDER_P();
            bool active = false;
            otime::RationalTime frameRate = time::invalidTime;
            try
            {
                const DeviceConfig& config = p.config->get();
                std::string modelName;
                {
                    DLIteratorWrapper dlIterator;
                    if (GetDeckLinkIterator(&dlIterator.p) != S_OK)
                    {
                        throw std::runtime_error("Cannot get iterator");
                    }

                    int count = 0;
                    while (dlIterator.p->Next(&p.thread.dl->dl.p) == S_OK)
                    {
                        if (count == config.deviceIndex)
                        {
#if defined(__APPLE__)
                            CFStringRef dlModelName;
                            p.thread.dl->dl.p->GetModelName(&dlModelName);
                            StringToStdString(dlModelName, modelName);
                            CFRelease(dlModelName);
#else // __APPLE__
                            dlstring_t dlModelName;
                            p.thread.dl->dl.p->GetModelName(&dlModelName);
                            modelName = DlToStdString(dlModelName);
                            DeleteString(dlModelName);
#endif // __APPLE__
                            break;
                        }

                        p.thread.dl->dl.p->Release();
                        p.thread.dl->dl.p = nullptr;

                        ++count;
                    }
                    if (!p.thread.dl->dl.p)
                    {
                        throw std::runtime_error("Device not found");
                    }
                }

                if (p.thread.dl->dl.p->QueryInterface(IID_IDeckLinkConfiguration, (void**)&p.thread.dl->dlConfig) != S_OK)
                {
                    throw std::runtime_error("Configuration device not found");
                }
                for (const auto& option : config.boolOptions)
                {
                    switch (option.first)
                    {
                    case Option::_444SDIVideoOutput:
                        p.thread.dl->dlConfig.p->SetFlag(bmdDeckLinkConfig444SDIVideoOutput, option.second);
                        break;
                    default: break;
                    }
                }
                /*if (0)
                {
                    BOOL value = 0;
                    p.dl->dlConfig.p->GetFlag(bmdDeckLinkConfig444SDIVideoOutput, &value);
                }*/

                if (p.thread.dl->dl.p->QueryInterface(IID_IDeckLinkOutput, (void**)&p.thread.dl->dlOutput) != S_OK)
                {
                    throw std::runtime_error("Output device not found");
                }

                {
                    DLDisplayModeIteratorWrapper dlDisplayModeIterator;
                    if (p.thread.dl->dlOutput.p->GetDisplayModeIterator(&dlDisplayModeIterator.p) != S_OK)
                    {
                        throw std::runtime_error("Cannot get display mode iterator");
                    }
                    DLDisplayModeWrapper dlDisplayMode;
                    int count = 0;
                    while (dlDisplayModeIterator.p->Next(&dlDisplayMode.p) == S_OK)
                    {
                        if (count == config.displayModeIndex)
                        {
                            break;
                        }

                        dlDisplayMode.p->Release();
                        dlDisplayMode.p = nullptr;

                        ++count;
                    }
                    if (!dlDisplayMode.p)
                    {
                        throw std::runtime_error("Display mode not found");
                    }

                    p.thread.size.w = dlDisplayMode.p->GetWidth();
                    p.thread.size.h = dlDisplayMode.p->GetHeight();
                    switch (config.pixelType)
                    {
                    case PixelType::_8BitBGRA:
                    case PixelType::_10BitRGB:
                    case PixelType::_10BitRGBX:
                    case PixelType::_10BitRGBXLE:
                        p.thread.pixelType = config.pixelType;
                        break;
                    case PixelType::_8BitYUV:
                        p.thread.pixelType = PixelType::_8BitBGRA;
                        break;
                    case PixelType::_10BitYUV:
                    case PixelType::_12BitRGB:
                    case PixelType::_12BitRGBLE:
                        p.thread.pixelType = PixelType::_10BitRGB;
                        break;
                    default:
                        p.thread.pixelType = PixelType::None;
                        break;
                    }
                    BMDTimeValue frameDuration;
                    BMDTimeScale frameTimescale;
                    dlDisplayMode.p->GetFrameRate(&frameDuration, &frameTimescale);
                    frameRate = otime::RationalTime(frameDuration, frameTimescale);
                    p.thread.dl->audioInfo.channelCount = 2;
                    p.thread.dl->audioInfo.dataType = audio::DataType::S16;
                    p.thread.dl->audioInfo.sampleRate = 48000;

                    if (auto context = p.context.lock())
                    {
                        context->log(
                            "tl::device::BMDOutputDevice",
                            string::Format(
                                "\n"
                                "    #{0} {1}\n"
                                "    video: {2} {3}\n"
                                "    audio: {4} {5} {6}").
                            arg(config.deviceIndex).
                            arg(modelName).
                            arg(p.thread.size).
                            arg(frameRate).
                            arg(p.thread.dl->audioInfo.channelCount).
                            arg(p.thread.dl->audioInfo.dataType).
                            arg(p.thread.dl->audioInfo.sampleRate));
                    }

                    HRESULT r = p.thread.dl->dlOutput.p->EnableVideoOutput(
                        dlDisplayMode.p->GetDisplayMode(),
                        bmdVideoOutputFlagDefault);
                    switch (r)
                    {
                    case S_OK:
                        break;
                    case E_ACCESSDENIED:
                        throw std::runtime_error("Unable to access the hardware");
                    default:
                        throw std::runtime_error("Cannot enable video output");
                    }

                    r = p.thread.dl->dlOutput.p->EnableAudioOutput(
                        bmdAudioSampleRate48kHz,
                        bmdAudioSampleType16bitInteger,
                        p.thread.dl->audioInfo.channelCount,
                        bmdAudioOutputStreamContinuous);
                    switch (r)
                    {
                    case S_OK:
                        break;
                    case E_INVALIDARG:
                        throw std::runtime_error("Invalid number of channels requested");
                    case E_ACCESSDENIED:
                        throw std::runtime_error("Unable to access the hardware");
                    default:
                        throw std::runtime_error("Cannot enable audio output");
                    }
                }

                p.thread.dl->dlOutputCallback.p = new DLOutputCallback(
                    p.thread.dl->dlOutput.p,
                    p.thread.size,
                    config.pixelType,
                    frameRate,
                    p.thread.dl->audioInfo);

                if (p.thread.dl->dlOutput.p->SetScheduledFrameCompletionCallback(p.thread.dl->dlOutputCallback.p) != S_OK)
                {
                    throw std::runtime_error("Cannot set video callback");
                }

                if (p.thread.dl->dlOutput.p->SetAudioCallback(p.thread.dl->dlOutputCallback.p) != S_OK)
                {
                    throw std::runtime_error("Cannot set audio callback");
                }

                active = true;
            }
            catch (const std::exception& e)
            {
                if (auto context = p.context.lock())
                {
                    context->log(
                        "tl::device::BMDOutputDevice",
                        e.what(),
                        log::Type::Error);
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.active = active;
                p.mutex.size = p.thread.size;
                p.mutex.frameRate = frameRate;
            }
        }

        namespace
        {
            image::PixelType getOffscreenType(device::PixelType value)
            {
                const std::array<image::PixelType, static_cast<size_t>(device::PixelType::Count)> data =
                {
                    image::PixelType::None,
                    image::PixelType::RGBA_U8,
                    image::PixelType::RGBA_U8,
                    image::PixelType::RGB_U10,
                    image::PixelType::RGB_U10,
                    image::PixelType::RGB_U10,
                    image::PixelType::RGB_U10,
                    image::PixelType::RGB_U10,
                    image::PixelType::RGB_U10
                };
                return data[static_cast<size_t>(value)];
            }

            GLenum getReadPixelsFormat(device::PixelType value)
            {
                const std::array<GLenum, static_cast<size_t>(device::PixelType::Count)> data =
                {
                    GL_NONE,
                    GL_BGRA,
                    GL_BGRA,
                    GL_BGRA,
                    GL_RGBA,
                    GL_RGBA,
                    GL_RGBA,
                    GL_RGBA,
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
                    GL_UNSIGNED_BYTE,
                    GL_UNSIGNED_INT_2_10_10_10_REV,
                    GL_UNSIGNED_INT_10_10_10_2,
                    GL_UNSIGNED_INT_10_10_10_2,
                    GL_UNSIGNED_INT_10_10_10_2,
                    GL_UNSIGNED_INT_10_10_10_2,
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
                    4,
                    256,
                    256,
                    256,
                    128,
                    256,
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
                    GL_FALSE,
                    GL_TRUE,
                    GL_TRUE,
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE
                };
                return data[static_cast<size_t>(value)];
            }
        }

        void BMDOutputDevice::_render(
            const timeline::OCIOOptions& ocioOptions,
            const timeline::LUTOptions& lutOptions,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();
            try
            {
                const math::Size2i renderSize = timeline::getRenderSize(compareOptions.mode, p.thread.sizes);
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = gl::offscreenColorDefault;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                }
                offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
                if (gl::doCreate(p.thread.offscreenBuffer, renderSize, offscreenBufferOptions))
                {
                    p.thread.offscreenBuffer = gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
                }

                if (p.thread.offscreenBuffer)
                {
                    gl::OffscreenBufferBinding binding(p.thread.offscreenBuffer);

                    p.thread.render->begin(renderSize);
                    p.thread.render->setOCIOOptions(ocioOptions);
                    p.thread.render->setLUTOptions(lutOptions);
                    p.thread.render->drawVideo(
                        p.thread.videoData,
                        timeline::getBoxes(compareOptions.mode, p.thread.sizes),
                        imageOptions,
                        displayOptions,
                        compareOptions);
                    p.thread.render->end();
                }

                const math::Size2i viewportSize = p.thread.size;
                offscreenBufferOptions = gl::OffscreenBufferOptions();
                offscreenBufferOptions.colorType = getOffscreenType(p.thread.pixelType);
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                }
                if (gl::doCreate(p.thread.offscreenBuffer2, viewportSize, offscreenBufferOptions))
                {
                    p.thread.offscreenBuffer2 = gl::OffscreenBuffer::create(viewportSize, offscreenBufferOptions);
                }

                math::Vector2i viewPosTmp = p.thread.viewPos;
                float viewZoomTmp = p.thread.viewZoom;
                if (p.thread.frameView)
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

                if (!p.thread.shader)
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
                    p.thread.shader = gl::Shader::create(vertexSource, fragmentSource);
                }

                if (p.thread.offscreenBuffer && p.thread.offscreenBuffer2)
                {
                    gl::OffscreenBufferBinding binding(p.thread.offscreenBuffer2);

                    glViewport(
                        0,
                        0,
                        GLsizei(viewportSize.w),
                        GLsizei(viewportSize.h));
                    glClearColor(0.F, 0.F, 0.F, 0.F);
                    glClear(GL_COLOR_BUFFER_BIT);

                    p.thread.shader->bind();
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
                    p.thread.shader->setUniform("transform.mvp", pm * vm);
                    p.thread.shader->setUniform("mirrorY", false);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, p.thread.offscreenBuffer->getColorID());

                    auto mesh = geom::box(math::Box2i(0, 0, renderSize.w, renderSize.h));
                    if (!p.thread.vbo)
                    {
                        p.thread.vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos2_F32_UV_U16);
                    }
                    if (p.thread.vbo)
                    {
                        p.thread.vbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
                    }

                    if (!p.thread.vao && p.thread.vbo)
                    {
                        p.thread.vao = gl::VAO::create(gl::VBOType::Pos2_F32_UV_U16, p.thread.vbo->getID());
                    }
                    if (p.thread.vao && p.thread.vbo)
                    {
                        p.thread.vao->bind();
                        p.thread.vao->draw(GL_TRIANGLES, 0, p.thread.vbo->getSize());
                    }

                    glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo[p.thread.pboIndex % p.thread.pbo.size()]);
                    p.thread.pboTime[p.thread.pboIndex % p.thread.pbo.size()] = !p.thread.videoData.empty() ? p.thread.videoData[0].time : time::invalidTime;
                    if (0 == viewportSize.w % getReadPixelsAlign(p.thread.pixelType) &&
                        !getReadPixelsSwap(p.thread.pixelType))
                    {
                        //std::cout << "BMDOutputDevice glGetTexImage" << std::endl;
                        glBindTexture(GL_TEXTURE_2D, p.thread.offscreenBuffer2->getColorID());
                        glGetTexImage(
                            GL_TEXTURE_2D,
                            0,
                            getReadPixelsFormat(p.thread.pixelType),
                            getReadPixelsType(p.thread.pixelType),
                            NULL);
                    }
                    else
                    {
                        //std::cout << "BMDOutputDevice glReadPixels" << std::endl;
                        glPixelStorei(GL_PACK_ALIGNMENT, getReadPixelsAlign(p.thread.pixelType));
                        glPixelStorei(GL_PACK_SWAP_BYTES, getReadPixelsSwap(p.thread.pixelType));
                        glReadPixels(
                            0,
                            0,
                            viewportSize.w,
                            viewportSize.h,
                            getReadPixelsFormat(p.thread.pixelType),
                            getReadPixelsType(p.thread.pixelType),
                            NULL);
                    }

                    ++(p.thread.pboIndex);
                    if (p.thread.pbo[p.thread.pboIndex % p.thread.pbo.size()])
                    {
                        auto dlVideoFrame = std::make_shared<DLVideoFrameWrapper>();
                        if (p.thread.dl->dlOutput.p->CreateVideoFrame(
                            viewportSize.w,
                            viewportSize.h,
                            getRowByteCount(viewportSize.w, p.thread.pixelType),
                            toBMD(p.thread.pixelType),
                            bmdFrameFlagFlipVertical,
                            &dlVideoFrame->p) != S_OK)
                        {
                            throw std::runtime_error("Cannot create video frame");
                        }

                        /*std::shared_ptr<image::HDRData> hdrDataP;
                        switch (p.thread.hdrMode)
                        {
                        case device::HDRMode::FromFile:
                            if (!p.thread.videoData.empty())
                            {
                                hdrDataP = device::getHDRData(p.thread.videoData[0]);
                            }
                            break;
                        case device::HDRMode::Custom:
                            hdrDataP.reset(new image::HDRData(p.thread.hdrData));
                            break;
                        default: break;
                        }
                        pixelData->setHDRData(hdrDataP);*/

                        glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo[p.thread.pboIndex % p.thread.pbo.size()]);
                        if (void* buffer = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY))
                        {
                            void* dlFrame = nullptr;
                            dlVideoFrame->p->GetBytes((void**)&dlFrame);
                            memcpy(
                                dlFrame,
                                buffer,
                                getDataByteCount(viewportSize, p.thread.pixelType));
                            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                        }
                        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                        p.thread.dl->dlOutputCallback.p->setVideo(
                            dlVideoFrame,
                            p.thread.pboTime[p.thread.pboIndex % p.thread.pbo.size()]);
                    }
                }
            }
            catch (const std::exception& e)
            {
                if (auto context = p.context.lock())
                {
                    context->log("tl::device::BMDOutputDevice", e.what(), log::Type::Error);
                }
            }
        }
    }
}
