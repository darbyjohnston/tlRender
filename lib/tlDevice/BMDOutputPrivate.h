// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDOutputDevice.h>

#include <tlCore/AudioResample.h>

#if defined(_WINDOWS)
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <atlbase.h>
#endif // _WINDOWS

#include "platform.h"

#if defined(__APPLE__)
typedef int64_t LONGLONG;
#elif defined(__linux__)
typedef bool BOOL;
typedef int64_t LONGLONG;
#endif // __APPLE__

namespace tl
{
    namespace bmd
    {
        class DLIteratorWrapper
        {
        public:
            ~DLIteratorWrapper() { if (p) p->Release(); }

            IDeckLinkIterator* operator -> () const { return p; }

            IDeckLinkIterator* p = nullptr;
        };

        class DLDisplayModeIteratorWrapper
        {
        public:
            ~DLDisplayModeIteratorWrapper() { if (p) p->Release(); }

            IDeckLinkDisplayModeIterator* operator -> () const { return p; }

            IDeckLinkDisplayModeIterator* p = nullptr;
        };

        class DLDisplayModeWrapper
        {
        public:
            ~DLDisplayModeWrapper() { if (p) p->Release(); }

            IDeckLinkDisplayMode* operator -> () const { return p; }

            IDeckLinkDisplayMode* p = nullptr;
        };

        class DLVideoFrameWrapper
        {
        public:
            ~DLVideoFrameWrapper() { if (p) p->Release(); }

            IDeckLinkMutableVideoFrame* operator -> () const { return p; }

            IDeckLinkMutableVideoFrame* p = nullptr;
        };

        class DLFrameConversionWrapper
        {
        public:
            ~DLFrameConversionWrapper() { if (p) p->Release(); }

            IDeckLinkVideoConversion* operator -> () { return p; }

            IDeckLinkVideoConversion* p = nullptr;
        };

        class DLHDRVideoFrame :
            public IDeckLinkVideoFrame,
            public IDeckLinkVideoFrameMetadataExtensions
        {
        public:
            DLHDRVideoFrame(IDeckLinkMutableVideoFrame*, image::HDRData&);

            virtual ~DLHDRVideoFrame();

            HRESULT QueryInterface(REFIID iid, LPVOID* ppv) override;
            ULONG AddRef(void) override;
            ULONG Release(void) override;

            long GetWidth(void) override { return _frame->GetWidth(); }
            long GetHeight(void) override { return _frame->GetHeight(); }
            long GetRowBytes(void) override { return _frame->GetRowBytes(); }
            BMDPixelFormat GetPixelFormat(void) override { return _frame->GetPixelFormat(); }
            BMDFrameFlags GetFlags(void) override { return _frame->GetFlags() | bmdFrameContainsHDRMetadata; }
            HRESULT GetBytes(void** buffer) override { return _frame->GetBytes(buffer); }
            HRESULT GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode** timecode) override { return _frame->GetTimecode(format, timecode); }
            HRESULT GetAncillaryData(IDeckLinkVideoFrameAncillary** ancillary) override { return _frame->GetAncillaryData(ancillary); }

            HRESULT GetInt(BMDDeckLinkFrameMetadataID metadataID, int64_t* value) override;
            HRESULT GetFloat(BMDDeckLinkFrameMetadataID metadataID, double* value) override;
            HRESULT GetFlag(BMDDeckLinkFrameMetadataID metadataID, BOOL* value) override;
#if defined(__APPLE__)
            HRESULT GetString(BMDDeckLinkFrameMetadataID metadataID, CFStringRef* value) override;
#elif defined(_WINDOWS)
            HRESULT GetString(BMDDeckLinkFrameMetadataID metadataID, BSTR* value) override;
#else // __APPLE__
            HRESULT GetString(BMDDeckLinkFrameMetadataID metadataID, const char** value) override;
#endif // __APPLE__
            HRESULT GetBytes(BMDDeckLinkFrameMetadataID metadataID, void* buffer, uint32_t* bufferSize) override;

