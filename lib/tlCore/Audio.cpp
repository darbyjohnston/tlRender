// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/Audio.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <array>

namespace tl
{
    namespace audio
    {

        namespace
        {
            template< typename T >
            inline void reverseAudio(uint8_t* inData,
                                     uint8_t channels,
                                     size_t sampleCount)
            {
                T* data = reinterpret_cast<T*>(inData);
                const size_t halfNumSamples = sampleCount/2;
                        
                for (size_t i = 0; i < halfNumSamples; ++i)
                {
                    T* out0 = data + i * channels;
                    T* out1 = data + (sampleCount - 1 - i) * channels;

                    for (size_t j = 0; j < channels; ++j)
                    {
                        T tmp = out0[j];
                        out0[j] = out1[j];
                        out1[j] = tmp;
                    }
                }
            }
        }
        
        TLRENDER_ENUM_IMPL(
            DataType,
            "None",
            "S8",
            "S16",
            "S32",
            "F32",
            "F64");
        TLRENDER_ENUM_SERIALIZE_IMPL(DataType);

        size_t getByteCount(DataType value) noexcept
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

        Info::Info()
        {}

        Info::Info(uint8_t channelCount, DataType dataType, size_t sampleRate) :
            channelCount(channelCount),
            dataType(dataType),
            sampleRate(sampleRate)
        {}

        void Audio::_init(const Info& info, size_t sampleCount)
        {
            _info = info;
            _sampleCount = sampleCount;
            const size_t byteCount = getByteCount();
            _data = new uint8_t[byteCount];
        }

        Audio::Audio()
        {}

        Audio::~Audio()
        {
            delete[] _data;
        }

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
            std::memset(_data, 0, getByteCount());
        }

        namespace
        {
            template<typename T, typename TI>
            void mixI(
                const uint8_t** in,
                size_t inCount,
                uint8_t* out,
                float volume,
                size_t size)
            {
                const T** const inP = reinterpret_cast<const T**>(in);
                T* const outP = reinterpret_cast<T*>(out);
                for (size_t i = 0; i < size; ++i)
                {
                    const TI min = static_cast<TI>(std::numeric_limits<T>::min());
                    const TI max = static_cast<TI>(std::numeric_limits<T>::max());
                    TI v = 0;
                    for (size_t j = 0; j < inCount; ++j)
                    {
                        v += math::clamp(static_cast<TI>(inP[j][i] * volume), min, max);
                    }
                    outP[i] = math::clamp(v, min, max);
                }
            }

            template<typename T>
            void mixF(
                const uint8_t** in,
                size_t inCount,
                uint8_t* out,
                float volume,
                size_t size)
            {
                const T** const inP = reinterpret_cast<const T**>(in);
                T* const outP = reinterpret_cast<T*>(out);
                for (size_t i = 0; i < size; ++i)
                {
                    T v = static_cast<T>(0);
                    for (size_t j = 0; j < inCount; ++j)
                    {
                        v += inP[j][i] * volume;
                    }
                    outP[i] = v;
                }
            }
        }

        void reverse(
            uint8_t** inOut,
            size_t inCount,
            size_t sampleCount,
            uint8_t channelCount,
            DataType type)
        {
            for( size_t i = 0; i < inCount; ++i )
            {
                switch (type)
                {
                case DataType::S8:
                {
                    reverseAudio<S8_T>(inOut[i],
                                       channelCount,
                                       sampleCount);
                    break;
                }
                case DataType::S16:
                {
                    reverseAudio<S16_T>(inOut[i],
                                        channelCount,
                                        sampleCount);
                    break;
                }
                case DataType::S32:
                {
                    reverseAudio<S32_T>(inOut[i],
                                        channelCount,
                                        sampleCount);
                    break;
                }
                case DataType::F32:
                {
                    reverseAudio<F32_T>(inOut[i],
                                        channelCount,
                                        sampleCount);
                    break;
                }
                case DataType::F64:
                {
                    reverseAudio<F64_T>(inOut[i],
                                        channelCount,
                                        sampleCount);
                    break;
                }
                default: break;
                }
            }
        }
        
        void mix(
            const uint8_t** in,
            size_t inCount,
            uint8_t* out,
            float volume,
            size_t sampleCount,
            uint8_t channelCount,
            DataType type)
        {
            const size_t size = sampleCount * static_cast<size_t>(channelCount);
            switch (type)
            {
            case DataType::S8:
                mixI<int8_t, int16_t>(in, inCount, out, volume, size);
                break;
            case DataType::S16:
                mixI<int16_t, int32_t>(in, inCount, out, volume, size);
                break;
            case DataType::S32:
                mixI<int32_t, int64_t>(in, inCount, out, volume, size);
                break;
            case DataType::F32:
                mixF<float>(in, inCount, out, volume, size);
                break;
            case DataType::F64:
                mixF<double>(in, inCount, out, volume, size);
                break;
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

        namespace
        {
            template<typename T>
            void _planarInterleave(const std::shared_ptr<Audio>& value, const std::shared_ptr<Audio>& out)
            {
                const uint8_t channelCount = value->getChannelCount();
                const size_t sampleCount = value->getSampleCount();
                std::vector<const T*> planes;
                for (uint8_t i = 0; i < channelCount; ++i)
                {
                    planes.push_back(reinterpret_cast<const T*>(value->getData()) + i * sampleCount);
                }
                planarInterleave(
                    planes.data(),
                    reinterpret_cast<T*>(out->getData()),
                    channelCount,
                    sampleCount);
            }
        }

        std::shared_ptr<Audio> planarInterleave(const std::shared_ptr<Audio>& value)
        {
            auto out = Audio::create(value->getInfo(), value->getSampleCount());
            switch (value->getDataType())
            {
            case DataType::S8: _planarInterleave<int8_t>(value, out); break;
            case DataType::S16: _planarInterleave<int16_t>(value, out); break;
            case DataType::S32: _planarInterleave<int32_t>(value, out); break;
            case DataType::F32: _planarInterleave<float>(value, out); break;
            case DataType::F64: _planarInterleave<double>(value, out); break;
            default: break;
            }
            return out;
        }

        namespace
        {
            template<typename T>
            void _planarDeinterleave(const T* value, T* out, size_t channelCount, size_t size)
            {
                const size_t planeSize = size;
                for (size_t c = 0; c < channelCount; ++c)
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

        size_t getSampleCount(const std::list<std::shared_ptr<audio::Audio> >& value)
        {
            size_t out = 0;
            for (const auto& i : value)
            {
                out += i->getSampleCount();
            }
            return out;
        }

        void copy(std::list<std::shared_ptr<Audio> >& in, uint8_t* out, size_t byteCount)
        {
            size_t size = 0;
            while (!in.empty() && (size + in.front()->getByteCount() <= byteCount))
            {
                auto item = in.front();
                const size_t itemByteCount = item->getByteCount();
                std::memcpy(out, item->getData(), itemByteCount);
                size += itemByteCount;
                out += itemByteCount;
                in.pop_front();
            }
            if (!in.empty() && size < byteCount)
            {
                auto item = in.front();
                in.pop_front();
                const size_t itemByteCount = byteCount - size;
                const size_t itemDataSize  = item->getInfo().getByteCount();
                std::memcpy(out, item->getData(), itemByteCount);
                const size_t newItemByteCount = item->getByteCount() - itemByteCount;
                auto newItem = audio::Audio::create(item->getInfo(), newItemByteCount / itemDataSize);
                std::memcpy(newItem->getData(), item->getData() + itemByteCount, newItemByteCount);
                in.push_front(newItem);
            }
        }
    }
}
