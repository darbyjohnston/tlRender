// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/AudioTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/AudioSystem.h>

#include <cstring>

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
                TLR_ASSERT(RTAUDIO_SINT16 == toRtAudio(DataType::S16));
                TLR_ASSERT(RTAUDIO_SINT32 == toRtAudio(DataType::S32));
                TLR_ASSERT(RTAUDIO_FLOAT32 == toRtAudio(DataType::F32));
                TLR_ASSERT(RTAUDIO_FLOAT64 == toRtAudio(DataType::F64));
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
                    TLR_ASSERT(out[i] == result[i]);
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
                    TLR_ASSERT(math::fuzzyCompare(out[i], result[i]));
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
                    TLR_ASSERT(out->getChannelCount() == in->getChannelCount());
                    TLR_ASSERT(out->getDataType() == j);
                    TLR_ASSERT(out->getSampleRate() == in->getSampleRate());
                    TLR_ASSERT(out->getSampleCount() == in->getSampleCount());
                }
            }
        }

        namespace
        {
            template<DataType DT, typename T>
            void _interleaveT()
            {
                {
                    auto in = Audio::create(Info(2, DT, 44100), 3);
                    T* inP = reinterpret_cast<T*>(in->getData());
                    inP[0] = 0;
                    inP[1] = 1;
                    inP[2] = 2;
                    inP[3] = 3;
                    inP[4] = 4;
                    inP[5] = 5;
                    const auto out = planarInterleave(in);
                    const T* outP = reinterpret_cast<const T*>(out->getData());
                    TLR_ASSERT(0 == outP[0]);
                    TLR_ASSERT(3 == outP[1]);
                    TLR_ASSERT(1 == outP[2]);
                    TLR_ASSERT(4 == outP[3]);
                    TLR_ASSERT(2 == outP[4]);
                    TLR_ASSERT(5 == outP[5]);
                }
                {
                    auto in = Audio::create(Info(2, DT, 44100), 3);
                    T* inP = reinterpret_cast<T*>(in->getData());
                    inP[0] = 0;
                    inP[1] = 3;
                    inP[2] = 1;
                    inP[3] = 4;
                    inP[4] = 2;
                    inP[5] = 5;
                    const auto out = planarDeinterleave(in);
                    const T* outP = reinterpret_cast<const T*>(out->getData());
                    TLR_ASSERT(0 == outP[0]);
                    TLR_ASSERT(1 == outP[1]);
                    TLR_ASSERT(2 == outP[2]);
                    TLR_ASSERT(3 == outP[3]);
                    TLR_ASSERT(4 == outP[4]);
                    TLR_ASSERT(5 == outP[5]);
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

                TLR_ASSERT(list.empty());
                for (size_t i = 0; i < 10; ++i)
                {
                    TLR_ASSERT(i == data[i]);
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

                TLR_ASSERT(5 == list.size());
                for (size_t i = 0; i < 10; ++i)
                {
                    TLR_ASSERT(i == data[i]);
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
