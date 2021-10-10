// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/AVIO.h>

namespace tlr
{
    namespace avio
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
            const std::shared_ptr<core::LogSystem>& logSystem)
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
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            IIO::_init(path, options, logSystem);
        }

        IRead::IRead()
        {}

        IRead::~IRead()
        {}

        std::future<VideoData> IRead::readVideo(
            const otime::RationalTime&,
            uint16_t)
        {
            return std::future<VideoData>();
        }

        std::future<AudioData> IRead::readAudio(
            const otime::RationalTime&,
            size_t)
        {
            return std::future<AudioData>();
        }

        void IWrite::_init(
            const file::Path& path,
            const Options& options,
            const Info& info,
            const std::shared_ptr<core::LogSystem>& logSystem)
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
            std::set<std::string> extensions;
        };

        void IPlugin::_init(
            const std::string& name,
            const std::set<std::string>& extensions,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            TLR_PRIVATE_P();
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

        const std::set<std::string>& IPlugin::getExtensions() const
        {
            return _p->extensions;
        }

        void IPlugin::setOptions(const Options& options)
        {
            _options = options;
        }

        uint8_t IPlugin::getWriteAlignment(imaging::PixelType) const
        {
            return 1;
        }

        memory::Endian IPlugin::getWriteEndian() const
        {
            return memory::getEndian();
        }

        bool IPlugin::_isWriteCompatible(const imaging::Info& info) const
        {
            const auto pixelTypes = getWritePixelTypes();
            const auto i = std::find(pixelTypes.begin(), pixelTypes.end(), info.pixelType);
            return i != pixelTypes.end() &&
                info.layout.alignment == getWriteAlignment(info.pixelType) &&
                info.layout.endian == getWriteEndian();
        }
    }
}
