// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/AudioTest.h>

#include <tlCore/Assert.h>
#include <tlCore/AudioSystem.h>

#include <cstring>

using namespace tl::core;
using namespace tl::core::audio;

namespace tl
{
    namespace tests
    {
        namespace core_test
        {
            AudioTest::AudioTest(const std::shared_ptr<system::Context>& context) :
                ITest("core_test::AudioTest", context)
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
                _mix();
                _convert();
                _interleave();
                _copy();
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
                {
                    TLRENDER_ASSERT(RTAUDIO_SINT16 == toRtAudio(DataType::S16));
                    TLRENDER_ASSERT(RTAUDIO_SINT32 == toRtAudio(DataType::S32));
                    TLRENDER_ASSERT(RTAUDIO_FLOAT32 == toRtAudio(DataType::F32));
                    TLRENDER_ASSERT(RTAUDIO_FLOAT64 == toRtAudio(DataType::F64));
                }
            }

            void AudioTest::_audio()
            {
                {
                    const Info info(2, DataType::S16, 44100);
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
                    const uint8_t* in[2] =
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
                    for (size_t i = 0; i < in0.size(); ++i)
                    {
                        TLRENDER_ASSERT(out[i] == result[i]);
                    }
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
                    const uint8_t* in[2] =
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
            }

            void AudioTest::_convert()
            {
                for (auto i : getDataTypeEnums())
                {
                    const auto in = Audio::create(Info(1, i, 44100), 1);
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

            namespace
            {
                template<DataType DT, typename T>
                void _interleaveT()
                {
                    {
                        auto in = Audio::create(Info(1, DT, 44100), 2);
                        T* inP = reinterpret_cast<T*>(in->getData());
                        inP[0] = 0;
                        inP[1] = 1;
                        const auto out0 = planarInterleave(in);
                        const T* out0P = reinterpret_cast<const T*>(out0->getData());
                        TLRENDER_ASSERT(0 == out0P[0]);
                        TLRENDER_ASSERT(1 == out0P[1]);
                        const auto out1 = planarDeinterleave(out0);
                        const T* out1P = reinterpret_cast<const T*>(out1->getData());
                        TLRENDER_ASSERT(0 == out1P[0]);
                        TLRENDER_ASSERT(1 == out1P[1]);
                    }
                    {
                        auto in = Audio::create(Info(2, DT, 44100), 2);
                        T* inP = reinterpret_cast<T*>(in->getData());
                        inP[0] = 0;
                        inP[1] = 1;
                        inP[2] = 2;
                        inP[3] = 3;
                        const auto out0 = planarInterleave(in);
                        const T* out0P = reinterpret_cast<const T*>(out0->getData());
                        TLRENDER_ASSERT(0 == out0P[0]);
                        TLRENDER_ASSERT(2 == out0P[1]);
                        TLRENDER_ASSERT(1 == out0P[2]);
                        TLRENDER_ASSERT(3 == out0P[3]);
                        const auto out1 = planarDeinterleave(out0);
                        const T* out1P = reinterpret_cast<const T*>(out1->getData());
                        TLRENDER_ASSERT(0 == out1P[0]);
                        TLRENDER_ASSERT(1 == out1P[1]);
                        TLRENDER_ASSERT(2 == out1P[2]);
                        TLRENDER_ASSERT(3 == out1P[3]);
                    }
                    {
                        auto in = Audio::create(Info(3, DT, 44100), 2);
                        T* inP = reinterpret_cast<T*>(in->getData());
                        inP[0] = 0;
                        inP[1] = 1;
                        inP[2] = 2;
                        inP[3] = 3;
                        inP[4] = 4;
                        inP[5] = 5;
                        const auto out0 = planarInterleave(in);
                        const T* out0P = reinterpret_cast<const T*>(out0->getData());
                        TLRENDER_ASSERT(0 == out0P[0]);
                        TLRENDER_ASSERT(2 == out0P[1]);
                        TLRENDER_ASSERT(4 == out0P[2]);
                        TLRENDER_ASSERT(1 == out0P[3]);
                        TLRENDER_ASSERT(3 == out0P[4]);
                        TLRENDER_ASSERT(5 == out0P[5]);
                        const auto out1 = planarDeinterleave(out0);
                        const T* out1P = reinterpret_cast<const T*>(out1->getData());
                        TLRENDER_ASSERT(0 == out1P[0]);
                        TLRENDER_ASSERT(1 == out1P[1]);
                        TLRENDER_ASSERT(2 == out1P[2]);
                        TLRENDER_ASSERT(3 == out1P[3]);
                        TLRENDER_ASSERT(4 == out1P[4]);
                        TLRENDER_ASSERT(5 == out1P[5]);
                    }
                    {
                        auto in = Audio::create(Info(6, DT, 44100), 2);
                        T* inP = reinterpret_cast<T*>(in->getData());
                        inP[0] = 0;
                        inP[1] = 1;
                        inP[2] = 2;
                        inP[3] = 3;
                        inP[4] = 4;
                        inP[5] = 5;
                        inP[6] = 6;
                        inP[7] = 7;
                        inP[8] = 8;
                        inP[9] = 9;
                        inP[10] = 10;
                        inP[11] = 11;
                        const auto out0 = planarInterleave(in);
                        const T* out0P = reinterpret_cast<const T*>(out0->getData());
                        TLRENDER_ASSERT(0 == out0P[0]);
                        TLRENDER_ASSERT(2 == out0P[1]);
                        TLRENDER_ASSERT(4 == out0P[2]);
                        TLRENDER_ASSERT(6 == out0P[3]);
                        TLRENDER_ASSERT(8 == out0P[4]);
                        TLRENDER_ASSERT(10 == out0P[5]);
                        TLRENDER_ASSERT(1 == out0P[6]);
                        TLRENDER_ASSERT(3 == out0P[7]);
                        TLRENDER_ASSERT(5 == out0P[8]);
                        TLRENDER_ASSERT(7 == out0P[9]);
                        TLRENDER_ASSERT(9 == out0P[10]);
                        TLRENDER_ASSERT(11 == out0P[11]);
                        const auto out1 = planarDeinterleave(out0);
                        const T* out1P = reinterpret_cast<const T*>(out1->getData());
                        TLRENDER_ASSERT(0 == out1P[0]);
                        TLRENDER_ASSERT(1 == out1P[1]);
                        TLRENDER_ASSERT(2 == out1P[2]);
                        TLRENDER_ASSERT(3 == out1P[3]);
                        TLRENDER_ASSERT(4 == out1P[4]);
                        TLRENDER_ASSERT(5 == out1P[5]);
                        TLRENDER_ASSERT(6 == out1P[6]);
                        TLRENDER_ASSERT(7 == out1P[7]);
                        TLRENDER_ASSERT(8 == out1P[8]);
                        TLRENDER_ASSERT(9 == out1P[9]);
                        TLRENDER_ASSERT(10 == out1P[10]);
                        TLRENDER_ASSERT(11 == out1P[11]);
                    }
                }
            }

            void AudioTest::_interleave()
            {
                _interleaveT<DataType::S8, int8_t>();
                _interleaveT<DataType::S16, int16_t>();
                _interleaveT<DataType::S32, int32_t>();
                _interleaveT<DataType::F32, float>();
                _interleaveT<DataType::F64, double>();
            }

            void AudioTest::_copy()
            {
                {
                    const Info info(1, DataType::S8, 10);

                    uint8_t data[10];
                    std::memset(data, 0, 10);

                    std::list<std::shared_ptr<Audio> > list;
                    for (size_t i = 0; i < 10; ++i)
                    {
                        auto item = Audio::create(info, 1);
                        item->getData()[0] = i;
                        list.push_back(item);
                    }

                    copy(list, data, 10);

                    TLRENDER_ASSERT(list.empty());
                    for (size_t i = 0; i < 10; ++i)
                    {
                        TLRENDER_ASSERT(i == data[i]);
                    }
                }
                {
                    const Info info(1, DataType::S8, 10);

                    uint8_t data[10];
                    std::memset(data, 0, 10);

                    std::list<std::shared_ptr<Audio> > list;
                    for (size_t i = 0; i < 5; ++i)
                    {
                        auto item = Audio::create(info, 1);
                        item->getData()[0] = i;
                        list.push_back(item);
                    }

                    copy(list, data, 10);

                    TLRENDER_ASSERT(list.empty());
                    size_t i = 0;
                    for (; i < 5; ++i)
                    {
                        TLRENDER_ASSERT(i == data[i]);
                    }
                    for (; i < 10; ++i)
                    {
                        TLRENDER_ASSERT(0 == data[i]);
                    }
                }
                {
                    const Info info(1, DataType::S8, 10);

                    uint8_t data[10];
                    std::memset(data, 0, 10);

                    std::list<std::shared_ptr<Audio> > list;
                    for (size_t i = 0; i < 15; ++i)
                    {
                        auto item = Audio::create(info, 1);
                        item->getData()[0] = i;
                        list.push_back(item);
                    }

                    copy(list, data, 10);

                    TLRENDER_ASSERT(5 == list.size());
                    for (size_t i = 0; i < 10; ++i)
                    {
                        TLRENDER_ASSERT(i == data[i]);
                    }
                }
                {
                    const Info info(1, DataType::S8, 10);

                    auto data = Audio::create(info, 10);
                    uint8_t* dataP = data->getData();
                    std::memset(dataP, 0, 10);

                    std::list<std::shared_ptr<Audio> > list;
                    for (size_t i = 0; i < 4; ++i)
                    {
                        auto item = Audio::create(info, 4);
                        for (size_t j = 0; j < 4; ++j)
                        {
                            item->getData()[j] = i * 4 + j;
                        }
                        list.push_back(item);
                    }

                    copy(list, dataP, 10);

                    TLRENDER_ASSERT(2 == list.size());
                    TLRENDER_ASSERT(2 == list.front()->getByteCount());
                    TLRENDER_ASSERT(10 == list.front()->getData()[0]);
                    TLRENDER_ASSERT(11 == list.front()->getData()[1]);
                    for (size_t i = 0; i < 10; ++i)
                    {
                        TLRENDER_ASSERT(i == dataP[i]);
                    }
                }
            }
        }
    }
}
