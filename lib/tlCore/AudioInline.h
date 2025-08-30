// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <feather-tk/core/Math.h>

#include <cstring>

namespace tl
{
    namespace audio
    {
        inline bool Info::isValid() const
        {
            return
                channelCount > 0 &&
                dataType != DataType::None &&
                sampleRate > 0;
        }

        inline size_t Info::getByteCount() const
        {
            return static_cast<size_t>(channelCount) * audio::getByteCount(dataType);
        }

        inline void S8ToS16(S8_T value, S16_T& out)
        {
            out = value * 256;
        }

        inline void S8ToS32(S8_T value, S32_T& out)
        {
            out = value * 256 * 256 * 256;
        }

        inline void S8ToF32(S8_T value, F32_T& out)
        {
            out = value / static_cast<float>(S8Range.max());
        }

        inline void S8ToF64(S8_T value, F64_T& out)
        {
            out = value / static_cast<double>(S8Range.max());
        }

        inline void S16ToS8(S16_T value, S8_T& out)
        {
            out = value / 256;
        }

        inline void S16ToS32(S16_T value, S32_T& out)
        {
            out = value * 256 * 256;
        }

        inline void S16ToF32(S16_T value, F32_T& out)
        {
            out = value / static_cast<float>(S16Range.max());
        }

        inline void S16ToF64(S16_T value, F64_T& out)
        {
            out = value / static_cast<double>(S16Range.max());
        }

        inline void S32ToS8(S32_T value, S8_T& out)
        {
            out = value / 256 / 256 / 256;
        }

        inline void S32ToS16(S32_T value, S16_T& out)
        {
            out = value / 256 / 256;
        }

        inline void S32ToF32(S32_T value, F32_T& out)
        {
            out = value / static_cast<float>(S32Range.max());
        }

        inline void S32ToF64(S32_T value, F64_T& out)
        {
            out = value / static_cast<double>(S32Range.max());
        }

        inline void F32ToS8(F32_T value, S8_T& out)
        {
            out = static_cast<S8_T>(ftk::clamp(
                static_cast<int16_t>(value * S8Range.max()),
                static_cast<int16_t>(S8Range.min()),
                static_cast<int16_t>(S8Range.max())));
        }

        inline void F32ToS16(F32_T value, S16_T& out)
        {
            out = static_cast<S16_T>(ftk::clamp(
                static_cast<int32_t>(value * S16Range.max()),
                static_cast<int32_t>(S16Range.min()),
                static_cast<int32_t>(S16Range.max())));
        }

        inline void F32ToS32(F32_T value, S32_T& out)
        {
            out = static_cast<S32_T>(ftk::clamp(
                static_cast<int64_t>(static_cast<int64_t>(value) * S32Range.max()),
                static_cast<int64_t>(S32Range.min()),
                static_cast<int64_t>(S32Range.max())));
        }

        inline void F32ToF64(F32_T value, F64_T& out)
        {
            out = static_cast<double>(value);
        }

        inline void F64ToS8(F64_T value, S8_T& out)
        {
            out = static_cast<S8_T>(ftk::clamp(
                static_cast<int16_t>(value * S8Range.max()),
                static_cast<int16_t>(S8Range.min()),
                static_cast<int16_t>(S8Range.max())));
        }

        inline void F64ToS16(F64_T value, S16_T& out)
        {
            out = static_cast<S16_T>(ftk::clamp(
                static_cast<int32_t>(value * S16Range.max()),
                static_cast<int32_t>(S16Range.min()),
                static_cast<int32_t>(S16Range.max())));
        }

        inline void F64ToS32(F64_T value, S32_T& out)
        {
            out = static_cast<S32_T>(ftk::clamp(
                static_cast<int64_t>(static_cast<int64_t>(value) * S32Range.max()),
                static_cast<int64_t>(S32Range.min()),
                static_cast<int64_t>(S32Range.max())));
        }

        inline void F64ToF32(F64_T value, F32_T& out)
        {
            out = static_cast<float>(value);
        }

        inline bool Info::operator == (const Info& other) const
        {
            return
                name == other.name &&
                channelCount == other.channelCount &&
                dataType == other.dataType &&
                sampleRate == other.sampleRate;
        }

        inline bool Info::operator != (const Info& other) const
        {
            return !(*this == other);
        }

        inline const Info& Audio::getInfo() const
        {
            return _info;
        }

        inline size_t Audio::getChannelCount() const
        {
            return _info.channelCount;
        }

        inline DataType Audio::getDataType() const
        {
            return _info.dataType;
        }

        inline size_t Audio::getSampleRate() const
        {
            return _info.sampleRate;
        }

        inline size_t Audio::getSampleCount() const
        {
            return _sampleCount;
        }

        inline bool Audio::isValid() const
        {
            return _info.isValid();
        }

        inline size_t Audio::getByteCount() const
        {
            return _info.getByteCount() * _sampleCount;
        }

        inline uint8_t* Audio::getData()
        {
            return _data.data();
        }

        inline const uint8_t* Audio::getData() const
        {
            return _data.data();
        }
    }
}
