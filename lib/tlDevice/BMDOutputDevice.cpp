// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputPrivate.h>

#include <tlDevice/BMDUtil.h>

#include <tlTimelineGL/Render.h>

#include <feather-tk/gl/OffscreenBuffer.h>
#include <feather-tk/gl/Texture.h>
#include <feather-tk/gl/Util.h>
#include <feather-tk/gl/Window.h>

#include <tlCore/AudioResample.h>

#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>

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
    namespace bmd
    {
        namespace
        {
            const size_t videoFrameDelay = 3;
            const std::chrono::milliseconds timeout(5);
        }

        bool FrameRate::operator == (const FrameRate& other) const
        {
            return num == other.num && den == other.den;
        }

        bool FrameRate::operator != (const FrameRate& other) const
        {
            return !(*this == other);
        }

        struct OutputDevice::Private
        {
            std::weak_ptr<feather_tk::Context> context;
            std::shared_ptr<feather_tk::ObservableValue<DeviceConfig> > config;
            std::shared_ptr<feather_tk::ObservableValue<bool> > enabled;
            std::shared_ptr<feather_tk::ObservableValue<bool> > active;
            std::shared_ptr<feather_tk::ObservableValue<feather_tk::Size2I> > size;
            std::shared_ptr<feather_tk::ObservableValue<FrameRate> > frameRate;
            std::shared_ptr<feather_tk::ObservableValue<int> > videoFrameDelay;

            std::shared_ptr<timeline::Player> player;
            std::shared_ptr<feather_tk::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<feather_tk::ValueObserver<double> > speedObserver;
            std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::RationalTime> > seekObserver;
            std::shared_ptr<feather_tk::ListObserver<timeline::VideoData> > videoObserver;
            std::shared_ptr<feather_tk::ListObserver<timeline::AudioData> > audioObserver;

            std::shared_ptr<feather_tk::gl::Window> window;

            struct Mutex
            {
                DeviceConfig config;
                bool enabled = false;
                bool active = false;
                feather_tk::Size2I size;
                FrameRate frameRate;
                int videoFrameDelay = bmd::videoFrameDelay;
                timeline::OCIOOptions ocioOptions;
                timeline::LUTOptions lutOptions;
                std::vector<feather_tk::ImageOptions> imageOptions;
                std::vector<timeline::DisplayOptions> displayOptions;
                HDRMode hdrMode = HDRMode::FromFile;
                image::HDRData hdrData;
                timeline::CompareOptions compareOptions;
                timeline::BackgroundOptions bgOptions;
                timeline::ForegroundOptions fgOptions;
                feather_tk::V2I viewPos;
                double viewZoom = 1.0;
                bool frameView = true;
                OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
                timeline::Playback playback = timeline::Playback::Stop;
                double speed = 0.0;
                OTIO_NS::RationalTime currentTime = time::invalidTime;
                bool seek = false;
                std::vector<timeline::VideoData> videoData;
                std::shared_ptr<feather_tk::Image> overlay;
                float volume = 1.F;
                bool mute = false;
                std::vector<bool> channelMute;
                double audioOffset = 0.0;
                std::vector<timeline::AudioData> audioData;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                std::unique_ptr<DLWrapper> dl;

                feather_tk::Size2I size;
                PixelType outputPixelType = PixelType::None;
                HDRMode hdrMode = HDRMode::FromFile;
                image::HDRData hdrData;
                feather_tk::V2I viewPos;
                double viewZoom = 1.0;
                bool frameView = true;
                OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
                std::vector<timeline::VideoData> videoData;
                std::shared_ptr<feather_tk::Image> overlay;

                std::shared_ptr<timeline::IRender> render;
                std::shared_ptr<feather_tk::gl::OffscreenBuffer> offscreenBuffer;
                GLuint pbo = 0;

                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void OutputDevice::_init(const std::shared_ptr<feather_tk::Context>& context)
        {
            FEATHER_TK_P();

            p.context = context;
            p.config = feather_tk::ObservableValue<DeviceConfig>::create();
            p.enabled = feather_tk::ObservableValue<bool>::create(false);
            p.active = feather_tk::ObservableValue<bool>::create(false);
            p.size = feather_tk::ObservableValue<feather_tk::Size2I>::create();
            p.frameRate = feather_tk::ObservableValue<FrameRate>::create();
            p.videoFrameDelay = feather_tk::ObservableValue<int>::create(bmd::videoFrameDelay);

            p.window = feather_tk::gl::Window::create(
                context,
                "tl::bmd::OutputDevice",
                feather_tk::Size2I(1, 1),
                static_cast<int>(feather_tk::gl::WindowOptions::None));
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    FEATHER_TK_P();
                    p.window->makeCurrent();
                    _run();
                    p.window->doneCurrent();
                });
        }

        OutputDevice::OutputDevice() :
            _p(new Private)
        {}

        OutputDevice::~OutputDevice()
        {
            FEATHER_TK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<OutputDevice> OutputDevice::create(
            const std::shared_ptr<feather_tk::Context>& context)
        {
            auto out = std::shared_ptr<OutputDevice>(new OutputDevice);
            out->_init(context);
            return out;
        }

        DeviceConfig OutputDevice::getConfig() const
        {
            return _p->config->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<DeviceConfig> > OutputDevice::observeConfig() const
        {
            return _p->config;
        }

        void OutputDevice::setConfig(const DeviceConfig& value)
        {
            FEATHER_TK_P();
            if (p.config->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.config = value;
                }
                p.thread.cv.notify_one();
            }
        }

        bool OutputDevice::isEnabled() const
        {
            return _p->enabled->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<bool> > OutputDevice::observeEnabled() const
        {
            return _p->enabled;
        }

        void OutputDevice::setEnabled(bool value)
        {
            FEATHER_TK_P();
            if (p.enabled->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.enabled = value;
                }
                p.thread.cv.notify_one();
            }
        }

        bool OutputDevice::isActive() const
        {
            return _p->active->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<bool> > OutputDevice::observeActive() const
        {
            return _p->active;
        }

        const feather_tk::Size2I& OutputDevice::getSize() const
        {
            return _p->size->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<feather_tk::Size2I> > OutputDevice::observeSize() const
        {
            return _p->size;
        }

        const FrameRate& OutputDevice::getFrameRate() const
        {
            return _p->frameRate->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<FrameRate> > OutputDevice::observeFrameRate() const
        {
            return _p->frameRate;
        }

        int OutputDevice::getVideoFrameDelay() const
        {
            return _p->videoFrameDelay->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<int> > OutputDevice::observeVideoFrameDelay() const
        {
            return _p->videoFrameDelay;
        }

        void OutputDevice::setView(
            const feather_tk::V2I& position,
            double          zoom,
            bool            frame)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.viewPos = position;
                p.mutex.viewZoom = zoom;
                p.mutex.frameView = frame;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.ocioOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setLUTOptions(const timeline::LUTOptions& value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.lutOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setImageOptions(const std::vector<feather_tk::ImageOptions>& value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.imageOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.displayOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setHDR(HDRMode hdrMode, const image::HDRData& hdrData)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.hdrMode = hdrMode;
                p.mutex.hdrData = hdrData;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setCompareOptions(const timeline::CompareOptions& value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.compareOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setBackgroundOptions(const timeline::BackgroundOptions& value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.bgOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setForegroundOptions(const timeline::ForegroundOptions& value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.fgOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setOverlay(const std::shared_ptr<feather_tk::Image>& value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.overlay = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setVolume(float value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.volume = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setMute(bool value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.mute = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setChannelMute(const std::vector<bool>& value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.channelMute = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setAudioOffset(double value)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.audioOffset = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setPlayer(const std::shared_ptr<timeline::Player>& value)
        {
            FEATHER_TK_P();
            if (value == p.player)
                return;

            p.playbackObserver.reset();
            p.speedObserver.reset();
            p.currentTimeObserver.reset();
            p.seekObserver.reset();
            p.videoObserver.reset();
            p.audioObserver.reset();

            p.player = value;

            if (p.player)
            {
                auto weak = std::weak_ptr<OutputDevice>(shared_from_this());
                p.playbackObserver = feather_tk::ValueObserver<timeline::Playback>::create(
                    p.player->observePlayback(),
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
                    },
                    feather_tk::ObserverAction::Suppress);
                p.speedObserver = feather_tk::ValueObserver<double>::create(
                    p.player->observeSpeed(),
                    [weak](double value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.speed = value;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    feather_tk::ObserverAction::Suppress);
                p.currentTimeObserver = feather_tk::ValueObserver<OTIO_NS::RationalTime>::create(
                    p.player->observeCurrentTime(),
                    [weak](const OTIO_NS::RationalTime& value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.currentTime = value;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    feather_tk::ObserverAction::Suppress);
                p.seekObserver = feather_tk::ValueObserver<OTIO_NS::RationalTime>::create(
                    p.player->observeSeek(),
                    [weak](const OTIO_NS::RationalTime&)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.seek = true;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    feather_tk::ObserverAction::Suppress);
                p.videoObserver = feather_tk::ListObserver<timeline::VideoData>::create(
                    p.player->observeCurrentVideo(),
                    [weak](const std::vector<timeline::VideoData>& value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.videoData = value;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    feather_tk::ObserverAction::Suppress);
                p.audioObserver = feather_tk::ListObserver<timeline::AudioData>::create(
                    p.player->observeCurrentAudio(),
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
                    },
                    feather_tk::ObserverAction::Suppress);
            }

            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (p.player)
                {
                    p.mutex.timeRange = p.player->getTimeRange();
                    p.mutex.playback = p.player->getPlayback();
                    p.mutex.speed = p.player->getSpeed();
                    p.mutex.currentTime = p.player->getCurrentTime();
                }
                else
                {
                    p.mutex.timeRange = time::invalidTimeRange;
                    p.mutex.playback = timeline::Playback::Stop;
                    p.mutex.speed = 0.0;
                    p.mutex.currentTime = time::invalidTime;
                }
                p.mutex.videoData.clear();
                p.mutex.audioData.clear();
                if (p.player)
                {
                    p.mutex.videoData = p.player->getCurrentVideo();
                    p.mutex.audioData = p.player->getCurrentAudio();
                }
            }
        }

        void OutputDevice::tick()
        {
            FEATHER_TK_P();
            bool active = false;
            feather_tk::Size2I size = p.size->get();
            FrameRate frameRate = p.frameRate->get();
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

        void OutputDevice::_run()
        {
            FEATHER_TK_P();

            DeviceConfig config;
            bool enabled = false;
            int videoFrameDelay = bmd::videoFrameDelay;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            std::vector<feather_tk::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            timeline::BackgroundOptions bgOptions;
            timeline::ForegroundOptions fgOptions;
            timeline::Playback playback = timeline::Playback::Stop;
            double speed = 0.0;
            OTIO_NS::RationalTime currentTime = time::invalidTime;
            bool seek = false;
            float volume = 1.F;
            bool mute = false;
            std::vector<bool> channelMute;
            double audioOffset = 0.0;
            std::vector<timeline::AudioData> audioData;
            std::shared_ptr<feather_tk::Image> overlay;

            if (auto context = p.context.lock())
            {
                p.thread.render = timeline_gl::Render::create(context);
            }

            auto t = std::chrono::steady_clock::now();
            while (p.thread.running)
            {
                bool createDevice = false;
                bool doRender = false;
                bool audioDataChanged = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        timeout,
                        [this, config, enabled, videoFrameDelay,
                        ocioOptions, lutOptions, imageOptions,
                        displayOptions, compareOptions, bgOptions, fgOptions,
                        playback, speed, currentTime, seek,
                        volume, mute, channelMute, audioOffset, audioData]
                        {
                            return
                                config != _p->mutex.config ||
                                enabled != _p->mutex.enabled ||
                                videoFrameDelay != _p->mutex.videoFrameDelay ||
                                ocioOptions != _p->mutex.ocioOptions ||
                                lutOptions != _p->mutex.lutOptions ||
                                imageOptions != _p->mutex.imageOptions ||
                                displayOptions != _p->mutex.displayOptions ||
                                _p->thread.hdrMode != _p->mutex.hdrMode ||
                                _p->thread.hdrData != _p->mutex.hdrData ||
                                compareOptions != _p->mutex.compareOptions ||
                                bgOptions != _p->mutex.bgOptions ||
                                fgOptions != _p->mutex.fgOptions ||
                                _p->thread.viewPos != _p->mutex.viewPos ||
                                _p->thread.viewZoom != _p->mutex.viewZoom ||
                                _p->thread.frameView != _p->mutex.frameView ||
                                _p->thread.timeRange != _p->mutex.timeRange ||
                                playback != _p->mutex.playback ||
                                speed != _p->mutex.speed ||
                                currentTime != _p->mutex.currentTime ||
                                _p->mutex.seek ||
                                _p->thread.videoData != _p->mutex.videoData ||
                                _p->thread.overlay != _p->mutex.overlay ||
                                volume != _p->mutex.volume ||
                                mute != _p->mutex.mute ||
                                channelMute != _p->mutex.channelMute ||
                                audioOffset != _p->mutex.audioOffset ||
                                audioData != _p->mutex.audioData;
                        }))
                    {
                        createDevice =
                            p.mutex.config != config ||
                            p.mutex.enabled != enabled ||
                            p.mutex.videoFrameDelay != videoFrameDelay;
                        config = p.mutex.config;
                        enabled = p.mutex.enabled;
                        videoFrameDelay = p.mutex.videoFrameDelay;

                        p.thread.timeRange = p.mutex.timeRange;
                        playback = p.mutex.playback;
                        speed = p.mutex.speed;
                        currentTime = p.mutex.currentTime;
                        seek = p.mutex.seek;
                        p.mutex.seek = false;

                        doRender =
                            createDevice ||
                            ocioOptions != p.mutex.ocioOptions ||
                            lutOptions != p.mutex.lutOptions ||
                            imageOptions != p.mutex.imageOptions ||
                            displayOptions != p.mutex.displayOptions ||
                            p.thread.hdrMode != p.mutex.hdrMode ||
                            p.thread.hdrData != p.mutex.hdrData ||
                            compareOptions != p.mutex.compareOptions ||
                            bgOptions != p.mutex.bgOptions ||
                            fgOptions != p.mutex.fgOptions ||
                            p.thread.viewPos != p.mutex.viewPos ||
                            p.thread.viewZoom != p.mutex.viewZoom ||
                            p.thread.frameView != p.mutex.frameView ||
                            p.thread.videoData != p.mutex.videoData ||
                            p.thread.overlay != p.mutex.overlay;
                        ocioOptions = p.mutex.ocioOptions;
                        lutOptions = p.mutex.lutOptions;
                        imageOptions = p.mutex.imageOptions;
                        displayOptions = p.mutex.displayOptions;
                        p.thread.hdrMode = p.mutex.hdrMode;
                        p.thread.hdrData = p.mutex.hdrData;
                        compareOptions = p.mutex.compareOptions;
                        bgOptions = p.mutex.bgOptions;
                        fgOptions = p.mutex.fgOptions;
                        p.thread.viewPos = p.mutex.viewPos;
                        p.thread.viewZoom = p.mutex.viewZoom;
                        p.thread.frameView = p.mutex.frameView;
                        p.thread.videoData = p.mutex.videoData;
                        p.thread.overlay = p.mutex.overlay;

                        audioDataChanged =
                            createDevice ||
                            audioData != p.mutex.audioData;
                        volume = p.mutex.volume;
                        mute = p.mutex.mute;
                        channelMute = p.mutex.channelMute;
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
                    feather_tk::Size2I size;
                    FrameRate frameRate;
                    if (enabled)
                    {
                        try
                        {
                            p.thread.dl.reset(new DLWrapper);
                            _createDevice(
                                config,
                                active,
                                size,
                                frameRate,
                                videoFrameDelay);
                        }
                        catch (const std::exception& e)
                        {
                            if (auto context = p.context.lock())
                            {
                                context->log(
                                    "tl::bmd::OutputDevice",
                                    e.what(),
                                    feather_tk::LogType::Error);
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
                            compareOptions,
                            bgOptions,
                            fgOptions);
                    }
                    catch (const std::exception& e)
                    {
                        if (auto context = p.context.lock())
                        {
                            context->log(
                                "tl::bmd::OutputDevice",
                                e.what(),
                                feather_tk::LogType::Error);
                        }
                    }
                }

                if (p.thread.dl && p.thread.dl->outputCallback)
                {
                    DLOutputCallbackData data;
                    data.playback = playback;
                    data.speed = speed;
                    data.currentTime = currentTime;
                    data.seek = seek;
                    data.volume = volume;
                    data.mute = mute;
                    data.channelMute = channelMute;
                    data.audioOffset = audioOffset;
                    p.thread.dl->outputCallback->setData(data);
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

        void OutputDevice::_createDevice(
            const DeviceConfig& config,
            bool& active,
            feather_tk::Size2I& size,
            FrameRate& frameRate,
            int videoFrameDelay)
        {
            FEATHER_TK_P();
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
                        "tl::bmd::OutputDevice",
                        feather_tk::Format("444 SDI output: {0}").arg(value));
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
                    frameRate.num = static_cast<int>(frameDuration);
                    frameRate.den = static_cast<int>(frameTimescale);

                    if (auto context = p.context.lock())
                    {
                        context->log(
                            "tl::bmd::OutputDevice",
                            feather_tk::Format(
                                "\n"
                                "    #{0} {1}/{2}\n"
                                "    video: {3} {4}\n"
                                "    audio: {5} {6} {7}").
                            arg(config.deviceIndex).
                            arg(modelName).
                            arg(p.thread.size).
                            arg(frameRate.num).
                            arg(frameRate.den).
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
                    videoFrameDelay,
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

        void OutputDevice::_render(
            const DeviceConfig& config,
            const timeline::OCIOOptions& ocioOptions,
            const timeline::LUTOptions& lutOptions,
            const std::vector<feather_tk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            const timeline::BackgroundOptions& bgOptions,
            const timeline::ForegroundOptions& fgOptions)
        {
            FEATHER_TK_P();

            // Create the offscreen buffer.
            const feather_tk::Size2I renderSize = timeline::getRenderSize(
                compareOptions.compare,
                p.thread.videoData);
            feather_tk::gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.color = getColorBuffer(p.thread.outputPixelType);
            if (!displayOptions.empty())
            {
                offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
            }
            offscreenBufferOptions.depth = feather_tk::gl::OffscreenDepth::_24;
            offscreenBufferOptions.stencil = feather_tk::gl::OffscreenStencil::_8;
            if (feather_tk::gl::doCreate(p.thread.offscreenBuffer, p.thread.size, offscreenBufferOptions))
            {
                p.thread.offscreenBuffer = feather_tk::gl::OffscreenBuffer::create(p.thread.size, offscreenBufferOptions);
            }

            // Render the video.
            if (p.thread.offscreenBuffer)
            {
                feather_tk::gl::OffscreenBufferBinding binding(p.thread.offscreenBuffer);

                p.thread.render->begin(p.thread.size);
                p.thread.render->setOCIOOptions(ocioOptions);
                p.thread.render->setLUTOptions(lutOptions);

                const auto pm = feather_tk::ortho(
                    0.F,
                    static_cast<float>(p.thread.size.w),
                    0.F,
                    static_cast<float>(p.thread.size.h),
                    -1.F,
                    1.F);
                p.thread.render->setTransform(pm);

                const auto boxes = timeline::getBoxes(compareOptions.compare, p.thread.videoData);
                feather_tk::V2I viewPosTmp = p.thread.viewPos;
                double viewZoomTmp = p.thread.viewZoom;
                if (p.thread.frameView)
                {
                    double zoom = p.thread.size.w / static_cast<double>(renderSize.w);
                    if (zoom * renderSize.h > p.thread.size.h)
                    {
                        zoom = p.thread.size.h / static_cast<double>(renderSize.h);
                    }
                    const feather_tk::V2I c(renderSize.w / 2, renderSize.h / 2);
                    viewPosTmp.x = p.thread.size.w / 2.0 - c.x * zoom;
                    viewPosTmp.y = p.thread.size.h / 2.0 - c.y * zoom;
                    viewZoomTmp = zoom;
                }
                feather_tk::M44F vm;
                vm = vm * feather_tk::translate(feather_tk::V3F(viewPosTmp.x, viewPosTmp.y, 0.F));
                vm = vm * feather_tk::scale(feather_tk::V3F(viewZoomTmp, viewZoomTmp, 1.F));
                p.thread.render->drawBackground(boxes, vm, bgOptions);

                p.thread.render->setTransform(pm * vm);
                p.thread.render->drawVideo(
                    p.thread.videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions,
                    getColorBuffer(p.thread.outputPixelType));

                p.thread.render->setTransform(pm);
                p.thread.render->drawForeground(boxes, vm, fgOptions);

                if (p.thread.overlay)
                {
                    feather_tk::ImageOptions imageOptions;
                    imageOptions.alphaBlend = feather_tk::AlphaBlend::Premultiplied;
                    p.thread.render->drawImage(
                        p.thread.overlay,
                        feather_tk::Box2I(
                            0,
                            0,
                            p.thread.overlay->getWidth(),
                            p.thread.overlay->getHeight()),
                        feather_tk::Color4F(1.F, 1.F, 1.F),
                        imageOptions);
                }

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

        void OutputDevice::_read()
        {
            FEATHER_TK_P();

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

            p.thread.dl->outputCallback->setVideo(dlVideoFrame);
        }
    }
}
