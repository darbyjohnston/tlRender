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
            memset(_data.data(), 0, getByteCount());
        }
    }
}