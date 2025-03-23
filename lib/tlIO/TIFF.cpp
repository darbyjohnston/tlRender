// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/TIFF.h>

#include <dtk/core/Format.h>

#include <tiffio.h>

#include <sstream>

namespace tl
{
    namespace tiff
    {
        void ReadPlugin::_init(const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            IReadPlugin::_init(
                "TIFF",
                {
                    { ".tiff", io::FileType::Sequence },
                    { ".tif", io::FileType::Sequence }
                },
                logSystem);
            TIFFSetErrorHandler(nullptr);
            TIFFSetWarningHandler(nullptr);
        }

        ReadPlugin::ReadPlugin()
        {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(logSystem);
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

        void WritePlugin::_init(const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            IWritePlugin::_init(
                "TIFF",
                {
                    { ".tiff", io::FileType::Sequence },
                    { ".tif", io::FileType::Sequence }
                },
                logSystem);
            TIFFSetErrorHandler(nullptr);
            TIFFSetWarningHandler(nullptr);
        }

        WritePlugin::WritePlugin()
        {}

        std::shared_ptr<WritePlugin> WritePlugin::create(
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<WritePlugin>(new WritePlugin);
            out->_init(logSystem);
            return out;
        }

        dtk::ImageInfo WritePlugin::getInfo(
            const dtk::ImageInfo& info,
            const io::Options& options) const
        {
            dtk::ImageInfo out;
            out.size = info.size;
            switch (info.type)
            {
            case dtk::ImageType::L_U8:
            case dtk::ImageType::L_U16:
            case dtk::ImageType::L_F32:
            case dtk::ImageType::LA_U8:
            case dtk::ImageType::LA_U16:
            case dtk::ImageType::LA_F32:
            case dtk::ImageType::RGB_U8:
            case dtk::ImageType::RGB_U16:
            case dtk::ImageType::RGB_F32:
            case dtk::ImageType::RGBA_U8:
            case dtk::ImageType::RGBA_U16:
            case dtk::ImageType::RGBA_F32:
                out.type = info.type;
                break;
            default: break;
            }
            out.layout.mirror.y = true;
            return out;
        }

        std::shared_ptr<io::IWrite> WritePlugin::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isCompatible(info.video[0], options)))
                throw std::runtime_error(dtk::Format("{0}: {1}").
                    arg(path.get()).
                    arg("Unsupported video"));
            return Write::create(path, info, options, _logSystem.lock());
        }
    }
}
