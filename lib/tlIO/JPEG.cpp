// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/JPEG.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace jpeg
    {
        void errorFunc(j_common_ptr in)
        {
            auto error = reinterpret_cast<ErrorStruct*>(in->err);
            char message[JMSG_LENGTH_MAX] = "";
            in->err->format_message(in, message);
            error->messages.push_back(message);
            ::longjmp(error->jump, 1);
        }

        void warningFunc(j_common_ptr in, int level)
        {
            if (level > 0)
            {
                return;
            }
            auto error = reinterpret_cast<ErrorStruct*>(in->err);
            char message[JMSG_LENGTH_MAX] = "";
            in->err->format_message(in, message);
            error->messages.push_back(message);
        }

        ReadPlugin::ReadPlugin()
        {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(
                "JPEG",
                {
                    { ".jpeg", io::FileType::Sequence },
                    { ".jpg", io::FileType::Sequence }
                },
                cache,
                logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            return Read::create(path, options, _cache, _logSystem.lock());
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _cache, _logSystem.lock());
        }

        WritePlugin::WritePlugin()
        {}

        std::shared_ptr<WritePlugin> WritePlugin::create(
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<WritePlugin>(new WritePlugin);
            out->_init(
                "JPEG",
                {
                    { ".jpeg", io::FileType::Sequence },
                    { ".jpg", io::FileType::Sequence }
                },
                logSystem);
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
            case dtk::ImageType::RGB_U8:
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
