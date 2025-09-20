// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/WMF.h>

#include <combaseapi.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <propvarutil.h>
#include <mfreadwrite.h>

namespace tl
{
    namespace wmf
    {
        namespace
        {
            const size_t requestTimeout = 5;
        }

        struct Read::Private
        {
            io::Info info;
            struct InfoRequest
            {
                std::promise<io::Info> promise;
            };

            struct VideoRequest
            {
                OTIO_NS::RationalTime time = time::invalidTime;
                io::Options options;
                std::promise<io::VideoData> promise;
            };

            struct AudioRequest
            {
                OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
                io::Options options;
                std::promise<io::AudioData> promise;
            };

            struct Mutex
            {
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<VideoRequest> > videoRequests;
                std::list<std::shared_ptr<AudioRequest> > audioRequests;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                OTIO_NS::RationalTime videoTime = time::invalidTime;
                OTIO_NS::RationalTime audioTime = time::invalidTime;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void Read::_init(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);
            FTK_P();

            p.thread.running = true;
            p.thread.thread = std::thread(
                [this, path]
                {
                    FTK_P();

                    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
                    if (SUCCEEDED(hr))
                    {
                        hr = MFStartup(MF_VERSION);
                        if (SUCCEEDED(hr))
                        {
                            const std::wstring fileName = ftk::toWide(path.get());
                            IMFSourceReader* mfReader = nullptr;
                            hr = MFCreateSourceReaderFromURL(
                                fileName.data(),
                                nullptr,
                                &mfReader);
                            if (SUCCEEDED(hr))
                            {
                                ftk::Size2I size;
                                double speed = 0.0;
                                double duration = 0.0;

                                IMFMediaType* mfMediaType = nullptr;
                                hr = mfReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &mfMediaType);
                                if (SUCCEEDED(hr))
                                {
                                    GUID subtype;
                                    hr = mfMediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
                                    UINT32 width = 0;
                                    UINT32 height = 0;
                                    MFGetAttributeSize(mfMediaType, MF_MT_FRAME_SIZE, &width, &height);
                                    size.w = width;
                                    size.h = height;

                                    UINT32 frameRateNum = 0;
                                    UINT32 frameRateDen = 0;
                                    MFGetAttributeRatio(
                                        mfMediaType,
                                        MF_MT_FRAME_RATE,
                                        &frameRateNum,
                                        &frameRateDen);
                                    if (frameRateDen > 0)
                                    {
                                        speed = frameRateNum / static_cast<double>(frameRateDen);
                                    }

                                    mfMediaType->Release();
                                }

                                PROPVARIANT mfPresAttr;
                                HRESULT hr = mfReader->GetPresentationAttribute(
                                    MF_SOURCE_READER_MEDIASOURCE,
                                    MF_PD_DURATION,
                                    &mfPresAttr);
                                if (SUCCEEDED(hr))
                                {
                                    LONGLONG durationNanoseconds = 0;
                                    hr = PropVariantToInt64(mfPresAttr, &durationNanoseconds);
                                    duration = durationNanoseconds / 10000000.0;
                                    PropVariantClear(&mfPresAttr);
                                }

                                p.info.video.push_back(ftk::ImageInfo(size, ftk::ImageType::RGB_U8));
                                p.info.videoTime = OTIO_NS::TimeRange(
                                    OTIO_NS::RationalTime(0.0, speed),
                                    OTIO_NS::RationalTime(duration * speed, speed));

                                _thread();

                                {
                                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                                    p.mutex.stopped = true;
                                }
                                cancelRequests();

                                mfReader->Release();
                            }
                            MFShutdown();
                        }
                        CoUninitialize();
                    }
                });
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {
            FTK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        std::future<io::Info> Read::getInfo()
        {
            FTK_P();
            auto request = std::make_shared<Private::InfoRequest>();
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::Info());
            }
            return future;
        }

        std::future<io::VideoData> Read::readVideo(
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            FTK_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->options = io::merge(options, _options);
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::VideoData());
            }
            return future;
        }

        std::future<io::AudioData> Read::readAudio(
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            FTK_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->timeRange = timeRange;
            request->options = io::merge(options, _options);
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.audioRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::AudioData());
            }
            return future;
        }

        void Read::cancelRequests()
        {
            FTK_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                infoRequests = std::move(p.mutex.infoRequests);
                videoRequests = std::move(p.mutex.videoRequests);
                audioRequests = std::move(p.mutex.audioRequests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(io::Info());
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(io::VideoData());
            }
            for (auto& request : audioRequests)
            {
                request->promise.set_value(io::AudioData());
            }
        }

        void Read::_thread()
        {
            FTK_P();
            while (p.thread.running)
            {
                // Check requests.
                std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
                std::shared_ptr<Private::VideoRequest> videoRequest;
                std::shared_ptr<Private::AudioRequest> audioRequest;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(requestTimeout),
                        [this]
                        {
                            return
                                !_p->mutex.infoRequests.empty() ||
                                !_p->mutex.videoRequests.empty() ||
                                !_p->mutex.audioRequests.empty();
                        }))
                    {
                        infoRequests = std::move(p.mutex.infoRequests);
                        if (!p.mutex.videoRequests.empty())
                        {
                            videoRequest = p.mutex.videoRequests.front();
                            p.mutex.videoRequests.pop_front();
                        }
                        if (!p.mutex.audioRequests.empty())
                        {
                            audioRequest = p.mutex.audioRequests.front();
                            p.mutex.audioRequests.pop_front();
                        }
                    }
                }

                // Information requests.
                for (auto& request : infoRequests)
                {
                    request->promise.set_value(p.info);
                }

                // Seek.
                if (videoRequest &&
                    !videoRequest->time.strictly_equal(p.thread.videoTime))
                {
                    p.thread.videoTime = videoRequest->time;
                }

                // Handle video requests.
                if (videoRequest)
                {
                    io::VideoData data;
                    data.time = videoRequest->time;
                    videoRequest->promise.set_value(data);

                    p.thread.videoTime += OTIO_NS::RationalTime(1.0, p.info.videoTime.duration().rate());
                }

                // Handle audio requests.
                if (audioRequest)
                {
                    io::AudioData audioData;
                    audioData.time = audioRequest->timeRange.start_time();
                    audioData.audio = audio::Audio::create(p.info.audio, audioRequest->timeRange.duration().value());
                    audioData.audio->zero();
                    audioRequest->promise.set_value(audioData);

                    p.thread.audioTime += audioRequest->timeRange.duration();
                }
            }
        }
    }
}
