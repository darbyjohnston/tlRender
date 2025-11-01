// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/OIIO.h>

#include <ftk/Core/Format.h>

#include <OpenImageIO/imagebufalgo.h>

namespace tl
{
    namespace oiio
    {
        void ReadPlugin::_init(const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            std::map<std::string, io::FileType> extensions;
            for (const auto& i : OIIO::get_extension_map())
            {
                for (const auto& extension : i.second)
                {
                    extensions["." + extension] = io::FileType::Sequence;
                }
            }
            IReadPlugin::_init("OIIO", extensions, logSystem);
        }

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<ftk::LogSystem>& logSystem)
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
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _logSystem.lock());
        }

        void WritePlugin::_init(const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            std::map<std::string, io::FileType> extensions;
            for (const auto& i : OIIO::get_extension_map())
            {
                for (const auto& extension : i.second)
                {
                    extensions[extension] = io::FileType::Sequence;
                }
            }
            IWritePlugin::_init("OIIO", extensions, logSystem);
        }

        std::shared_ptr<WritePlugin> WritePlugin::create(
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<WritePlugin>(new WritePlugin);
            out->_init(logSystem);
            return out;
        }

        ftk::ImageInfo WritePlugin::getInfo(
            const ftk::ImageInfo& info,
            const io::Options& options) const
        {
            ftk::ImageInfo out;
            out.size = info.size;
            switch (info.type)
            {
            case ftk::ImageType::L_U8:
            case ftk::ImageType::L_U16:
            case ftk::ImageType::L_U32:
            case ftk::ImageType::L_F16:
            case ftk::ImageType::L_F32:
            case ftk::ImageType::LA_U8:
            case ftk::ImageType::LA_U16:
            case ftk::ImageType::LA_U32:
            case ftk::ImageType::LA_F16:
            case ftk::ImageType::LA_F32:
            case ftk::ImageType::RGB_U8:
            case ftk::ImageType::RGB_U16:
            case ftk::ImageType::RGB_U32:
            case ftk::ImageType::RGB_F16:
            case ftk::ImageType::RGB_F32:
            case ftk::ImageType::RGBA_U8:
            case ftk::ImageType::RGBA_U16:
            case ftk::ImageType::RGBA_U32:
            case ftk::ImageType::RGBA_F16:
            case ftk::ImageType::RGBA_F32:
                out.type = info.type;
                break;
            default: break;
            }
            return out;
        }

        std::shared_ptr<io::IWrite> WritePlugin::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isCompatible(info.video[0], options)))
                throw std::runtime_error(ftk::Format("Unsupported video: \"{0}\"").
                    arg(path.get()));
            return Write::create(path, info, options, _logSystem.lock());
        }
    }
}
