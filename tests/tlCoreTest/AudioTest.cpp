// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/AudioTest.h>

#include <tlCore/Assert.h>
#include <tlCore/AudioResample.h>
#include <tlCore/AudioSystem.h>

#include <cstring>

using namespace tl::audio;

namespace tl
{
    namespace core_tests
    {
        AudioTest::AudioTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::AudioTest", context)
        {}

        std::shared_ptr<AudioTest> AudioTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<AudioTest>(new AudioTest(context));
        }

        void AudioTest::run()
        {
            _enums();
            _types();
            _audio();
            _audioSystem();
            _combine();
            _mix();
            _reverse();
            _convert();
            _move();
            _resample();
        }

        void AudioTest::_enums()
        {
            _enum<DataType>("DataType", getDataTypeEnums);
        }

        void AudioTest::_types()
        {
            for (auto i : getDataTypeEnums())
            {
                std::stringstream ss;
                ss << i << " byte count: " << getByteCount(i);
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
                TLRENDER_ASSERT(info == info);
                TLRENDER_ASSERT(info != Info());
                auto audio = Audio::create(info, 1000);
                audio->zero();
                TLRENDER_ASSERT(audio->getInfo() == info);
                TLRENDER_ASSERT(audio->getChannelCount() == info.channelCount);
                TLRENDER_ASSERT(audio->getDataType() == info.dataType);
                TLRENDER_ASSERT(audio->getSampleRate() == info.sampleRate);
                TLRENDER_ASSERT(audio->getSampleCount() == 1000);
                TLRENDER_ASSERT(audio->isValid());
                TLRENDER_ASSERT(audio->getData());
                TLRENDER_ASSERT(static_cast<const Audio*>(audio.get())->getData());
            }
        }

        void AudioTest::_audioSystem()
        {
            auto system = _context->getSystem<System>();
            for (const auto& i : system->getDrivers())
            {
                std::stringstream ss;
                ss << "api: " << i;
                _print(ss.str());
            }
            for (const auto& i : system->getDevices())
            {
                std::stringstream ss;
                ss << "device: " << i.id.number << " " << i.id.name;
                _print(ss.str());
            }
            {
                std::stringstream ss;
                const DeviceInfo device = system->getDefaultDevice();
                ss << "default device: " << device.id.number << " " << device.id.name;
                _print(ss.str());
            }
        }

        void AudioTest::_combine()
        {
            std::list<std::shared_ptr<Audio> > list;
            auto audio = Audio::create(Info(1, DataType::S8, 41000), 1);
            audio->getData()[0] = 1;
            list.push_back(audio);
            audio = Audio::create(Info(1, DataType::S8, 41000), 1);
            audio->getData()[0] = 2;
            list.push_back(audio);
            audio = Audio::create(Info(1, DataType::S8, 41000), 1);
            audio->getData()[0] = 3;
            list.push_back(audio);
            auto combined = combine(list);
            TLRENDER_ASSERT(3 == combined->getSampleCount());
            TLRENDER_ASSERT(1 == combined->getData()[0]);
            TLRENDER_ASSERT(2 == combined->getData()[1]);
            TLRENDER_ASSERT(3 == combined->getData()[2]);
        }

        namespace
        {
            template<DataType DT, typename T>
            void _mixI()
            {
                const std::vector<T> in0 =
                {
                    0,
                    std::numeric_limits<T>::max(),
                    std::numeric_limits<T>::min(),
                    std::numeric_limits<T>::max(),
                    std::numeric_limits<T>::min()
                };
                const std::vector<T> in1 =
                {
                    0,
                    std::numeric_limits<T>::max(),
                    std::numeric_limits<T>::min(),
                    std::numeric_limits<T>::min(),
                    std::numeric_limits<T>::max()
                };
                const uint8_t* in[] =
                {
                    reinterpret_cast<const uint8_t*>(in0.data()),
                    reinterpret_cast<const uint8_t*>(in1.data())
                };
                const std::vector<T> out =
                {
                    0,
                    std::numeric_limits<T>::max(),
                    std::numeric_limits<T>::min(),
                    std::numeric_limits<T>::max() + std::numeric_limits<T>::min(),
                    std::numeric_limits<T>::max() + std::numeric_limits<T>::min()
                };
                std::vector<T> result(in0.size(), 0);
                mix(in,
                    2,
                    reinterpret_cast<uint8_t*>(result.data()),
                    1.F,
                    in0.size(),
                    1,
                    DT);
                TLRENDER_ASSERT(out == result);
            }