            void UpdateHDRMetadata(const image::HDRData& metadata) { _hdrData = metadata; }

        private:
            IDeckLinkMutableVideoFrame* _frame = nullptr;
            image::HDRData _hdrData;
            std::atomic<ULONG> _refCount;
        };

        class DLOutputCallback :
            public IDeckLinkVideoOutputCallback,
            public IDeckLinkAudioOutputCallback
        {
        public:
            DLOutputCallback(
                IDeckLinkOutput*,
                const math::Size2i& size,
                PixelType pixelType,
                const FrameRate& frameRate,
                int videoFrameDelay,
                const audio::Info& audioInfo);

            void setPlayback(timeline::Playback, const otime::RationalTime&);
            void seek(const otime::RationalTime&);
            void setVideo(const std::shared_ptr<DLVideoFrameWrapper>&);
            void setVolume(float);
            void setMute(bool);
            void setAudioOffset(double);
            void setAudioData(const std::vector<timeline::AudioData>&);

            HRESULT STDMETHODCALLTYPE ScheduledFrameCompleted(IDeckLinkVideoFrame*, BMDOutputFrameCompletionResult) override;
            HRESULT STDMETHODCALLTYPE ScheduledPlaybackHasStopped() override;

            HRESULT STDMETHODCALLTYPE RenderAudioSamples(BOOL preroll) override;

            HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID* ppv) override;
            ULONG STDMETHODCALLTYPE AddRef() override;
            ULONG STDMETHODCALLTYPE Release() override;

        private:
            IDeckLinkOutput* _dlOutput = nullptr;
            math::Size2i _size;
            PixelType _pixelType = PixelType::None;
            FrameRate _frameRate;
            audio::Info _audioInfo;
            timeline::Playback _playback = timeline::Playback::Stop;
            otime::RationalTime _seek = time::invalidTime;

            std::atomic<size_t> _refCount;

            struct VideoMutex
            {
                std::list<std::shared_ptr<DLVideoFrameWrapper> > videoFrames;
                std::mutex mutex;
            };
            VideoMutex _videoMutex;

            struct VideoThread
            {
                std::shared_ptr<DLVideoFrameWrapper> videoFrame;
#if defined(_WINDOWS)
                CComPtr<IDeckLinkVideoConversion> frameConverter;
#else // _WINDOWS
                DLFrameConversionWrapper frameConverter;
#endif // _WINDOWS
                uint64_t frameCount = 0;
                std::chrono::steady_clock::time_point t;
            };
            VideoThread _videoThread;

            struct AudioMutex
            {
                timeline::Playback playback = timeline::Playback::Stop;
                float volume = 1.F;
                bool mute = false;
                double audioOffset = 0.0;
                std::vector<timeline::AudioData> audioData;
                bool reset = false;
                otime::RationalTime start = time::invalidTime;
                otime::RationalTime current = time::invalidTime;
                std::mutex mutex;
            };
            AudioMutex _audioMutex;

            struct AudioThread
            {
                size_t frame = 0;
                std::shared_ptr<audio::AudioResample> resample;
            };
            AudioThread _audioThread;
        };

        class DLWrapper
        {
        public:
            ~DLWrapper()
            {
                if (outputCallback)
                {
                    outputCallback->Release();
                }
                if (output)
                {
                    output->StopScheduledPlayback(0, nullptr, 0);
                    output->DisableVideoOutput();
                    output->DisableAudioOutput();
                    output->Release();
                }
                if (status)
                {
                    status->Release();
                }
                if (config)
                {
                    config->Release();
                }
                if (p)
                {
                    p->Release();
                }
            }

            IDeckLink* p = nullptr;
            IDeckLinkConfiguration* config = nullptr;
            IDeckLinkStatus* status = nullptr;
            IDeckLinkOutput* output = nullptr;
            DLOutputCallback* outputCallback = nullptr;
        };
    }
}

