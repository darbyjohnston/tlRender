// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Audio.h>

#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <array>

namespace tlr
{
    namespace audio
    {
        TLR_ENUM_IMPL(
            DataType,
            "None",
            "S8",
            "S16",
            "S32",
            "F32",
            "F64");
        TLR_ENUM_SERIALIZE_IMPL(DataType);

        uint8_t getByteCount(DataType value) noexcept
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

        DataType getIntType(uint8_t value) noexcept
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

        DataType getFloatType(uint8_t value) noexcept
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

        RtAudioFormat toRtAudio(DataType value) noexcept
        {
            RtAudioFormat out = 0;
            switch (value)
            {
            case DataType::S16: out = RTAUDIO_SINT16; break;
            case DataType::S32: out = RTAUDIO_SINT32; break;
            case DataType::F32: out = RTAUDIO_FLOAT32; break;
            case DataType::F64: out = RTAUDIO_FLOAT64; break;
            default: break;
            }
            return out;
        }

        Info::Info()
        {}

        Info::Info(uint8_t channelCount, DataType dataType, size_t sampleRate) :
            channelCount(channelCount),
            dataType(dataType),
            sampleRate(sampleRate)
        {}

        std::ostream& operator << (std::ostream & os, const Info & value)
        {
            os << static_cast<size_t>(value.channelCount) << "," << value.dataType << "," << value.sampleRate;
            return os;
        }

        void Audio::_init(const Info& info, size_t sampleCount)
        {
            _info = info;
            _sampleCount = sampleCount;
            _data.resize(getByteCount());
        }

        Audio::Audio()
        {}

        std::shared_ptr<Audio> Audio::create(const Info& info, size_t sampleCount)
        {
            auto out = std::shared_ptr<Audio>(new Audio);
            out->_init(info, sampleCount);
            return out;
        }

        void Audio::zero()
        {
            std::memset(_data.data(), 0, getByteCount());
        }

#define _VOLUME(t) \
    { \
        const t##_T* inP = reinterpret_cast<const t##_T*>(in); \
        t##_T* outP = reinterpret_cast<t##_T*>(out); \
        t##_T* const endP = outP + sampleCount * channelCount; \
        for (; outP < endP; ++inP, ++outP) \
        { \
            *outP = *inP * volume; \
        } \
    }

