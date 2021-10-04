// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Range.h>
#include <tlrCore/Util.h>

#include <rtaudio/RtAudio.h>

#include <string>
#include <vector>

namespace tlr
{
    //! Audio functionality.
    namespace audio
    {
        //! \name Pixel Types
        ///@{

        //! Audio data types.
        enum class DataType
        {
            None,
            S8,
            S16,
            S32,
            F32,
            F64,

            Count,
            First = None
        };
        TLR_ENUM(DataType);
        TLR_ENUM_SERIALIZE(DataType);

        typedef int8_t   S8_T;
        typedef int16_t S16_T;
        typedef int32_t S32_T;
        typedef float   F32_T;
        typedef double  F64_T;

        const math::Range<S8_T> S8Range(
            std::numeric_limits<S8_T>::min(),
            std::numeric_limits<S8_T>::max());

        const math::Range<S16_T> S16Range(
            std::numeric_limits<S16_T>::min(),
            std::numeric_limits<S16_T>::max());

        const math::Range<S32_T> S32Range(
            std::numeric_limits<S32_T>::min(),
            std::numeric_limits<S32_T>::max());

        const math::Range<F32_T> F32Range(-1.F, 1.F);
        const math::Range<F64_T> F64Range(-1.F, 1.F);

        //! Get the byte count for the given data type.
        uint8_t getByteCount(DataType) noexcept;

        //! Determine the integer data type for a given byte count.
        DataType getIntType(uint8_t) noexcept;

        //! Determine the floating point data type for a given byte count.
        DataType getFloatType(uint8_t) noexcept;

        //! Convert a data type to a RtAudio type.
        RtAudioFormat toRtAudio(DataType) noexcept;

        ///@}

        //! Audio information.
        class Info
        {
        public:
            Info();
            Info(
                uint8_t  channelCount,
                DataType dataType,
                size_t   sampleRate);

            std::string name         = "Default";
            uint8_t     channelCount = 0;
            DataType    dataType     = DataType::None;
            size_t      sampleRate   = 0;

            bool isValid() const;
            size_t getByteCount() const;

            bool operator == (const Info&) const;
            bool operator != (const Info&) const;
        };

        std::ostream& operator << (std::ostream&, const Info&);

        //! Audio.
        class Audio
        {
            TLR_NON_COPYABLE(Audio);

        protected:
            void _init(const Info&, size_t sampleCount);
            Audio();

        public:
            //! Create new audio.
            static std::shared_ptr<Audio> create(const Info&, size_t sampleCount);

            //! Get the audio information.
            const Info& getInfo() const;

            //! Get the audio channel count.
            uint8_t getChannelCount() const;

            //! Get the audio data type.
            DataType getDataType() const;

            //! Get the audio sample rate.
            size_t getSampleRate() const;

            //! Get the audio sample count.
            size_t getSampleCount() const;

            //! Is the audio valid?
            bool isValid() const;

            //! Get the audio data byte count.
            size_t getByteCount() const;

            //! Get the audio data.
            uint8_t* getData();

            //! Get the audio data.
            const uint8_t* getData() const;

            //! Zero the audio data.
            void zero();

        private:
            Info _info;
            size_t _sampleCount = 0;
            std::vector<uint8_t> _data;
        };
    }
}

#include <tlrCore/AudioInline.h>
