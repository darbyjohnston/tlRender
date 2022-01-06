// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/JPEG.h>

namespace tlr
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
            
        std::shared_ptr<Plugin> Plugin::create(const std::shared_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
                "JPEG",
                {
                    { ".jpeg", avio::FileExtensionType::VideoOnly },
                    { ".jpg", avio::FileExtensionType::VideoOnly }
                },
                logSystem);
            return out;
        }

        std::shared_ptr<avio::IRead> Plugin::read(
            const file::Path& path,
            const avio::Options& options)
        {
            std::shared_ptr<avio::IRead> out;
            if (auto logSystem = _logSystem.lock())
            {
                out = Read::create(path, avio::merge(options, _options), logSystem);
            }
            return out;
        }

        std::vector<imaging::PixelType> Plugin::getWritePixelTypes() const
        {
            return
            {
                imaging::PixelType::L_U8,
                imaging::PixelType::RGB_U8
            };
        }

        std::shared_ptr<avio::IWrite> Plugin::write(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options)
        {
            std::shared_ptr<avio::IWrite> out;
            if (auto logSystem = _logSystem.lock())
            {
                out = !info.video.empty() && _isWriteCompatible(info.video[0]) ?
                    Write::create(path, info, avio::merge(options, _options), logSystem) :
                    nullptr;
            }
            return out;
        }
    }
}
