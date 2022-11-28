// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Audio.h>

namespace tl
{
    namespace audio
    {
        //! Convert audio data.
        class AudioConvert
        {
            TLRENDER_NON_COPYABLE(AudioConvert);

        protected:
            void _init(
                const audio::Info& input,
                const audio::Info& output);

            AudioConvert();

        public:
            ~AudioConvert();

            //! Create a new audio data converter.
            static std::shared_ptr<AudioConvert> create(
                const audio::Info& input,
                const audio::Info& ouput);

            //! Get input audio information.
            const audio::Info& getInputInfo() const;

            //! Get output audio information.
            const audio::Info& getOutputInfo() const;

            //! Convert audio data.
            std::vector<uint8_t> convert(const uint8_t*, size_t sampleCount);

            //! Flush the converter.
            void flush();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
