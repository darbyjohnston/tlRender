// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/WMF.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/LogSystem.h>

/*extern "C"
{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

} // extern "C"*/

#include <combaseapi.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <propvarutil.h>
#include <wmcodecdsp.h>

namespace tl
{
    namespace wmf
    {
        namespace
        {
            const double timeConversion = 10000000.0;
            const size_t requestTimeout = 5;

            const GUID MFVideoFormat_I422 = { FCC('I422'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_I444 = { FCC('I444'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_P010 = { FCC('P010'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_P016 = { FCC('P016'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_P210 = { FCC('P210'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_P216 = { FCC('P216'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_v210 = { FCC('v210'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_v216 = { FCC('v216'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_v40 = { FCC('v40'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_Y210 = { FCC('Y210'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_Y216 = { FCC('Y216'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_Y40 = { FCC('Y40'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
            const GUID MFVideoFormat_Y416 = { FCC('Y416'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };

            std::string videoFormatToString(GUID guid)
            {
                const std::vector<std::pair<GUID, std::string> > data =
                {
                    { MFVideoFormat_RGB8, "RGB8" },
                    { MFVideoFormat_RGB555, "RGB555" },
                    { MFVideoFormat_RGB565, "RGB565" },
                    { MFVideoFormat_RGB24, "RGB24" },
                    { MFVideoFormat_RGB32, "RGB32" },
                    { MFVideoFormat_ARGB32, "ARGB32" },
                    { MFVideoFormat_A2R10G10B10, "A2R10G10B10" },
                    { MFVideoFormat_A16B16G16R16F, "A16B16G16R16F" },

                    { MFVideoFormat_AI44, "AI44" },
                    { MFVideoFormat_AYUV, "AYUV" },
                    { MFVideoFormat_I420, "I420" },
                    { MFVideoFormat_IYUV, "IYUV" },
                    { MFVideoFormat_NV11, "NV11" },
                    { MFVideoFormat_NV12, "NV12" },
                    { MFVideoFormat_NV21, "NV21" },
                    { MFVideoFormat_UYVY, "UYVY" },
                    { MFVideoFormat_Y41P, "Y41P" },
                    { MFVideoFormat_Y41T, "Y41T" },
                    { MFVideoFormat_Y42T, "Y42T" },
                    { MFVideoFormat_YUY2, "YUY2" },
                    { MFVideoFormat_YVU9, "YVU9" },
                    { MFVideoFormat_YV12, "YV12" },
                    { MFVideoFormat_YVYU, "YVYU" },

                    { MFVideoFormat_I422, "I422"},
                    { MFVideoFormat_I444, "I444"},
                    { MFVideoFormat_P010, "P010"},
                    { MFVideoFormat_P016, "P016"},
                    { MFVideoFormat_P210, "P210"},
                    { MFVideoFormat_P216, "P216"},
                    { MFVideoFormat_v210, "v210"},
                    { MFVideoFormat_v216, "v216"},
                    { MFVideoFormat_v40, "v40"},
                    { MFVideoFormat_Y210, "Y210"},
                    { MFVideoFormat_Y216, "Y216"},
                    { MFVideoFormat_Y40, "Y40"},
                    { MFVideoFormat_Y416, "Y416"},

                    { MFVideoFormat_L8, "L8" },
                    { MFVideoFormat_L16, "L16" },
                    { MFVideoFormat_D16, "D16" },

                    { MFVideoFormat_MP43, "MP43" },
                    { MFVideoFormat_MP4S, "MP4S" },
                    { MFVideoFormat_M4S2, "M4S2" },
                    { MFVideoFormat_MP4V, "MP4V" },
                    { MFVideoFormat_WMV1, "WMV1" },
                    { MFVideoFormat_WMV2, "WMV2" },
                    { MFVideoFormat_WMV3, "WMV3" },
                    { MFVideoFormat_WVC1, "WVC1" },
                    { MFVideoFormat_MSS1, "MSS1" },
                    { MFVideoFormat_MSS2, "MSS2" },
                    { MFVideoFormat_MPG1, "MPG1" },
                    { MFVideoFormat_DVSL, "dvsl" },
                    { MFVideoFormat_DVSD, "dvsd" },
                    { MFVideoFormat_DVHD, "dvhd" },
                    { MFVideoFormat_DV25, "dv25" },
                    { MFVideoFormat_DV50, "dv50" },
                    { MFVideoFormat_DVH1, "dvh1" },
                    { MFVideoFormat_DVC, "dvc " },
                    { MFVideoFormat_H264, "H264" },
                    { MFVideoFormat_H265, "H265" },
                    { MFVideoFormat_MJPG, "MJPG" },
                    { MFVideoFormat_420O, "420O" },
                    { MFVideoFormat_HEVC, "HEVC" },
                    { MFVideoFormat_HEVC_ES, "HEVS" },
                    { MFVideoFormat_VP80, "VP80" },
                    { MFVideoFormat_VP90, "VP90" },
                    { MFVideoFormat_ORAW, "ORAW" }
                };
                auto i = std::find_if(
                    data.begin(),
                    data.end(),
                    [guid](const std::pair<GUID, std::string>& value)
                    {
                        return guid == value.first;
                    });
                return i != data.end() ? i->second : std::string();
            }

            std::string audioFormatToString(GUID guid)
            {
                const std::vector<std::pair<GUID, std::string> > data =
                {
                    { MFAudioFormat_PCM, "PCM" },
                    { MFAudioFormat_Float, "Float" },
                    { MFAudioFormat_DRM, "DRM" },
                    { MFAudioFormat_WMAudioV8, "WMAudioV8" },
                    { MFAudioFormat_WMAudioV9, "WMAudioV9" },
                    { MFAudioFormat_WMAudio_Lossless, "WMAudio_Lossless" },
                    { MFAudioFormat_WMASPDIF, "WMASPDIF" },
                    { MFAudioFormat_MSP1, "MSP1" },
                    { MFAudioFormat_MP3, "MP3" },
                    { MFAudioFormat_MPEG, "MPEG" },
                    { MFAudioFormat_AAC, "AAC" },
                    { MFAudioFormat_ADTS, "ADTS" },
                    { MFAudioFormat_AMR_NB, "AMR_NB" },
                    { MFAudioFormat_AMR_WB, "AMR_WB" },
                    { MFAudioFormat_AMR_WP, "AMR_WP" },
                    { MFAudioFormat_FLAC, "FLAC" },
                    { MFAudioFormat_ALAC, "ALAC" },
                    { MFAudioFormat_Dolby_AC4, "Dolby_AC4" },
                    { MFAudioFormat_Dolby_AC3, "Dolby_AC3" },
                    { MFAudioFormat_Dolby_DDPlus, "Dolby_DDPlus" },
                    { MFAudioFormat_Dolby_AC4_V1, "Dolby_AC4_V1" },
                    { MFAudioFormat_Dolby_AC4_V2, "Dolby_AC4_V2" },
                    { MFAudioFormat_Dolby_AC4_V1_ES, "Dolby_AC4_V1_ES" },
                    { MFAudioFormat_Dolby_AC4_V2_ES, "Dolby_AC4_V2_ES" },
                    { MFAudioFormat_MPEGH, "MPEGH" },
                    { MFAudioFormat_MPEGH_ES, "MPEGH_ES" },
                    { MFAudioFormat_Vorbis, "Vorbis" },
                    { MFAudioFormat_DTS_RAW, "DTS_RAW" },
                    { MFAudioFormat_DTS_HD, "DTS_HD" },
                    { MFAudioFormat_DTS_XLL, "DTS_XLL" },
                    { MFAudioFormat_DTS_LBR, "DTS_LBR" },
                    { MFAudioFormat_DTS_UHD, "DTS_UHD" },
                    { MFAudioFormat_DTS_UHDY, "DTS_UHDY" }
                };
                auto i = std::find_if(
                    data.begin(),
                    data.end(),
                    [guid](const std::pair<GUID, std::string>& value)
                    {
                        return guid == value.first;
                    });
                return i != data.end() ? i->second : std::string();
            }
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
            class IMFMediaTypeWrapper
            {
            public:
                ~IMFMediaTypeWrapper()
                {
                    if (p)
                    {
                        p->Release();
                    }
                }

                IMFMediaType* p = nullptr;
            };

            class IMFAttributesWrapper
            {
            public:
                ~IMFAttributesWrapper()
                {
                    if (p)
                    {
                        p->Release();
                    }
                }

                IMFAttributes* p = nullptr;
            };

            class IMFSampleWrapper
            {
            public:
                ~IMFSampleWrapper()
                {
                    if (p)
                    {
                        p->Release();
                    }
                }

                IMFSample* p = nullptr;
            };

            class IMFMediaBufferWrapper
            {
            public:
                ~IMFMediaBufferWrapper()
                {
                    if (p)
                    {
                        p->Release();
                    }
                }

                IMFMediaBuffer* p = nullptr;
            };

            class WMFObject
            {
            public:
                WMFObject(const file::Path& path);

                ~WMFObject();

                double getDuration() const;
                const ftk::ImageInfo& getImageInfo() const;
                double getVideoSpeed() const;
                const audio::Info& WMFObject::getAudioInfo() const;

                std::shared_ptr<ftk::Image> readImage(const OTIO_NS::RationalTime&);

            private:
                int _getFirstStream(GUID);

                bool _comInit = false;
                bool _wmfInit = false;
                IMFSourceReader* _reader = nullptr;
                double _duration = 0.0;
                int _videoStream = -1;
                GUID _videoType = MFVideoFormat_NV12;
                size_t _videoStride = 0;
                ftk::ImageInfo _imageInfo;
                double _videoSpeed = 0.0;
                int _audioStream = -1;
                audio::Info _audioInfo;
                OTIO_NS::RationalTime _time;

                /*AVPixelFormat _avInputPixelFormat = AV_PIX_FMT_P010;
                AVPixelFormat _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                AVFrame* _avFrame = nullptr;
                AVFrame* _avFrame2 = nullptr;
                SwsContext* _swsContext = nullptr;*/

                /*IUnknown* _colorTransformUnknown = nullptr;
                IMFTransform* _colorTransform = nullptr;
                IMFSampleWrapper _outSample;
                IMFMediaBufferWrapper _outBuffer;*/
            };

            WMFObject::WMFObject(const file::Path& path)
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

                // Initialize color converter.
                /*hr = MFTRegisterLocalByCLSID(
                    __uuidof(CColorConvertDMO),
                    MFT_CATEGORY_VIDEO_PROCESSOR,
                    L"",
                    MFT_ENUM_FLAG_SYNCMFT,
                    0,
                    NULL,
                    0,
                    NULL);
                if (FAILED(hr))
                {
                    throw std::runtime_error("Cannot initialize color converter");
                }*/

                // Create source reader.
                IMFAttributesWrapper attr;
                hr = MFCreateAttributes(&attr.p, 1);
                if (FAILED(hr))
                {
                    throw std::runtime_error("Cannot create atrtibutes");
                }
                attr.p->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE);
                attr.p->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, FALSE);
                const std::wstring fileName = ftk::toWide(path.get());
                hr = MFCreateSourceReaderFromURL(
                    fileName.data(),
                    attr.p,
                    &_reader);
                if (FAILED(hr))
                {
                    throw std::runtime_error("Cannot create source reader");
                }

                // Get the duration.
                PROPVARIANT presAttr;
                hr = _reader->GetPresentationAttribute(
                    MF_SOURCE_READER_MEDIASOURCE,
                    MF_PD_DURATION,
                    &presAttr);
                if (SUCCEEDED(hr))
                {
                    LONGLONG durationNanoseconds = 0;
                    hr = PropVariantToInt64(presAttr, &durationNanoseconds);
                    _duration = durationNanoseconds / timeConversion;
                    PropVariantClear(&presAttr);
                }

                // Initialize the video stream.
                _videoStream = _getFirstStream(MFMediaType_Video);
                if (_videoStream != -1)
                {
                    IMFMediaTypeWrapper readerMediaType;
                    hr = _reader->GetNativeMediaType(_videoStream, 0, &readerMediaType.p);

                    GUID subType;
                    readerMediaType.p->GetGUID(MF_MT_SUBTYPE, &subType);
                    std::cout << "video: " << videoFormatToString(subType) << std::endl;
                    _imageInfo.type = ftk::ImageType::YUV_420P_U8;
                    if (MFVideoFormat_HEVC == subType)
                    {
                        _videoType = MFVideoFormat_P010;
                        _imageInfo.type = ftk::ImageType::YUV_420P_U16;
                    }

                    UINT32 width = 0;
                    UINT32 height = 0;
                    MFGetAttributeSize(readerMediaType.p, MF_MT_FRAME_SIZE, &width, &height);
                    _imageInfo.size.w = width;
                    _imageInfo.size.h = height;
                    _imageInfo.layout.mirror.y = true;

                    UINT32 frameRateNum = 0;
                    UINT32 frameRateDen = 0;
                    MFGetAttributeRatio(
                        readerMediaType.p,
                        MF_MT_FRAME_RATE,
                        &frameRateNum,
                        &frameRateDen);
                    if (frameRateDen > 0)
                    {
                        _videoSpeed = frameRateNum / static_cast<double>(frameRateDen);
                    }

                    UINT32 stride = MFGetAttributeUINT32(readerMediaType.p, MF_MT_DEFAULT_STRIDE, 0);
                    std::cout << "stride: " << stride << std::endl;

                    IMFMediaTypeWrapper inputMediaType;
                    MFCreateMediaType(&inputMediaType.p);
                    //readerMediaType.p->CopyAllItems(inputMediaType.p);
                    inputMediaType.p->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
                    inputMediaType.p->SetGUID(MF_MT_SUBTYPE, _videoType);
                    //inputMediaType.p->SetUINT32(MF_MT_DEFAULT_STRIDE, 0);
                    
                    //MFSetAttributeSize(inputMediaType.p, MF_MT_FRAME_SIZE, width, height);
                    hr = _reader->SetCurrentMediaType(_videoStream, nullptr, inputMediaType.p);
                    if (FAILED(hr))
                    {
                        std::cout << "cannot set video format" << std::endl;
                        _videoStream = -1;
                    }

                    stride = MFGetAttributeUINT32(inputMediaType.p, MF_MT_DEFAULT_STRIDE, 0);
                    std::cout << "stride2: " << stride << std::endl;
                    if (stride > 0)
                    {
                        _videoStride = stride;
                    }
                    else
                    {
                        LONG stridel = 0;
                        MFGetStrideForBitmapInfoHeader(subType.Data1, width, &stridel);
                        //std::cout << "stridel: " << stridel << std::endl;
                        if (stridel > 0)
                        {
                            _videoStride = stridel;
                        }
                    }

                    /*_avFrame = av_frame_alloc();
                    if (!_avFrame)
                    {
                        throw std::runtime_error("Cannot allocate sws frame");
                    }
                    //! \bug These fields need to be filled out for
                    //! sws_scale_frame()?
                    _avFrame->format = _avInputPixelFormat;
                    _avFrame->width = width;
                    _avFrame->height = height;
                    av_frame_get_buffer(_avFrame, 1);

                    _avFrame2 = av_frame_alloc();
                    if (!_avFrame2)
                    {
                        throw std::runtime_error("Cannot allocate sws frame");
                    }
                    //! \bug These fields need to be filled out for
                    //! sws_scale_frame()?
                    _avFrame2->format = _avOutputPixelFormat;
                    _avFrame2->width = width;
                    _avFrame2->height = height;
                    av_frame_get_buffer(_avFrame2, 1);

                    _swsContext = sws_alloc_context();
                    if (!_swsContext)
                    {
                        throw std::runtime_error("Cannot allocate sws context");
                    }
                    av_opt_set_defaults(_swsContext);
                    int r = av_opt_set_int(_swsContext, "srcw", width, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(_swsContext, "srch", height, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(_swsContext, "src_format", _avInputPixelFormat, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(_swsContext, "dstw", width, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(_swsContext, "dsth", height, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(_swsContext, "dst_format", _avOutputPixelFormat, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(_swsContext, "sws_flags", SWS_FAST_BILINEAR, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(_swsContext, "threads", 0, AV_OPT_SEARCH_CHILDREN);
                    r = sws_init_context(_swsContext, nullptr, nullptr);
                    if (r < 0)
                    {
                        throw std::runtime_error("Cannot initialize sws context");
                    }

                    const int* inTable = nullptr;
                    int        inFull = 0;
                    const int* outTable = nullptr;
                    int        outFull = 0;
                    int        brightness = 0;
                    int        contrast = 0;
                    int        saturation = 0;
                    r = sws_getColorspaceDetails(
                        _swsContext,
                        (int**)&inTable,
                        &inFull,
                        (int**)&outTable,
                        &outFull,
                        &brightness,
                        &contrast,
                        &saturation);
                    int colorSpace = AVCOL_SPC_UNSPECIFIED;
                    colorSpace = AVCOL_SPC_BT2020_NCL;
                    if (AVCOL_SPC_UNSPECIFIED == colorSpace)
                    {
                        colorSpace = AVCOL_SPC_BT709;
                    }
                    r = sws_setColorspaceDetails(
                        _swsContext,
                        sws_getCoefficients(colorSpace),
                        1,
                        sws_getCoefficients(AVCOL_SPC_BT709),
                        1,
                        brightness,
                        contrast,
                        saturation);*/

                    /*if (_videoStream != -1)
                    {
                        // Create the color converter.
                        hr = CoCreateInstance(
                            CLSID_CColorConvertDMO,
                            nullptr,
                            CLSCTX_INPROC_SERVER,
                            IID_IUnknown,
                            (void**)&_colorTransformUnknown);
                        if (FAILED(hr))
                        {
                            throw std::runtime_error("Cannot create color converter");
                        }
                        hr = _colorTransformUnknown->QueryInterface(IID_PPV_ARGS(&_colorTransform));
                        if (FAILED(hr))
                        {
                            throw std::runtime_error("Cannot query color converter");
                        }
                        IMFMediaTypeWrapper outputMediaType;
                        MFCreateMediaType(&outputMediaType.p);
                        inputMediaType.p->CopyAllItems(outputMediaType.p);
                        outputMediaType.p->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB24);
                        hr = _colorTransform->SetInputType(0, inputMediaType.p, 0);
                        if (FAILED(hr))
                        {
                            throw std::runtime_error("Cannot set color converter input");
                        }
                        hr = _colorTransform->SetOutputType(0, outputMediaType.p, 0);
                        if (FAILED(hr))
                        {
                            throw std::runtime_error("Cannot set color converter output");
                        }
                        DWORD wmfStatus = 0;
                        hr = _colorTransform->GetInputStatus(0, &wmfStatus);
                        if (FAILED(hr))
                        {
                            throw std::runtime_error("Cannot get color converter input status");
                        }
                        if (MFT_INPUT_STATUS_ACCEPT_DATA != wmfStatus)
                        {
                            throw std::runtime_error("Color converter not accepting data");
                        }
                        _colorTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
                        _colorTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
                        _colorTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
                        MFT_OUTPUT_STREAM_INFO streamInfo;
                        _colorTransform->GetOutputStreamInfo(0, &streamInfo);
                        MFCreateSample(&_outSample.p);
                        MFCreateMemoryBuffer(streamInfo.cbSize, &_outBuffer.p);
                        _outSample.p->AddBuffer(_outBuffer.p);
                    }*/
                }

                // Initialize the audio stream.
                _audioStream = _getFirstStream(MFMediaType_Audio);
                if (_audioStream != -1)
                {
                    IMFMediaTypeWrapper readerMediaType;
                    hr = _reader->GetNativeMediaType(_audioStream, 0, &readerMediaType.p);

                    GUID subType;
                    readerMediaType.p->GetGUID(MF_MT_SUBTYPE, &subType);
                    std::cout << "audio: " << audioFormatToString(subType) << std::endl;

                    _audioInfo.channelCount = MFGetAttributeUINT32(readerMediaType.p, MF_MT_AUDIO_NUM_CHANNELS, 0);
                    const UINT32 bitsPerSample = MFGetAttributeUINT32(readerMediaType.p, MF_MT_AUDIO_BITS_PER_SAMPLE, 0);
                    const UINT32 samplesPerSecond = MFGetAttributeUINT32(readerMediaType.p, MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);
                    const double samplesPerSecondF = MFGetAttributeUINT32(readerMediaType.p, MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND, 0.0);
                    switch (bitsPerSample)
                    {
                    case 16:
                        _audioInfo.dataType = audio::DataType::S16;
                        _audioInfo.sampleRate = samplesPerSecond;
                        break;
                    case 32:
                        if (samplesPerSecond > 0)
                        {
                            _audioInfo.dataType = audio::DataType::S32;
                            _audioInfo.sampleRate = samplesPerSecond;
                        }
                        else if (samplesPerSecondF > 0.0)
                        {
                            _audioInfo.dataType = audio::DataType::F32;
                            _audioInfo.sampleRate = samplesPerSecondF;
                        }
                        break;
                    default: break;
                    }
                    IMFMediaTypeWrapper inputMediaType;
                    hr = MFCreateMediaType(&inputMediaType.p);
                    readerMediaType.p->CopyAllItems(inputMediaType.p);
                    inputMediaType.p->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
                    hr = _reader->SetCurrentMediaType(_audioStream, nullptr, inputMediaType.p);
                    if (FAILED(hr))
                    {
                        //std::cout << "cannot set audio format" << std::endl;
                        _audioStream = -1;
                    }
                }
            }

            WMFObject::~WMFObject()
            {
                /*if (_swsContext)
                {
                    sws_freeContext(_swsContext);
                }
                if (_avFrame2)
                {
                    av_frame_free(&_avFrame2);
                }
                if (_avFrame)
                {
                    av_frame_free(&_avFrame);
                }*/
                /*if (_colorTransform)
                {
                    _colorTransform->Release();
                }
                if (_colorTransformUnknown)
                {
                    _colorTransform->Release();
                }*/
                if (_reader)
                {
                    _reader->Release();
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

            const ftk::ImageInfo& WMFObject::getImageInfo() const
            {
                return _imageInfo;
            }

            double WMFObject::getVideoSpeed() const
            {
                return _videoSpeed;
            }

            const audio::Info& WMFObject::getAudioInfo() const
            {
                return _audioInfo;
            }

            std::shared_ptr<ftk::Image> WMFObject::readImage(const OTIO_NS::RationalTime& time)
            {
                std::shared_ptr<ftk::Image> out;
                //std::cout << "read: " << time << std::endl;

                if (_videoStream != -1)
                {
                    HRESULT hr = S_OK;
                    LONGLONG timeStamp;
                    if (time != _time + OTIO_NS::RationalTime(1.0, _videoSpeed))
                    {
                        //std::cout << "seek: " << time << std::endl;
                        PROPVARIANT var;
                        HRESULT hr = InitPropVariantFromInt64(time.rescaled_to(1.0).value() * timeConversion, &var);
                        if (SUCCEEDED(hr))
                        {
                            hr = _reader->SetCurrentPosition(GUID_NULL, var);
                            //if (FAILED(hr))
                            //{
                            //    std::cout << "seek failed" << std::endl;
                            //}
                            PropVariantClear(&var);
                        }
                    }

                    OTIO_NS::RationalTime t;
                    bool end = false;
                    do
                    {
                        DWORD flags = 0;
                        IMFSampleWrapper sample;
                        hr = _reader->ReadSample(
                            _videoStream,
                            0,
                            nullptr,
                            &flags,
                            &timeStamp,
                            &sample.p);
                        if (FAILED(hr))
                        {
                            //std::cout << "failed to read sample" << std::endl;
                            break;
                        }
                        if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
                        {
                            //std::cout << "end of stream" << std::endl;
                            end = true;
                        }

                        t = OTIO_NS::RationalTime(
                            timeStamp / timeConversion * _videoSpeed,
                            _videoSpeed).round();
                        if (sample.p && t >= time)
                        {
                            //std::cout << "t: " << t << std::endl;
                            end = true;
                            _time = time;

                            /*hr = _colorTransform->ProcessInput(0, sample.p, NULL);
                            if (FAILED(hr))
                            {
                                std::cout << "failed to process color input" << std::endl;
                                break;
                            }

                            MFT_OUTPUT_DATA_BUFFER outputDataBuffer;
                            outputDataBuffer.dwStreamID = 0;
                            outputDataBuffer.dwStatus = 0;
                            outputDataBuffer.pEvents = NULL;
                            outputDataBuffer.pSample = _outSample.p;
                            DWORD processOutputStatus = 0;
                            _colorTransform->ProcessOutput(0, 1, &outputDataBuffer, &processOutputStatus);
                            if (FAILED(processOutputStatus))
                            {
                                std::cout << "failed to process color output" << std::endl;
                                break;
                            }*/

                            out = ftk::Image::create(_imageInfo);
                            //out->zero();

                            /*IMF2DBuffer* buf2D = nullptr;
                            _outSample->QueryInterface(IID_PPV_ARGS(&buf2D));
                            if (buf2D)
                            {
                                BYTE* bufP = nullptr;
                                LONG bufPitch = 0;
                                hr = buf2D->Lock2D(&bufP, &bufPitch);
                                if (SUCCEEDED(hr))
                                {}
                            }*/

                            IMFMediaBuffer* buf = nullptr;
                            //hr = _outSample.p->ConvertToContiguousBuffer(&buf);
                            hr = sample.p->ConvertToContiguousBuffer(&buf);
                            if (SUCCEEDED(hr))
                            {
                                BYTE* bufP = nullptr;
                                DWORD bufLen = 0;
                                hr = buf->Lock(&bufP, nullptr, &bufLen);
                                if (SUCCEEDED(hr))
                                {
                                    //memcpy(out->getData(), bufP, out->getByteCount());

                                    const int w = _imageInfo.size.w;
                                    const int h = _imageInfo.size.h;
                                    if (MFVideoFormat_NV12 == _videoType)
                                    {
                                        uint8_t* outP = out->getData();
                                        size_t stride = _videoStride > 0 ? _videoStride : w;
                                        for (int y = 0; y < h; ++y, bufP += stride, outP += w)
                                        {
                                            memcpy(outP, bufP, w);
                                        }
                                        uint8_t* outP2 = outP + w / 2 * h / 2;
                                        for (int y = 0; y < h / 2; ++y, bufP += stride, outP += w / 2, outP2 += w / 2)
                                        {
                                            for (int x = 0; x < w / 2; ++x)
                                            {
                                                outP[x] = bufP[x * 2];
                                                outP2[x] = bufP[x * 2 + 1];
                                            }
                                        }
                                    }
                                    else if (MFVideoFormat_P010 == _videoType)
                                    {
                                        uint16_t* bufP16 = reinterpret_cast<uint16_t*>(bufP);
                                        uint16_t* outP16 = reinterpret_cast<uint16_t*>(out->getData());
                                        size_t stride = _videoStride > 0 ? _videoStride : w;
                                        for (int y = 0; y < h; ++y, bufP16 += stride, outP16 += w)
                                        {
                                            memcpy(outP16, bufP16, w * 2);
                                        }
                                        uint16_t* outP16_2 = outP16 + w / 2 * h / 2;
                                        for (int y = 0; y < h / 2; ++y, bufP16 += stride, outP16 += w / 2, outP16_2 += w / 2)
                                        {
                                            for (int x = 0; x < w / 2; ++x)
                                            {
                                                outP16[x] = bufP16[x * 2];
                                                outP16_2[x] = bufP16[x * 2 + 1];
                                            }
                                        }
                                    }

                                    /*av_image_fill_arrays(
                                        _avFrame->data,
                                        _avFrame->linesize,
                                        bufP,
                                        _avInputPixelFormat,
                                        _imageInfo.size.w,
                                        _imageInfo.size.h,
                                        1);
                                    av_image_fill_arrays(
                                        _avFrame2->data,
                                        _avFrame2->linesize,
                                        out->getData(),
                                        _avOutputPixelFormat,
                                        _imageInfo.size.w,
                                        _imageInfo.size.h,
                                        1);
                                    sws_scale_frame(_swsContext, _avFrame2, _avFrame);*/

                                    buf->Unlock();
                                }
                                buf->Release();
                            }
                        }
                        else
                        {
                            //std::cout << "  skip: " << t << std::endl;
                        }
                    } while (t < time && !end);
                }

                return out;
            }

            int WMFObject::_getFirstStream(GUID guid)
            {
                int out = -1;
                HRESULT hr = S_OK;
                for (int i = 0; -1 == out && SUCCEEDED(hr); ++i)
                {
                    IMFMediaTypeWrapper mediaType;
                    hr = _reader->GetNativeMediaType(i, 0, &mediaType.p);
                    if (SUCCEEDED(hr))
                    {
                        GUID majorType;
                        mediaType.p->GetMajorType(&majorType);
                        if (majorType == guid)
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
                        _thread(path);
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::wmf::Read",
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

        void Read::_thread(const file::Path& path)
        {
            FTK_P();

            WMFObject wmf(path);
            p.info.video.push_back(wmf.getImageInfo());
            p.info.videoTime = OTIO_NS::TimeRange(
                OTIO_NS::RationalTime(0.0, wmf.getVideoSpeed()),
                OTIO_NS::RationalTime(wmf.getDuration() * wmf.getVideoSpeed(), wmf.getVideoSpeed()).floor());
            p.info.audio = wmf.getAudioInfo();
            p.info.audioTime = OTIO_NS::TimeRange(
                OTIO_NS::RationalTime(0.0, p.info.audio.sampleRate),
                OTIO_NS::RationalTime(wmf.getDuration() * p.info.audio.sampleRate, p.info.audio.sampleRate).floor());

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

                // Handle video requests.
                if (videoRequest)
                {
                    io::VideoData data;
                    data.time = videoRequest->time;
                    data.image = wmf.readImage(videoRequest->time);
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
