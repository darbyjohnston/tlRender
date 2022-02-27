// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/JPEG.h>

#include <tlCore/StringFormat.h>

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

        Plugin::Plugin()
        {}

        std::shared_ptr<Plugin> Plugin::create(const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
                "JPEG",
                {
                    { ".jpeg", io::FileExtensionType::VideoOnly },
                    { ".jpg", io::FileExtensionType::VideoOnly }
                },
                logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            return Read::create(path, io::merge(options, _options), _logSystem);
        }

        imaging::Info Plugin::getWriteInfo(
            const imaging::Info& info,
            const io::Options& options) const
        {
            imaging::Info out;
            out.size = info.size;
            switch (info.pixelType)
            {
            case imaging::PixelType::L_U8:
            case imaging::PixelType::RGB_U8:
                out.pixelType = info.pixelType;
                break;
            default: break;
            }
            out.layout.mirror.y = true;
            return out;
        }

        std::shared_ptr<io::IWrite> Plugin::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isWriteCompatible(info.video[0], options)))
                throw std::runtime_error(string::Format("{0}: {1}").
                    arg(path.get()).
                    arg("Unsupported video"));
            return Write::create(path, info, io::merge(options, _options), _logSystem);
        }
    }
}
