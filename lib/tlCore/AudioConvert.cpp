// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/AudioConvert.h>

#if defined(TLRENDER_FFMPEG)
extern "C"
{
#include <libswresample/swresample.h>
}
#endif // TLRENDER_FFMPEG

namespace tl
{
    namespace audio
    {
        namespace
        {
#if defined(TLRENDER_FFMPEG)
            AVSampleFormat fromAudioType(audio::DataType value)
            {
                AVSampleFormat out = AV_SAMPLE_FMT_NONE;
                switch (value)
                {
                case audio::DataType::S16: out = AV_SAMPLE_FMT_S16; break;
                case audio::DataType::S32: out = AV_SAMPLE_FMT_S32; break;
                case audio::DataType::F32: out = AV_SAMPLE_FMT_FLT; break;
                case audio::DataType::F64: out = AV_SAMPLE_FMT_DBL; break;
                default: break;
                }
                return out;
            }
#endif // TLRENDER_FFMPEG
        }

        struct AudioConvert::Private
        {
            audio::Info inputInfo;
            audio::Info outputInfo;
#if defined(TLRENDER_FFMPEG)
            SwrContext* swrContext = nullptr;
#endif // TLRENDER_FFMPEG
        };

        void AudioConvert::_init(
            const audio::Info& inputInfo,
            const audio::Info& outputInfo)
        {
            TLRENDER_P();
            p.inputInfo = inputInfo;
            p.outputInfo = outputInfo;
#if defined(TLRENDER_FFMPEG)
            if (p.inputInfo.isValid() && p.outputInfo.isValid())
            {
                AVChannelLayout inputChannelLayout;
                av_channel_layout_default(&inputChannelLayout, p.inputInfo.channelCount);
                AVChannelLayout outputChannelLayout;
                av_channel_layout_default(&outputChannelLayout, p.outputInfo.channelCount);
                int r = swr_alloc_set_opts2(
                    &p.swrContext,
                    &outputChannelLayout,
                    fromAudioType(p.outputInfo.dataType),
                    p.outputInfo.sampleRate,
                    &inputChannelLayout,
                    fromAudioType(p.inputInfo.dataType),
                    p.inputInfo.sampleRate,
                    0,
                    nullptr);
                av_channel_layout_uninit(&inputChannelLayout);
                av_channel_layout_uninit(&outputChannelLayout);
                if (p.swrContext)
                {
                    swr_init(p.swrContext);
                }
            }
#endif // TLRENDER_FFMPEG
        }

        AudioConvert::AudioConvert() :
            _p(new Private())
        {}

        AudioConvert::~AudioConvert()
        {
            TLRENDER_P();
#if defined(TLRENDER_FFMPEG)
            if (p.swrContext)
            {
                swr_free(&p.swrContext);
            }
#endif // TLRENDER_FFMPEG
        }

        std::shared_ptr<AudioConvert> AudioConvert::create(
            const audio::Info& inputInfo,
            const audio::Info& outputInfo)
        {
            auto out = std::shared_ptr<AudioConvert>(new AudioConvert);
            out->_init(inputInfo, outputInfo);
            return out;
        }

        const audio::Info& AudioConvert::getInputInfo() const
        {
            return _p->inputInfo;
        }

        const audio::Info& AudioConvert::getOutputInfo() const
        {
            return _p->outputInfo;
        }

        std::shared_ptr<Audio> AudioConvert::convert(const std::shared_ptr<Audio>& value)
        {
            TLRENDER_P();
            std::shared_ptr<Audio> out;
#if defined(TLRENDER_FFMPEG)
            if (p.swrContext && value)
            {
                const size_t sampleCount = value->getSampleCount();
                const int64_t swrDelay = swr_get_delay(p.swrContext, p.inputInfo.sampleRate);
                //std::cout << "delay: " << swrDelay << std::endl;
                const size_t swrOutputSamples = sampleCount + swrDelay;
                auto tmp = Audio::create(p.outputInfo, swrOutputSamples);
                uint8_t* swrOutputBufferP[] = { tmp->getData() };
                const uint8_t* swrInputBufferP[] = { value->getData() };
                const int swrOutputCount = swr_convert(
                    p.swrContext,
                    swrOutputBufferP,
                    swrOutputSamples,
                    swrInputBufferP,
                    sampleCount);
                out = Audio::create(p.outputInfo, swrOutputCount);
                memcpy(out->getData(), tmp->getData(), out->getByteCount());
            }
#endif // TLRENDER_FFMPEG
            return out;
        }

        void AudioConvert::flush()
        {
            TLRENDER_P();
#if defined(TLRENDER_FFMPEG)
            auto tmp = Audio::create(p.outputInfo, 100);
            uint8_t* swrOutputBufferP[] = { tmp->getData() };
            int r = 0;
            do
            {
                r = swr_convert(
                    p.swrContext,
                    swrOutputBufferP,
                    tmp->getSampleCount(),
                    nullptr,
                    0);
            }
            while (r > 0);
#endif // TLRENDER_FFMPEG
        }
    }
}
