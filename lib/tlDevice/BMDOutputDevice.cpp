// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputPrivate.h>

#include <tlDevice/BMDUtil.h>

#include <tlTimelineGL/Render.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Texture.h>
#include <tlGL/Util.h>

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
            const std::chrono::milliseconds timeout(5);
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
                HDRMode hdrMode = HDRMode::FromFile;
                image::HDRData hdrData;
                timeline::CompareOptions compareOptions;
                math::Vector2i viewPos;
                double viewZoom = 1.0;
                bool frameView = true;
                otime::TimeRange timeRange = time::invalidTimeRange;
                timeline::Playback playback = timeline::Playback::Stop;
                otime::RationalTime currentTime = time::invalidTime;
                std::vector<image::Size> sizes;
                std::vector<timeline::VideoData> videoData;
                float volume = 1.F;
                bool mute = false;
                double audioOffset = 0.0;
                std::vector<timeline::AudioData> audioData;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                std::unique_ptr<DLWrapper> dl;

                math::Size2i size;
                PixelType outputPixelType = PixelType::None;
                HDRMode hdrMode = HDRMode::FromFile;
                image::HDRData hdrData;
                math::Vector2i viewPos;
                double viewZoom = 1.0;
                bool frameView = true;
                otime::TimeRange timeRange = time::invalidTimeRange;
                std::vector<image::Size> sizes;
                std::vector<timeline::VideoData> videoData;

                std::shared_ptr<timeline::IRender> render;
                std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;
                GLuint pbo = 0;
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
            double                    zoom,
            bool                      frame)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.viewPos = position;
                p.mutex.viewZoom = zoom;
                p.mutex.frameView = frame;
            }
            p.thread.cv.notify_one();
        }

        void BMDOutputDevice::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.ocioOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void BMDOutputDevice::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.lutOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void BMDOutputDevice::setImageOptions(const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.imageOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void BMDOutputDevice::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.displayOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void BMDOutputDevice::setHDR(HDRMode hdrMode, const image::HDRData& hdrData)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.hdrMode = hdrMode;
                p.mutex.hdrData = hdrData;
            }
            p.thread.cv.notify_one();
        }

        void BMDOutputDevice::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.compareOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void BMDOutputDevice::setVolume(float value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.volume = value;
            }
            p.thread.cv.notify_one();
        }

        void BMDOutputDevice::setMute(bool value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.mute = value;
            }
            p.thread.cv.notify_one();
        }

        void BMDOutputDevice::setAudioOffset(double value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.audioOffset = value;
            }
            p.thread.cv.notify_one();
        }

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
                    p.mutex.timeRange = p.players.front()->getTimeRange();
                    p.mutex.playback = p.players.front()->getPlayback();
                    p.mutex.currentTime = p.players.front()->getCurrentTime();
                }
                else
                {
                    p.mutex.timeRange = time::invalidTimeRange;
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
            float volume = 1.F;
            bool mute = false;
            double audioOffset = 0.0;
            std::vector<timeline::AudioData> audioData;

            if (auto context = p.context.lock())
            {
                p.thread.render = timeline_gl::Render::create(context);
            }

            auto t = std::chrono::steady_clock::now();
            while (p.thread.running)
            {
                bool createDevice = false;
                bool doRender = false;
                bool overlayChanged = false;
                bool audioDataChanged = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        timeout,
                        [this, config, enabled,
                        ocioOptions, lutOptions, imageOptions,
                        displayOptions, compareOptions,
                        playback, currentTime,
                        volume, mute, audioOffset, audioData]
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
                                _p->thread.timeRange != _p->mutex.timeRange ||
                                playback != _p->mutex.playback ||
                                currentTime != _p->mutex.currentTime ||
                                _p->thread.sizes != _p->mutex.sizes ||
                                _p->thread.videoData != _p->mutex.videoData ||
                                volume != _p->mutex.volume ||
                                mute != _p->mutex.mute ||
                                audioOffset != _p->mutex.audioOffset ||
                                audioData != _p->mutex.audioData;
                        }))
                    {
                        createDevice =
                            p.mutex.config != config ||
                            p.mutex.enabled != enabled;
                        config = p.mutex.config;
                        enabled = p.mutex.enabled;

                        p.thread.timeRange = p.mutex.timeRange;
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

                        audioDataChanged =
                            createDevice ||
                            audioData != p.mutex.audioData;
                        volume = p.mutex.volume;
                        mute = p.mutex.mute;
                        audioOffset = p.mutex.audioOffset;
                        audioData = p.mutex.audioData;
                    }
                }

                if (createDevice)
                {
                    if (p.thread.pbo != 0)
                    {
                        glDeleteBuffers(1, &p.thread.pbo);
                        p.thread.pbo = 0;
                    }
                    p.thread.offscreenBuffer.reset();
                    p.thread.dl.reset();

                    bool active = false;
                    math::Size2i size;
                    otime::RationalTime frameRate = time::invalidTime;
                    if (enabled)
                    {
                        try
                        {
                            p.thread.dl.reset(new DLWrapper);
                            _createDevice(config, active, size, frameRate);
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
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.active = active;
                        p.mutex.size = p.thread.size;
                        p.mutex.frameRate = frameRate;
                    }

                    glGenBuffers(1, &p.thread.pbo);
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo);
                    glBufferData(
                        GL_PIXEL_PACK_BUFFER,
                        getPackPixelsSize(p.thread.size, p.thread.outputPixelType),
                        NULL,
                        GL_STREAM_READ);
                }

                if (doRender && p.thread.render)
                {
                    try
                    {
                        _render(
                            config,
                            ocioOptions,
                            lutOptions,
                            imageOptions,
                            displayOptions,
                            compareOptions);
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
                }

                if (p.thread.dl && p.thread.dl->outputCallback)
                {
                    p.thread.dl->outputCallback->setPlayback(
                        playback,
                        currentTime - p.thread.timeRange.start_time());
                }
                if (p.thread.dl && p.thread.dl->outputCallback)
                {
                    p.thread.dl->outputCallback->setVolume(volume);
                    p.thread.dl->outputCallback->setMute(mute);
                    p.thread.dl->outputCallback->setAudioOffset(audioOffset);
                }
                if (p.thread.dl && p.thread.dl->outputCallback && audioDataChanged)
                {
                    p.thread.dl->outputCallback->setAudioData(audioData);
                }

                if (p.thread.dl && p.thread.dl->output && p.thread.dl->outputCallback &&
                    doRender && p.thread.render)
                {
                    _read();
                }

                const auto t1 = std::chrono::steady_clock::now();
                const std::chrono::duration<double> diff = t1 - t;
                //std::cout << "diff: " << diff.count() * 1000 << std::endl;
                t = t1;
            }

            if (p.thread.pbo != 0)
            {
                glDeleteBuffers(1, &p.thread.pbo);
                p.thread.pbo = 0;
            }
            p.thread.offscreenBuffer.reset();
            p.thread.render.reset();
            p.thread.dl.reset();
        }

        void BMDOutputDevice::_createDevice(
            const DeviceConfig& config,
            bool& active,
            math::Size2i& size,
            otime::RationalTime& frameRate)
        {
            TLRENDER_P();
            if (config.deviceIndex != -1 &&
                config.displayModeIndex != -1 &&
                config.pixelType != PixelType::None)
            {
                std::string modelName;
                {
                    DLIteratorWrapper dlIterator;
                    if (GetDeckLinkIterator(&dlIterator.p) != S_OK)
                    {
                        throw std::runtime_error("Cannot get iterator");
                    }

                    int count = 0;
                    while (dlIterator->Next(&p.thread.dl->p) == S_OK)
                    {
                        if (count == config.deviceIndex)
                        {
#if defined(__APPLE__)
                            CFStringRef dlModelName;
                            p.thread.dl->p->GetModelName(&dlModelName);
                            StringToStdString(dlModelName, modelName);
                            CFRelease(dlModelName);
#else // __APPLE__
                            dlstring_t dlModelName;
                            p.thread.dl->p->GetModelName(&dlModelName);
                            modelName = DlToStdString(dlModelName);
                            DeleteString(dlModelName);
#endif // __APPLE__
                            break;
                        }

                        p.thread.dl->p->Release();
                        p.thread.dl->p = nullptr;

                        ++count;
                    }
                    if (!p.thread.dl->p)
                    {
                        throw std::runtime_error("Device not found");
                    }
                }

                if (p.thread.dl->p->QueryInterface(IID_IDeckLinkConfiguration, (void**)&p.thread.dl->config) != S_OK)
                {
                    throw std::runtime_error("Cannot get configuration");
                }
                for (const auto& option : config.boolOptions)
                {
                    switch (option.first)
                    {
                    case Option::_444SDIVideoOutput:
                        p.thread.dl->config->SetFlag(bmdDeckLinkConfig444SDIVideoOutput, option.second);
                        break;
                    default: break;
                    }
                }
                if (auto context = p.context.lock())
                {
                    BOOL value = 0;
                    p.thread.dl->config->GetFlag(bmdDeckLinkConfig444SDIVideoOutput, &value);
                    context->log(
                        "tl::device::BMDOutputDevice",
                        string::Format("444 SDI output: {0}").arg(value));
                }

                if (p.thread.dl->p->QueryInterface(IID_IDeckLinkStatus, (void**)&p.thread.dl->status) != S_OK)
                {
                    throw std::runtime_error("Cannot get status");
                }

                if (p.thread.dl->p->QueryInterface(IID_IDeckLinkOutput, (void**)&p.thread.dl->output) != S_OK)
                {
                    throw std::runtime_error("Cannot get output");
                }

                const audio::Info audioInfo(2, audio::DataType::S16, 48000);
                {
                    DLDisplayModeIteratorWrapper dlDisplayModeIterator;
                    if (p.thread.dl->output->GetDisplayModeIterator(&dlDisplayModeIterator.p) != S_OK)
                    {
                        throw std::runtime_error("Cannot get display mode iterator");
                    }
                    DLDisplayModeWrapper dlDisplayMode;
                    int count = 0;
                    while (dlDisplayModeIterator->Next(&dlDisplayMode.p) == S_OK)
                    {
                        if (count == config.displayModeIndex)
                        {
                            break;
                        }

                        dlDisplayMode->Release();
                        dlDisplayMode.p = nullptr;

                        ++count;
                    }
                    if (!dlDisplayMode.p)
                    {
                        throw std::runtime_error("Display mode not found");
                    }

                    p.thread.size.w = dlDisplayMode->GetWidth();
                    p.thread.size.h = dlDisplayMode->GetHeight();
                    p.thread.outputPixelType = getOutputType(config.pixelType);
                    BMDTimeValue frameDuration;
                    BMDTimeScale frameTimescale;
                    dlDisplayMode->GetFrameRate(&frameDuration, &frameTimescale);
                    frameRate = otime::RationalTime(frameDuration, frameTimescale);

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
                            arg(audioInfo.channelCount).
                            arg(audioInfo.dataType).
                            arg(audioInfo.sampleRate));
                    }

                    HRESULT r = p.thread.dl->output->EnableVideoOutput(
                        dlDisplayMode->GetDisplayMode(),
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

                    r = p.thread.dl->output->EnableAudioOutput(
                        bmdAudioSampleRate48kHz,
                        bmdAudioSampleType16bitInteger,
                        audioInfo.channelCount,
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

                p.thread.dl->outputCallback = new DLOutputCallback(
                    p.thread.dl->output,
                    p.thread.size,
                    config.pixelType,
                    frameRate,
                    audioInfo);

                if (p.thread.dl->output->SetScheduledFrameCompletionCallback(p.thread.dl->outputCallback) != S_OK)
                {
                    throw std::runtime_error("Cannot set video callback");
                }

                if (p.thread.dl->output->SetAudioCallback(p.thread.dl->outputCallback) != S_OK)
                {
                    throw std::runtime_error("Cannot set audio callback");
                }

                active = true;
            }
        }

        void BMDOutputDevice::_render(
            const DeviceConfig& config,
            const timeline::OCIOOptions& ocioOptions,
            const timeline::LUTOptions& lutOptions,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            // Create the offscreen buffer.
            const math::Size2i renderSize = timeline::getRenderSize(
                compareOptions.mode,
                p.thread.sizes);
            gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = getOffscreenType(p.thread.outputPixelType);
            if (!displayOptions.empty())
            {
                offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
            }
            offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
            offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
            if (gl::doCreate(p.thread.offscreenBuffer, p.thread.size, offscreenBufferOptions))
            {
                p.thread.offscreenBuffer = gl::OffscreenBuffer::create(p.thread.size, offscreenBufferOptions);
            }

            // Render the video.
            if (p.thread.offscreenBuffer)
            {
                gl::OffscreenBufferBinding binding(p.thread.offscreenBuffer);

                p.thread.render->begin(p.thread.size);
                p.thread.render->setOCIOOptions(ocioOptions);
                p.thread.render->setLUTOptions(lutOptions);

                math::Vector2i viewPosTmp = p.thread.viewPos;
                double viewZoomTmp = p.thread.viewZoom;
                if (p.thread.frameView)
                {
                    double zoom = p.thread.size.w / static_cast<double>(renderSize.w);
                    if (zoom * renderSize.h > p.thread.size.h)
                    {
                        zoom = p.thread.size.h / static_cast<double>(renderSize.h);
                    }
                    const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
                    viewPosTmp.x = p.thread.size.w / 2.0 - c.x * zoom;
                    viewPosTmp.y = p.thread.size.h / 2.0 - c.y * zoom;
                    viewZoomTmp = zoom;
                }
                math::Matrix4x4f vm;
                vm = vm * math::translate(math::Vector3f(viewPosTmp.x, viewPosTmp.y, 0.F));
                vm = vm * math::scale(math::Vector3f(viewZoomTmp, viewZoomTmp, 1.F));
                const auto pm = math::ortho(
                    0.F,
                    static_cast<float>(p.thread.size.w),
                    0.F,
                    static_cast<float>(p.thread.size.h),
                    -1.F,
                    1.F);
                p.thread.render->setTransform(pm * vm);

                p.thread.render->drawVideo(
                    p.thread.videoData,
                    timeline::getBoxes(compareOptions.mode, p.thread.sizes),
                    imageOptions,
                    displayOptions,
                    compareOptions);

                p.thread.render->end();

                glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo);
                glPixelStorei(GL_PACK_ALIGNMENT, getPackPixelsAlign(p.thread.outputPixelType));
                glPixelStorei(GL_PACK_SWAP_BYTES, getPackPixelsSwap(p.thread.outputPixelType));
                glBindTexture(GL_TEXTURE_2D, p.thread.offscreenBuffer->getColorID());
                glGetTexImage(
                    GL_TEXTURE_2D,
                    0,
                    getPackPixelsFormat(p.thread.outputPixelType),
                    getPackPixelsType(p.thread.outputPixelType),
                    NULL);
            }
        }

        void BMDOutputDevice::_read()
        {
            TLRENDER_P();

            auto dlVideoFrame = std::make_shared<DLVideoFrameWrapper>();
            if (p.thread.dl->output->CreateVideoFrame(
                p.thread.size.w,
                p.thread.size.h,
                getRowByteCount(p.thread.size.w, p.thread.outputPixelType),
                toBMD(p.thread.outputPixelType),
                bmdFrameFlagDefault,
                &dlVideoFrame->p) != S_OK)
            {
                throw std::runtime_error("Cannot create video frame");
            }

            void* dlVideoFrameP = nullptr;
            dlVideoFrame->p->GetBytes((void**)&dlVideoFrameP);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo);
            if (void* pboP = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY))
            {
                copyPackPixels(pboP, dlVideoFrameP, p.thread.size, p.thread.outputPixelType);
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }

            p.thread.dl->outputCallback->setVideo(
                dlVideoFrame,
                !p.thread.videoData.empty() ?
                (p.thread.videoData.front().time - p.thread.timeRange.start_time()) :
                time::invalidTime);
        }
    }
}
