// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlCore/Audio.h>

#include <ftk/Core/Error.h>
#include <ftk/Core/String.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace audio
    {
        FTK_ENUM_IMPL(
            DataType,
            "None",
            "S8",
            "S16",
            "S32",
            "F32",
            "F64");

        size_t getByteCount(DataType value)
        {
            const std::array<uint8_t, static_cast<size_t>(DataType::Count)> data =
            {
                0,
                1,
                2,
                4,
                4,
                8
            };
            return data[static_cast<size_t>(value)];
        }

        DataType getIntType(size_t value)
        {
            const std::array<DataType, 9> data =
            {
                DataType::None,
                DataType::S8,
                DataType::S16,
                DataType::None,
                DataType::S32,
                DataType::None,
                DataType::None,
                DataType::None,
                DataType::None
            };
            return value < data.size() ? data[value] : DataType::None;
        }

        DataType getFloatType(size_t value)
        {
            const std::array<DataType, 9> data =
            {
                DataType::None,
                DataType::None,
                DataType::None,
                DataType::None,
                DataType::F32,
                DataType::None,
                DataType::None,
                DataType::None,
                DataType::F64
            };
            return value < data.size() ? data[value] : DataType::None;
        }

        Info::Info()
        {}

        Info::Info(size_t channelCount, DataType dataType, size_t sampleRate) :
            channelCount(channelCount),
            dataType(dataType),
            sampleRate(sampleRate)
        {}

        void Audio::_init(const Info& info, size_t sampleCount)
        {
            _info = info;
            _sampleCount = sampleCount;
            const size_t byteCount = getByteCount();
            _data.resize(byteCount);
        }

        Audio::Audio()
        {}

        Audio::~Audio()
        {}

        std::shared_ptr<Audio> Audio::create(
            const Info& info,
            size_t sampleCount)
        {
            auto out = std::shared_ptr<Audio>(new Audio);
            out->_init(info, sampleCount);
            return out;
        }

        void Audio::zero()
        {
            std::memset(_data.data(), 0, getByteCount());
        }

        std::shared_ptr<audio::Audio> combine(const std::list<std::shared_ptr<audio::Audio> >& chunks)
        {
            std::shared_ptr<audio::Audio> out;
            size_t size = 0;
            for (const auto& chunk : chunks)
            {
                size += chunk->getSampleCount();
            }
            if (size > 0)
            {
                out = audio::Audio::create(chunks.front()->getInfo(), size);
                uint8_t* p = out->getData();
                for (const auto& chunk : chunks)
                {
                    memcpy(p, chunk->getData(), chunk->getByteCount());
                    p += chunk->getByteCount();
                }
            }
            return out;
        }

        namespace
        {
            template<typename T, typename TI>
            void mixI(
                const uint8_t** in,
                size_t inCount,
                uint8_t* out,
                float* volume,
                size_t channelCount,
                size_t sampleCount)
            {
                const T** inP = reinterpret_cast<const T**>(in);
                T* outP = reinterpret_cast<T*>(out);
                const TI min = static_cast<TI>(std::numeric_limits<T>::min());
                const TI max = static_cast<TI>(std::numeric_limits<T>::max());
                for (size_t i = 0; i < sampleCount; ++i, outP += channelCount)
                {
                    for (size_t j = 0; j < channelCount; ++j)
                    {
                        TI v = 0;
                        for (size_t k = 0; k < inCount; ++k)
                        {
                            v += ftk::clamp(static_cast<TI>(inP[k][i * channelCount + j] * volume[j]), min, max);
                        }
                        outP[j] = ftk::clamp(v, min, max);
                    }
                }
            }

            template<typename T>
            void mixF(
                const uint8_t** in,
                size_t inCount,
                uint8_t* out,
                float* volume,
                size_t channelCount,
                size_t sampleCount)
            {
                const T** inP = reinterpret_cast<const T**>(in);
                T* outP = reinterpret_cast<T*>(out);
                for (size_t i = 0; i < sampleCount; ++i, outP += channelCount)
                {
                    for (size_t j = 0; j < channelCount; ++j)
                    {
                        T v = static_cast<T>(0);
                        for (size_t k = 0; k < inCount; ++k)
                        {
                            v += inP[k][i * channelCount + j] * volume[j];
                        }
                        outP[j] = v;
                    }
                }
            }
        }

        std::shared_ptr<Audio> mix(
            const std::vector<std::shared_ptr<Audio> >& in,
            float volume,
            const std::vector<bool>& channelMute)
        {
            std::shared_ptr<Audio> out;
            if (!in.empty())
            {
                const Info& info = in.front()->getInfo();
                const size_t sampleCount = in.front()->getSampleCount();
                out = Audio::create(info, sampleCount);
                std::vector<const uint8_t*> inP;
                for (size_t i = 0; i < in.size(); ++i)
                {
                    inP.push_back(in[i]->getData());
                }
                std::vector<float> channelVolumes;
                for (size_t i = 0; i < info.channelCount; ++i)
                {
                    channelVolumes.push_back(
                        i < channelMute.size() && channelMute[i] ?
                        0.F :
                        volume);
                }
                switch (info.dataType)
                {
                case DataType::S8:
                    mixI<int8_t, int16_t>(
                        inP.data(),
                        inP.size(),
                        out->getData(),
                        channelVolumes.data(),
                        info.channelCount,
                        sampleCount);
                    break;
                case DataType::S16:
                    mixI<int16_t, int32_t>(
                        inP.data(),
                        inP.size(),
                        out->getData(),
                        channelVolumes.data(),
                        info.channelCount,
                        sampleCount);
                    break;
                case DataType::S32:
                    mixI<int32_t, int64_t>(
                        inP.data(),
                        inP.size(),
                        out->getData(),
                        channelVolumes.data(),
                        info.channelCount,
                        sampleCount);
                    break;
                case DataType::F32:
                    mixF<float>(
                        inP.data(),
                        inP.size(),
                        out->getData(),
                        channelVolumes.data(),
                        info.channelCount,
                        sampleCount);
                    break;
                case DataType::F64:
                    mixF<double>(
                        inP.data(),
                        inP.size(),
                        out->getData(),
                        channelVolumes.data(),
                        info.channelCount,
                        sampleCount);
                    break;
                default: break;
                }
            }
            return out;
        }

        namespace
        {
            template<typename T>
            void reverseT(
                const uint8_t* in,
                uint8_t*       out,
                size_t         sampleCount,
                size_t         channelCount)
            {
                const T* inP = reinterpret_cast<const T*>(in) +
                    (sampleCount - 1) * channelCount;
                T* outP = reinterpret_cast<T*>(out);
                for (size_t i = 0; i < sampleCount; ++i, inP -= channelCount, outP += channelCount)
                {
                    for (size_t j = 0; j < channelCount; ++j)
                    {
                        outP[j] = inP[j];
                    }
                }
            }
        }

        std::shared_ptr<Audio> reverse(const std::shared_ptr<Audio>& audio)
        {
            const Info& info = audio->getInfo();
            const size_t sampleCount = audio->getSampleCount();
            auto out = Audio::create(info, sampleCount);
            switch (info.dataType)
            {
            case DataType::S8:
                reverseT<int8_t>(audio->getData(), out->getData(), sampleCount, info.channelCount);
                break;
            case DataType::S16:
                reverseT<int16_t>(audio->getData(), out->getData(), sampleCount, info.channelCount);
                break;
            case DataType::S32:
                reverseT<int32_t>(audio->getData(), out->getData(), sampleCount, info.channelCount);
                break;
            case DataType::F32:
                reverseT<float>(audio->getData(), out->getData(), sampleCount, info.channelCount);
                break;
            case DataType::F64:
                reverseT<double>(audio->getData(), out->getData(), sampleCount, info.channelCount);
                break;
            default: break;
            }
            return out;
        }

        namespace
        {
            template<typename T>
            void changeSpeedT(
                const uint8_t* in,
                uint8_t*       out,
                size_t         inSampleCount,
                size_t         outSampleCount,
                size_t         channelCount)
            {
                const T* inP = reinterpret_cast<const T*>(in);
                T* outP = reinterpret_cast<T*>(out);
                for (size_t i = 0; i < outSampleCount; ++i)
                {
                    const size_t j = i / static_cast<double>(outSampleCount - 1) *
                        (inSampleCount - 1);
                    for (size_t c = 0; c < channelCount; ++c)
                    {
                        outP[i * channelCount + c] = inP[j * channelCount + c];
                    }
                }
            }
        }

        std::shared_ptr<Audio> changeSpeed(const std::shared_ptr<Audio>& audio, double mult)
        {
            const Info& info = audio->getInfo();
            const size_t inSampleCount = audio->getSampleCount();
            const size_t outSampleCount = inSampleCount * mult;
            std::shared_ptr<Audio> out = Audio::create(info, outSampleCount);
            switch (info.dataType)
            {
            case DataType::S8:
                changeSpeedT<int8_t>(audio->getData(), out->getData(), inSampleCount, outSampleCount, info.channelCount);
                break;
            case DataType::S16:
                changeSpeedT<int16_t>(audio->getData(), out->getData(), inSampleCount, outSampleCount, info.channelCount);
                break;
            case DataType::S32:
                changeSpeedT<int32_t>(audio->getData(), out->getData(), inSampleCount, outSampleCount, info.channelCount);
                break;
            case DataType::F32:
                changeSpeedT<float>(audio->getData(), out->getData(), inSampleCount, outSampleCount, info.channelCount);
                break;
            case DataType::F64:
                changeSpeedT<double>(audio->getData(), out->getData(), inSampleCount, outSampleCount, info.channelCount);
                break;
            default: break;
            }
            return out;
        }

#define _CONVERT(a, b) \
    { \
        const a##_T * inP = reinterpret_cast<const a##_T *>(in->getData()); \
        b##_T * outP = reinterpret_cast<b##_T *>(out->getData()); \
        for (size_t i = 0; i < sampleCount * channelCount; ++i, ++inP, ++outP) \
        { \
            a##To##b(*inP, *outP); \
        } \
    }

        std::shared_ptr<Audio> convert(const std::shared_ptr<Audio>& in, DataType type)
        {
            const DataType inType = in->getDataType();
            const size_t sampleCount = in->getSampleCount();
            const size_t channelCount = in->getChannelCount();
            auto out = Audio::create(Info(channelCount, type, in->getSampleRate()), sampleCount);
            if (inType == type)
            {
                std::memcpy(
                    out->getData(),
                    in->getData(),
                    sampleCount * channelCount * getByteCount(type));
            }
            else
            {
                switch (inType)
                {
                case DataType::S8:
                    switch (type)
                    {
                    case DataType::S16: _CONVERT(S8, S16); break;
                    case DataType::S32: _CONVERT(S8, S32); break;
                    case DataType::F32: _CONVERT(S8, F32); break;
                    case DataType::F64: _CONVERT(S8, F64); break;
                    default: break;
                    }
                    break;
                case DataType::S16:
                    switch (type)
                    {
                    case DataType::S8:  _CONVERT(S16, S8);  break;
                    case DataType::S32: _CONVERT(S16, S32); break;
                    case DataType::F32: _CONVERT(S16, F32); break;
                    case DataType::F64: _CONVERT(S16, F64); break;
                    default: break;
                    }
                    break;
                case DataType::S32:
                    switch (type)
                    {
                    case DataType::S8:  _CONVERT(S32, S8);  break;
                    case DataType::S16: _CONVERT(S32, S16); break;
                    case DataType::F32: _CONVERT(S32, F32); break;
                    case DataType::F64: _CONVERT(S32, F64); break;
                    default: break;
                    }
                    break;
                case DataType::F32:
                    switch (type)
                    {
                    case DataType::S8:  _CONVERT(F32, S8);  break;
                    case DataType::S16: _CONVERT(F32, S16); break;
                    case DataType::S32: _CONVERT(F32, S32); break;
                    case DataType::F64: _CONVERT(F32, F64); break;
                    default: break;
                    }
                    break;
                case DataType::F64:
                    switch (type)
                    {
                    case DataType::S8:  _CONVERT(F64, S8);  break;
                    case DataType::S16: _CONVERT(F64, S16); break;
                    case DataType::S32: _CONVERT(F64, S32); break;
                    case DataType::F32: _CONVERT(F64, F32); break;
                    default: break;
                    }
                    break;
                default: break;
                }
            }
            return out;
        }

        size_t getSampleCount(const std::list<std::shared_ptr<audio::Audio> >& value)
        {
            size_t out = 0;
            for (const auto& i : value)
            {
                if (i)
                {
                    out += i->getSampleCount();
                }
            }
            return out;
        }

        void move(std::list<std::shared_ptr<Audio> >& in, uint8_t* out, size_t sampleCount)
        {
            size_t size = 0;
            while (!in.empty() && (size + in.front()->getSampleCount() <= sampleCount))
            {
                std::memcpy(out, in.front()->getData(), in.front()->getByteCount());
                size += in.front()->getSampleCount();
                out += in.front()->getByteCount();
                in.pop_front();
            }
            if (!in.empty() && size < sampleCount)
            {
                auto item = in.front();
                in.pop_front();
                const size_t remainingSize = sampleCount - size;
                std::memcpy(
                    out,
                    item->getData(),
                    remainingSize * item->getInfo().getByteCount());
                const size_t newItemSampleCount = item->getSampleCount() - remainingSize;
                auto newItem = audio::Audio::create(item->getInfo(), newItemSampleCount);
                std::memcpy(
                    newItem->getData(),
                    item->getData() + remainingSize * item->getInfo().getByteCount(),
                    newItem->getByteCount());
                in.push_front(newItem);
                size += remainingSize;
            }
        }
    }
}
