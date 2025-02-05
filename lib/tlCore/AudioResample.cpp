// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/AudioResample.h>

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

        struct AudioResample::Private
        {
            audio::Info inputInfo;
            audio::Info outputInfo;
#if defined(TLRENDER_FFMPEG)
            SwrContext* swrContext = nullptr;
#endif // TLRENDER_FFMPEG
        };

        void AudioResample::_init(
            const audio::Info& inputInfo,
            const audio::Info& outputInfo)
        {
            DTK_P();
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

        AudioResample::AudioResample() :
            _p(new Private())
        {}

        AudioResample::~AudioResample()
        {
            DTK_P();
#if defined(TLRENDER_FFMPEG)
            if (p.swrContext)
            {
                swr_free(&p.swrContext);
            }
#endif // TLRENDER_FFMPEG
        }

        std::shared_ptr<AudioResample> AudioResample::create(
            const audio::Info& inputInfo,
            const audio::Info& outputInfo)
        {
            auto out = std::shared_ptr<AudioResample>(new AudioResample);
            out->_init(inputInfo, outputInfo);
            return out;
        }

        const audio::Info& AudioResample::getInputInfo() const
        {
            return _p->inputInfo;
        }

        const audio::Info& AudioResample::getOutputInfo() const
        {
            return _p->outputInfo;
        }

        std::shared_ptr<Audio> AudioResample::process(const std::shared_ptr<Audio>& value)
        {
            DTK_P();
            std::shared_ptr<Audio> out;
#if defined(TLRENDER_FFMPEG)
            if (p.swrContext && value)
            {
                const size_t sampleCount = value->getSampleCount();
                //std::cout << "sampleCount: " << sampleCount << std::endl;
                const int swrOutputSamples = swr_get_out_samples(p.swrContext, sampleCount);
                //std::cout << "swrOutputSamples: " << swrOutputSamples << std::endl;
                auto swrOutputBuffer = Audio::create(p.outputInfo, swrOutputSamples);
                uint8_t* swrOutputBufferP[] = { swrOutputBuffer->getData() };
                const uint8_t* swrInputBufferP[] = { value->getData() };
                const int swrOutputCount = swr_convert(
                    p.swrContext,
                    swrOutputBufferP,
                    swrOutputSamples,
                    swrInputBufferP,
                    sampleCount);
                //std::cout << "swrOutputCount: " << swrOutputCount << std::endl << std::endl;
                out = Audio::create(p.outputInfo, swrOutputCount > 0 ? swrOutputCount : 0);
                memcpy(out->getData(), swrOutputBuffer->getData(), out->getByteCount());
            }
#endif // TLRENDER_FFMPEG
            return out;
        }

        void AudioResample::flush()
        {
            DTK_P();
#if defined(TLRENDER_FFMPEG)
            if (p.swrContext)
            {
                const int drain = swr_get_out_samples(p.swrContext, 0);
                //std::cout << "drain: " << drain << std::endl;
                std::vector<uint8_t> tmp(drain * p.outputInfo.getByteCount(), 0);
                uint8_t* tmpP[] = { tmp.data() };
                swr_convert(
                    p.swrContext,
                    tmpP,
                    drain,
                    nullptr,
                    0);
            }
#endif // TLRENDER_FFMPEG
        }
    }
}
