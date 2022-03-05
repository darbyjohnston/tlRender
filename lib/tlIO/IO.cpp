// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/IO.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <array>

namespace tl
{
    namespace io
    {
        VideoData::VideoData()
        {}

        VideoData::VideoData(
            const otime::RationalTime& time,
            uint16_t layer,
            const std::shared_ptr<imaging::Image>& image) :
            time(time),
            layer(layer),
            image(image)
        {}

        AudioData::AudioData()
        {}

        AudioData::AudioData(
            const otime::RationalTime& time,
            const std::shared_ptr<audio::Audio>& audio) :
            time(time),
            audio(audio)
        {}

        Options merge(const Options& a, const Options& b)
        {
            Options out = a;
            out.insert(b.begin(), b.end());
            return out;
        }

        void IIO::_init(
            const file::Path& path,
            const Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            _logSystem = logSystem;
            _path = path;
            _options = options;
        }

        IIO::IIO()
        {}

        IIO::~IIO()
        {}

        void IRead::_init(
            const file::Path& path,
            const Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IIO::_init(path, options, logSystem);
        }

        IRead::IRead()
        {}

        IRead::~IRead()
        {}

        std::future<VideoData> IRead::readVideo(const otime::RationalTime&, uint16_t)
        {
            return std::future<VideoData>();
        }

        std::future<AudioData> IRead::readAudio(const otime::TimeRange&)
        {
            return std::future<AudioData>();
        }

        void IWrite::_init(
            const file::Path& path,
            const Options& options,
            const Info& info,
            const std::weak_ptr<log::System>& logSystem)
        {
            IIO::_init(path, options, logSystem);
            _info = info;
        }

        IWrite::IWrite()
        {}

        IWrite::~IWrite()
        {}

        struct IPlugin::Private
        {
            std::string name;
            std::map<std::string, FileType> extensions;
        };

        void IPlugin::_init(
            const std::string& name,
            const std::map<std::string, FileType>& extensions,
            const std::weak_ptr<log::System>& logSystem)
        {
            TLRENDER_P();
            _logSystem = logSystem;
            p.name = name;
            p.extensions = extensions;
        }

        IPlugin::IPlugin() :
            _p(new Private)
        {}

        IPlugin::~IPlugin()
        {}

        const std::string& IPlugin::getName() const
        {
            return _p->name;
        }

        std::set<std::string> IPlugin::getExtensions(int types) const
        {
            std::set<std::string> out;
            for (const auto& i : _p->extensions)
            {
                if (static_cast<int>(i.second) & types)
                {
                    out.insert(i.first);
                }
            }
            return out;
        }

        void IPlugin::setOptions(const Options& options)
        {
            _options = options;
        }

        bool IPlugin::_isWriteCompatible(const imaging::Info& info, const Options& options) const
        {
            return info.pixelType != imaging::PixelType::None && info == getWriteInfo(info, options);
        }
    }
}