            template<DataType DT, typename T>
            void _mixF()
            {
                const std::vector<T> in0 =
                {
                    0,
                    1,
                    -1,
                    1,
                    -1
                };
                const std::vector<T> in1 =
                {
                    0,
                    1,
                    -1,
                    -1,
                    1
                };
                const uint8_t* in[] =
                {
                    reinterpret_cast<const uint8_t*>(in0.data()),
                    reinterpret_cast<const uint8_t*>(in1.data())
                };
                const std::vector<T> out =
                {
                    0,
                    2,
                    -2,
                    0,
                    0
                };
                std::vector<T> result(in0.size(), 0);
                mix(in,
                    2,
                    reinterpret_cast<uint8_t*>(result.data()),
                    1.F,
                    in0.size(),
                    1,
                    DT);
                for (size_t i = 0; i < in0.size(); ++i)
                {
                    TLRENDER_ASSERT(math::fuzzyCompare(out[i], result[i]));
                }
            }
        }

        void AudioTest::_mix()
        {
            _mixI<DataType::S8, int8_t>();
            _mixI<DataType::S16, int16_t>();
            _mixI<DataType::S32, int32_t>();
            _mixF<DataType::F32, float>();
            _mixF<DataType::F64, double>();
            {
                const std::vector<int8_t> in0 =
                {
                    0, 0,
                    1, 1,
                    2, 2,
                    3, 3,
                    4, 4
                };
                const std::vector<int8_t> in1 =
                {
                    0, 0,
                    1, 1,
                    2, 2,
                    3, 3,
                    4, 4
                };
                const uint8_t* in[] =
                {
                    reinterpret_cast<const uint8_t*>(in0.data()),
                    reinterpret_cast<const uint8_t*>(in1.data())
                };
                std::vector<int8_t> result(in0.size(), 0);
                mix(in,
                    2,
                    reinterpret_cast<uint8_t*>(result.data()),
                    1.F,
                    in0.size() / 2,
                    2,
                    DataType::S8);
                const std::vector<int8_t> out =
                {
                    0, 0,
                    2, 2,
                    4, 4,
                    6, 6,
                    8, 8
                };
                TLRENDER_ASSERT(out == result);
            }
        }

        void AudioTest::_reverse()
        {
            auto audio = Audio::create(Info(1, DataType::S8, 41000), 3);
            audio->getData()[0] = 1;
            audio->getData()[1] = 2;
            audio->getData()[2] = 3;
            auto reversed = reverse(audio);
            TLRENDER_ASSERT(3 == reversed->getData()[0]);
            TLRENDER_ASSERT(2 == reversed->getData()[1]);
            TLRENDER_ASSERT(1 == reversed->getData()[2]);
        }

        void AudioTest::_convert()
        {
            for (auto i : getDataTypeEnums())
            {
                const auto in = Audio::create(Info(1, i, 44100), 1);
                in->zero();
                for (auto j : getDataTypeEnums())
                {
                    const auto out = convert(in, j);
                    TLRENDER_ASSERT(out->getChannelCount() == in->getChannelCount());
                    TLRENDER_ASSERT(out->getDataType() == j);
                    TLRENDER_ASSERT(out->getSampleRate() == in->getSampleRate());
                    TLRENDER_ASSERT(out->getSampleCount() == in->getSampleCount());
                }
            }
        }

