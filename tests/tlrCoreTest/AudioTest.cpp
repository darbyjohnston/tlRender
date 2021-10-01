// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/AudioTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/AudioSystem.h>

using namespace tlr::audio;

namespace tlr
{
    namespace CoreTest
    {
        AudioTest::AudioTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::AudioTest", context)
        {}

        std::shared_ptr<AudioTest> AudioTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<AudioTest>(new AudioTest(context));
        }

        void AudioTest::run()
        {
            _enums();
            _util();
            _audio();
            _audioSystem();
        }

        void AudioTest::_enums()
        {
            _enum<DataType>("DataType", getDataTypeEnums);
            _enum<DeviceFormat>("DeviceFormat", getDeviceFormatEnums);
        }

        void AudioTest::_util()
        {
            for (auto i : getDataTypeEnums())
            {
                std::stringstream ss;
                ss << i << " byte count: " << static_cast<int>(getByteCount(i));
                _print(ss.str());
            }
            for (auto i : { 0, 1, 2, 3, 4, 5, 6, 7, 8 })
            {
                std::stringstream ss;
                ss << i << " bytes int type: " << getIntType(i);
                _print(ss.str());
            }
            for (auto i : { 0, 1, 2, 3, 4, 5, 6, 7, 8 })
            {
                std::stringstream ss;
                ss << i << " bytes float type: " << getFloatType(i);
                _print(ss.str());
            }
        }

        void AudioTest::_audio()
        {
            {
                const Info info(2, DataType::S16, 44100);
                auto audio = Audio::create(info, 1000);
                audio->zero();
                TLR_ASSERT(audio->getInfo() == info);
                TLR_ASSERT(audio->getChannelCount() == info.channelCount);
                TLR_ASSERT(audio->getDataType() == info.dataType);
                TLR_ASSERT(audio->getSampleRate() == info.sampleRate);
                TLR_ASSERT(audio->getSampleCount() == 1000);
                TLR_ASSERT(audio->isValid());
                TLR_ASSERT(audio->getData());
                TLR_ASSERT(static_cast<const Audio*>(audio.get())->getData());
            }
        }

        void AudioTest::_audioSystem()
        {
            auto system = _context->getSystem<System>();
            for (const auto& i : system->getAPIs())
            {
                std::stringstream ss;
                ss << "api: " << i;
                _print(ss.str());
            }
            for (const auto& i : system->getDevices())
            {
                std::stringstream ss;
                ss << "device: " << i.name;
                _print(ss.str());
            }
            {
                std::stringstream ss;
                ss << "default input device: " << system->getDefaultInputDevice();
                _print(ss.str());
            }
            {
                std::stringstream ss;
                ss << "default output device: " << system->getDefaultOutputDevice();
                _print(ss.str());
            }
        }
    }
}
