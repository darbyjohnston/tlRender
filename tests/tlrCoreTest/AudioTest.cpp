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
            _types();
            _audio();
            _audioSystem();
            _util();
        }

        void AudioTest::_enums()
        {
            _enum<DataType>("DataType", getDataTypeEnums);
            _enum<DeviceFormat>("DeviceFormat", getDeviceFormatEnums);
        }

        void AudioTest::_types()
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

        void AudioTest::_util()
        {
            {
                const audio::Info info(1, audio::DataType::S8, 10);

                uint8_t data[10];
                memset(data, 0, 10);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 10; ++i)
                {
                    auto item = audio::Audio::create(info, 1);
                    item->getData()[0] = i;
                    list.push_back(item);
                }

                audio::copy(list, data, 10);

                TLR_ASSERT(list.empty());
                for (size_t i = 0; i < 10; ++i)
                {
                    TLR_ASSERT(i == data[i]);
                }
            }
            {
                const audio::Info info(1, audio::DataType::S8, 10);

                uint8_t data[10];
                memset(data, 0, 10);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 5; ++i)
                {
                    auto item = audio::Audio::create(info, 1);
                    item->getData()[0] = i;
                    list.push_back(item);
                }

                audio::copy(list, data, 10);

                TLR_ASSERT(list.empty());
                size_t i = 0;
                for (; i < 5; ++i)
                {
                    TLR_ASSERT(i == data[i]);
                }
                for (; i < 10; ++i)
                {
                    TLR_ASSERT(0 == data[i]);
                }
            }
            {
                const audio::Info info(1, audio::DataType::S8, 10);

                uint8_t data[10];
                memset(data, 0, 10);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 15; ++i)
                {
                    auto item = audio::Audio::create(info, 1);
                    item->getData()[0] = i;
                    list.push_back(item);
                }

                audio::copy(list, data, 10);

                TLR_ASSERT(5 == list.size());
                for (size_t i = 0; i < 10; ++i)
                {
                    TLR_ASSERT(i == data[i]);
                }
            }
            {
                const audio::Info info(1, audio::DataType::S8, 10);

                auto data = audio::Audio::create(info, 10);
                uint8_t* dataP = data->getData();
                memset(dataP, 0, 10);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 4; ++i)
                {
                    auto item = audio::Audio::create(info, 4);
                    for (size_t j = 0; j < 4; ++j)
                    {
                        item->getData()[j] = i * 4 + j;
                    }
                    list.push_back(item);
                }

                audio::copy(list, data);

                TLR_ASSERT(2 == list.size());
                TLR_ASSERT(2 == list.front()->getByteCount());
                TLR_ASSERT(10 == list.front()->getData()[0]);
                TLR_ASSERT(11 == list.front()->getData()[1]);
                for (size_t i = 0; i < 10; ++i)
                {
                    TLR_ASSERT(i == dataP[i]);
                }
            }
        }
    }
}