        void volume(const uint8_t* in, uint8_t* out, float volume, size_t sampleCount, uint8_t channelCount, DataType type)
        {
            switch (type)
            {
            case DataType::S8:  _VOLUME(S8);  break;
            case DataType::S16: _VOLUME(S16); break;
            case DataType::S32: _VOLUME(S32); break;
            case DataType::F32: _VOLUME(F32); break;
            case DataType::F64: _VOLUME(F64); break;
            default: break;
            }
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
            const size_t channelCount = static_cast<size_t>(in->getChannelCount());
            auto out = Audio::create(Info(channelCount, type, in->getSampleRate()), sampleCount);
            if (inType == type)
            {
                std::memcpy(out->getData(), in->getData(), sampleCount * channelCount * getByteCount(type));
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

        namespace
        {
            template<typename U>
            void _planarInterleave(const U* value, U* out, uint8_t channelCount, size_t size)
            {
                const size_t planeSize = size;
                for (uint8_t c = 0; c < channelCount; ++c)
                {
                    const U* inP = value + c * planeSize;
                    const U* endP = inP + planeSize;
                    U* outP = out + c;
                    for (; inP < endP; ++inP, outP += channelCount)
                    {
                        *outP = *inP;
                    }
                }
            }
        }

        std::shared_ptr<Audio> planarInterleave(const std::shared_ptr<Audio>& value)
        {
            const size_t sampleCount = value->getSampleCount();
            const size_t channelCount = value->getChannelCount();
            auto out = Audio::create(value->getInfo(), sampleCount);
            switch (value->getDataType())
            {
            case DataType::S8:
                _planarInterleave(
                    reinterpret_cast<const S8_T*> (value->getData()),
                    reinterpret_cast<S8_T*> (out->getData()),
                    channelCount,
                    sampleCount);
                break;
            case DataType::S16:
                _planarInterleave(
                    reinterpret_cast<const S16_T*>(value->getData()),
                    reinterpret_cast<S16_T*>(out->getData()),
                    channelCount,
                    sampleCount);
                break;
            case DataType::S32:
                _planarInterleave(
                    reinterpret_cast<const S32_T*>(value->getData()),
                    reinterpret_cast<S32_T*>(out->getData()),
                    channelCount,
                    sampleCount);
                break;
            case DataType::F32:
                _planarInterleave(
                    reinterpret_cast<const F32_T*>(value->getData()),
                    reinterpret_cast<F32_T*>(out->getData()),
                    channelCount,
                    sampleCount);
                break;
            case DataType::F64:
                _planarInterleave(
                    reinterpret_cast<const F64_T*>(value->getData()),
                    reinterpret_cast<F64_T*>(out->getData()),
                    channelCount,
                    sampleCount);
                break;
            default: break;
            }
            return out;
        }

        namespace
        {
            template<typename T>
            void _planarDeinterleave(const T* value, T* out, uint8_t channelCount, size_t size)
            {
                const size_t planeSize = size;
                for (uint8_t c = 0; c < channelCount; ++c)
                {
                    const T* inP = value + c;
                    T* outP = out + c * planeSize;
                    for (size_t i = 0; i < planeSize; ++i, inP += channelCount, ++outP)
                    {
                        *outP = *inP;
                    }
                }
            }
        }

        std::shared_ptr<Audio> planarDeinterleave(const std::shared_ptr<Audio>& value)
        {
            const uint8_t channelCount = value->getChannelCount();
            const size_t sampleCount = value->getSampleCount();
            auto out = Audio::create(value->getInfo(), sampleCount);
            switch (value->getDataType())
            {
            case DataType::S8:
                _planarDeinterleave(
                    reinterpret_cast<const S8_T*> (value->getData()),
                    reinterpret_cast<S8_T*>(out->getData()),
                    channelCount,
                    sampleCount);
                break;
            case DataType::S16:
                _planarDeinterleave(
                    reinterpret_cast<const S16_T*>(value->getData()),
                    reinterpret_cast<S16_T*>(out->getData()),
                    channelCount,
                    sampleCount);
                break;
            case DataType::S32:
                _planarDeinterleave(
                    reinterpret_cast<const S32_T*>(value->getData()),
                    reinterpret_cast<S32_T*>(out->getData()),
                    channelCount,
                    sampleCount);
                break;
            case DataType::F32:
                _planarDeinterleave(
                    reinterpret_cast<const F32_T*>(value->getData()),
                    reinterpret_cast<F32_T*>(out->getData()),
                    channelCount,
                    sampleCount);
                break;
            case DataType::F64:
                _planarDeinterleave(
                    reinterpret_cast<const F64_T*>(value->getData()),
                    reinterpret_cast<F64_T*>(out->getData()),
                    channelCount,
                    sampleCount);
                break;
            default: break;
            }
            return out;
        }

        void copy(std::list<std::shared_ptr<Audio> >& in, uint8_t* out, size_t byteCount)
        {
            size_t size = 0;
            while (!in.empty() && (size + in.front()->getByteCount() <= byteCount))
            {
                const size_t itemByteCount = in.front()->getByteCount();
                std::memcpy(out, in.front()->getData(), itemByteCount);
                size += itemByteCount;
                out += itemByteCount;
                in.pop_front();
            }
            if (!in.empty() && size < byteCount)
            {
                auto item = in.front();
                in.pop_front();
                std::memcpy(out, item->getData(), byteCount - size);
                const size_t newItemByteCount = item->getByteCount() - (byteCount - size);
                auto newItem = audio::Audio::create(item->getInfo(), newItemByteCount / item->getInfo().getByteCount());
                std::memcpy(newItem->getData(), item->getData() + byteCount - size, newItemByteCount);
                in.push_front(newItem);
            }
        }
    }
}