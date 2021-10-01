// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace audio
    {
        inline bool Info::isValid() const
        {
            return dataType != DataType::None;
        }

        inline size_t Info::getByteCount() const
        {
            return channelCount * audio::getByteCount(dataType);
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

        inline uint8_t Audio::getChannelCount() const
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
