// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/STB.h>

#include <feather-tk/core/Format.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace stb
    {
        ReadPlugin::ReadPlugin()
        {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(
                "STB",
                {
                    { ".tga", io::FileType::Sequence },
                    { ".bmp", io::FileType::Sequence },
                    { ".psd", io::FileType::Sequence },
                },
                logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            return Read::create(path, options, _logSystem.lock());
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const std::vector<feather_tk::InMemoryFile>& memory,
            const io::Options& options)
        {
            return Read::create( path, memory, options, _logSystem.lock());
        }

        WritePlugin::WritePlugin()
        {}

        std::shared_ptr<WritePlugin> WritePlugin::create(
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<WritePlugin>(new WritePlugin);
            out->_init(
                "STB",
                {
                    { ".tga", io::FileType::Sequence },
                    { ".bmp", io::FileType::Sequence }
                },
                logSystem);
            return out;
        }

        feather_tk::ImageInfo WritePlugin::getInfo(const feather_tk::ImageInfo& info, const io::Options& options) const
        {
            feather_tk::ImageInfo out;
            out.size = info.size;
            switch (info.type)
            {
            case feather_tk::ImageType::L_U8:
            case feather_tk::ImageType::LA_U8:
            case feather_tk::ImageType::RGB_U8:
            case feather_tk::ImageType::RGBA_U8:
                out.type = info.type;
                break;
            default: break;
            }
            out.layout.endian = feather_tk::Endian::MSB;
            return out;
        }

        std::shared_ptr<io::IWrite> WritePlugin::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isCompatible(info.video[0], options)))
                throw std::runtime_error(feather_tk::Format("Unsupported video: \"{0}\"").
                    arg(path.get()));
            return Write::create(path, info, options, _logSystem.lock());
        }
    }
}