        void AudioTest::_move()
        {
            {
                const Info info(2, DataType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 10; ++i)
                {
                    auto item = Audio::create(info, 1);
                    reinterpret_cast<audio::S16_T*>(item->getData())[0] = i;
                    reinterpret_cast<audio::S16_T*>(item->getData())[1] = i;
                    list.push_back(item);
                }

                move(list, data.data(), 10);

                TLRENDER_ASSERT(list.empty());
                TLRENDER_ASSERT(0 == getSampleCount(list));
                audio::S16_T* p = reinterpret_cast<audio::S16_T*>(data.data());
                for (size_t i = 0; i < 10; ++i)
                {
                    TLRENDER_ASSERT(i == p[i * 2]);
                    TLRENDER_ASSERT(i == p[i * 2 + 1]);
                }
            }
            {
                const Info info(2, DataType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 5; ++i)
                {
                    auto item = Audio::create(info, 1);
                    reinterpret_cast<audio::S16_T*>(item->getData())[0] = i;
                    reinterpret_cast<audio::S16_T*>(item->getData())[1] = i;
                    list.push_back(item);
                }

                move(list, data.data(), 10);

                TLRENDER_ASSERT(list.empty());
                audio::S16_T* p = reinterpret_cast<audio::S16_T*>(data.data());
                size_t i = 0;
                for (; i < 5; ++i)
                {
                    TLRENDER_ASSERT(i == p[i * 2]);
                    TLRENDER_ASSERT(i == p[i * 2 + 1]);
                }
                for (; i < 10; ++i)
                {
                    TLRENDER_ASSERT(0 == p[i * 2]);
                    TLRENDER_ASSERT(0 == p[i * 2 + 1]);
                }
            }
            {
                const Info info(2, DataType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 15; ++i)
                {
                    auto item = Audio::create(info, 1);
                    reinterpret_cast<audio::S16_T*>(item->getData())[0] = i;
                    reinterpret_cast<audio::S16_T*>(item->getData())[1] = i;
                    list.push_back(item);
                }

                move(list, data.data(), 10);

                TLRENDER_ASSERT(5 == list.size());
                TLRENDER_ASSERT(5 == getSampleCount(list));
                audio::S16_T* p = reinterpret_cast<audio::S16_T*>(data.data());
                for (size_t i = 0; i < 10; ++i)
                {
                    TLRENDER_ASSERT(i == p[i * 2]);
                    TLRENDER_ASSERT(i == p[i * 2 + 1]);
                }
            }
            {
                const Info info(2, DataType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 4; ++i)
                {
                    auto item = Audio::create(info, 4);
                    for (size_t j = 0; j < 4; ++j)
                    {
                        reinterpret_cast<audio::S16_T*>(item->getData())[j * 2] = i * 4 + j;
                        reinterpret_cast<audio::S16_T*>(item->getData())[j * 2 + 1] = i * 4 + j;
                    }
                    list.push_back(item);
                }

                move(list, data.data(), 10);

                TLRENDER_ASSERT(2 == list.size());
                TLRENDER_ASSERT(6 == getSampleCount(list));
                TLRENDER_ASSERT(2 == list.front()->getSampleCount());
                TLRENDER_ASSERT(10 == reinterpret_cast<audio::S16_T*>(list.front()->getData())[0]);
                TLRENDER_ASSERT(11 == reinterpret_cast<audio::S16_T*>(list.front()->getData())[2]);
                audio::S16_T* p = reinterpret_cast<audio::S16_T*>(data.data());
                for (size_t i = 0; i < 10; ++i)
                {
                    TLRENDER_ASSERT(i == p[i * 2]);
                    TLRENDER_ASSERT(i == p[i * 2 + 1]);
                }
            }
        }

        void AudioTest::_resample()
        {
            for (auto dataType :
                {
                    DataType::S16,
                    DataType::S32,
                    DataType::F32,
                    DataType::F64,
                    DataType::None
                })
            {
                const Info a(2, dataType, 44100);
                const Info b(1, dataType, 44100);
                auto r = AudioResample::create(a, b);
                TLRENDER_ASSERT(a == r->getInputInfo());
                TLRENDER_ASSERT(b == r->getOutputInfo());
                auto in = Audio::create(a, 44100);
                auto out = r->process(in);
#if defined(TLRENDER_FFMPEG)
                if (dataType != DataType::None)
                {
                    TLRENDER_ASSERT(b == out->getInfo());
                    TLRENDER_ASSERT(44100 == out->getSampleCount());
                }
#endif // TLRENDER_FFMPEG
                r->flush();
            }
        }
    }
}
