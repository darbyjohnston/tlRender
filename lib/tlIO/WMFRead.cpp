// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/WMF.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/LogSystem.h>

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

        namespace
        {
            class WMFMediaTypeWrapper
            {
            public:
                ~WMFMediaTypeWrapper()
                {
                    if (p)
                    {
                        p->Release();
                    }
                }

                IMFMediaType* p = nullptr;
            };

            class WMFObject
            {
            public:
                WMFObject(
                    const file::Path& path,
                    const std::vector<ftk::InMemoryFile>& memory);

                ~WMFObject();

                double getDuration() const;
                ftk::ImageInfo getImageInfo() const;
                double getVideoSpeed() const;

                void seek(double);

                std::shared_ptr<ftk::Image> readImage();

            private:
                int _getFirstVideoStream();

                bool _comInit = false;
                bool _wmfInit = false;
                IMFSourceReader* _wmfReader = nullptr;
                double _duration = 0.0;
                int _videoStream = -1;
                ftk::ImageInfo _imageInfo;
                double _videoSpeed = 0.0;
                double _seek = -1.0;
            };

            WMFObject::WMFObject(
                const file::Path& path,
                const std::vector<ftk::InMemoryFile>& memory)
            {
                // Initialize COM.
                HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
                if (FAILED(hr))
                {
                    throw std::runtime_error("Cannot initialize COM");
                }
                _comInit = true;

                // Initialize WMF.
                hr = MFStartup(MF_VERSION);
                if (FAILED(hr))
                {
                    throw std::runtime_error("Cannot initialize WMF");
                }
                _wmfInit = true;

                // Create source reader.
                IMFAttributes* wmfAttr = NULL;
                hr = MFCreateAttributes(&wmfAttr, 1);
                if (FAILED(hr))
                {
                    throw std::runtime_error("Cannot create atrtibutes");
                }
                //wmfAttr->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
                //wmfAttr->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
                const std::wstring fileName = ftk::toWide(path.get());
                hr = MFCreateSourceReaderFromURL(
                    fileName.data(),
                    wmfAttr,
                    &_wmfReader);
                wmfAttr->Release();
                if (FAILED(hr))
                {
                    throw std::runtime_error("Cannot create source reader");
                }

                // Get the duration.
                PROPVARIANT mfPresAttr;
                hr = _wmfReader->GetPresentationAttribute(
                    MF_SOURCE_READER_MEDIASOURCE,
                    MF_PD_DURATION,
                    &mfPresAttr);
                if (SUCCEEDED(hr))
                {
                    LONGLONG durationNanoseconds = 0;
                    hr = PropVariantToInt64(mfPresAttr, &durationNanoseconds);
                    _duration = durationNanoseconds / 10000000.0;
                    PropVariantClear(&mfPresAttr);
                }

                // Initialize the video stream.
                _videoStream = _getFirstVideoStream();
                if (_videoStream != -1)
                {
                    WMFMediaTypeWrapper wmfMediaType;
                    hr = _wmfReader->GetNativeMediaType(_videoStream, 0, &wmfMediaType.p);

                    GUID subType;
                    wmfMediaType.p->GetGUID(MF_MT_SUBTYPE, &subType);

                    UINT32 width = 0;
                    UINT32 height = 0;
                    MFGetAttributeSize(wmfMediaType.p, MF_MT_FRAME_SIZE, &width, &height);
                    _imageInfo.size.w = width;
                    _imageInfo.size.h = height;
                    //_imageInfo.type = ftk::ImageType::RGBA_U8;
                    _imageInfo.type = ftk::ImageType::YUV_420P_U8;

                    UINT32 frameRateNum = 0;
                    UINT32 frameRateDen = 0;
                    MFGetAttributeRatio(
                        wmfMediaType.p,
                        MF_MT_FRAME_RATE,
                        &frameRateNum,
                        &frameRateDen);
                    if (frameRateDen > 0)
                    {
                        _videoSpeed = frameRateNum / static_cast<double>(frameRateDen);
                    }

                    WMFMediaTypeWrapper wmfMediaType2;
                    hr = MFCreateMediaType(&wmfMediaType2.p);
                    if (SUCCEEDED(hr))
                    {
                        wmfMediaType2.p->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
                        //wmfMediaType2.p->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
                        wmfMediaType2.p->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_I420);
                        hr = _wmfReader->SetCurrentMediaType(_videoStream, nullptr, wmfMediaType2.p);
                        if (FAILED(hr))
                        {
                            throw std::runtime_error("Cannot set video format");
                        }
                    }
                }
            }

            WMFObject::~WMFObject()
            {
                if (_wmfReader)
                {
                    _wmfReader->Release();
                }
                if (_wmfInit)
                {
                    MFShutdown();
                }
                if (_comInit)
                {
                    CoUninitialize();
                }
            }

            double WMFObject::getDuration() const
            {
                return _duration;
            }

            ftk::ImageInfo WMFObject::getImageInfo() const
            {
                return _imageInfo;
            }

            double WMFObject::getVideoSpeed() const
            {
                return _videoSpeed;
            }

            void WMFObject::seek(double value)
            {
                _seek = value;
                PROPVARIANT var;
                HRESULT hr = InitPropVariantFromInt64(value * 10000000.0, &var);
                if (SUCCEEDED(hr))
                {
                    hr = _wmfReader->SetCurrentPosition(GUID_NULL, var);
                    PropVariantClear(&var);
                }
            }

            std::shared_ptr<ftk::Image> WMFObject::readImage()
            {
                std::shared_ptr<ftk::Image> out;
                HRESULT hr = S_OK;
                LONGLONG timeStamp;
                do
                {
                    DWORD flags;
                    IMFSample* sample = nullptr;
                    hr = _wmfReader->ReadSample(
                        _videoStream,
                        0,
                        nullptr,
                        &flags,
                        &timeStamp,
                        &sample);
                    if (FAILED(hr))
                    {
                        break;
                    }
                    if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
                    {
                        break;
                    }
                    if (timeStamp / 10000000.0 >= _seek)
                    {
                        IMFMediaBuffer* buf = nullptr;
                        sample->ConvertToContiguousBuffer(&buf);
                        //DWORD bufLen = 0;
                        //buf->GetCurrentLength(&bufLen);
                        BYTE* bufP = nullptr;
                        DWORD bufLen = 0;
                        buf->Lock(&bufP, nullptr, &bufLen);
                        out = ftk::Image::create(_imageInfo);
                        memcpy(out->getData(), bufP, out->getByteCount());
                        buf->Unlock();
                        buf->Release();
                        sample->Release();
                    }
                }
                while (timeStamp / 10000000.0 < _seek);
                return out;
            }

            int WMFObject::_getFirstVideoStream()
            {
                int out = -1;
                HRESULT hr = S_OK;
                for (int i = 0; -1 == out && SUCCEEDED(hr); ++i)
                {
                    WMFMediaTypeWrapper wmfMediaType;
                    hr = _wmfReader->GetNativeMediaType(i, 0, &wmfMediaType.p);
                    if (SUCCEEDED(hr))
                    {
                        GUID majorType;
                        wmfMediaType.p->GetMajorType(&majorType);
                        if (majorType == MFMediaType_Video)
                        {
                            out = i;
                        }
                    }
                }
                return out;
            }
        }

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
                [this, path, memory]
                {
                    FTK_P();
                    try
                    {
                        _thread(path, memory);
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ffmpeg::Read",
                                e.what(),
                                ftk::LogType::Error);
                        }
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.stopped = true;
                    }
                    cancelRequests();
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

        void Read::_thread(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory)
        {
            FTK_P();

            WMFObject wmf(path, memory);
            p.info.video.push_back(wmf.getImageInfo());
            p.info.videoTime = OTIO_NS::TimeRange(
                OTIO_NS::RationalTime(0.0, wmf.getVideoSpeed()),
                OTIO_NS::RationalTime(wmf.getDuration() * wmf.getVideoSpeed(), wmf.getVideoSpeed()));

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
                    wmf.seek(videoRequest->time.rescaled_to(1.0).value() * 10000000.0);
                    p.thread.videoTime = videoRequest->time;
                }

                // Handle video requests.
                if (videoRequest)
                {
                    io::VideoData data;
                    data.time = videoRequest->time;
                    data.image = wmf.readImage();
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
