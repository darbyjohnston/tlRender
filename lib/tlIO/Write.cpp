// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/Write.h>

#include <dtk/core/LogSystem.h>

namespace tl
{
    namespace io
    {
        void IWrite::_init(
            const file::Path& path,
            const Options& options,
            const Info& info,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            IIO::_init(path, options, logSystem);
            _info = info;
        }

        IWrite::IWrite()
        {}

        IWrite::~IWrite()
        {}

        struct IWritePlugin::Private
        {
        };

        void IWritePlugin::_init(
            const std::string& name,
            const std::map<std::string, FileType>& extensions,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            IPlugin::_init(name, extensions, logSystem);
        }

        IWritePlugin::IWritePlugin() :
            _p(new Private)
        {}

        IWritePlugin::~IWritePlugin()
        {}

        bool IWritePlugin::_isCompatible(const dtk::ImageInfo& info, const Options& options) const
        {
            return info.type != dtk::ImageType::None && info == getInfo(info, options);
        }
    }
}
