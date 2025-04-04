// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/SGI.h>

#include <dtk/core/Format.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace sgi
    {
        ReadPlugin::ReadPlugin()
        {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(
                "SGI",
                {
                    { ".sgi", io::FileType::Sequence },
                    { ".rgba", io::FileType::Sequence },
                    { ".rgb", io::FileType::Sequence },
                    { ".bw", io::FileType::Sequence }
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
            const std::vector<dtk::InMemoryFile>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _logSystem.lock());
        }

        WritePlugin::WritePlugin()
        {}

        std::shared_ptr<WritePlugin> WritePlugin::create(
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<WritePlugin>(new WritePlugin);
            out->_init(
                "SGI",
                {
                    { ".sgi", io::FileType::Sequence }
                },
                logSystem);
            return out;
        }

        dtk::ImageInfo WritePlugin::getInfo(const dtk::ImageInfo& info, const io::Options& options) const
        {
            dtk::ImageInfo out;
            out.size = info.size;
            switch (info.type)
            {
            case dtk::ImageType::L_U8:
            case dtk::ImageType::L_U16:
            case dtk::ImageType::LA_U8:
            case dtk::ImageType::LA_U16:
            case dtk::ImageType::RGB_U8:
            case dtk::ImageType::RGB_U16:
            case dtk::ImageType::RGBA_U8:
            case dtk::ImageType::RGBA_U16:
                out.type = info.type;
                break;
            default: break;
            }
            out.layout.endian = dtk::Endian::MSB;
            return out;
        }

        std::shared_ptr<io::IWrite> WritePlugin::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isCompatible(info.video[0], options)))
                throw std::runtime_error(dtk::Format("Unsupported video: \"{0}\"").
                    arg(path.get()));
            return Write::create(path, info, options, _logSystem.lock());
        }
    }
}
