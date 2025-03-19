// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/Plugin.h>

#include <dtk/core/LogSystem.h>

namespace tl
{
    namespace io
    {
        void IIO::_init(
            const file::Path& path,
            const Options& options,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            _path = path;
            _options = options;
            _logSystem = logSystem;
        }

        IIO::IIO()
        {}

        IIO::~IIO()
        {}

        const file::Path& IIO::getPath() const
        {
            return _path;
        }

        struct IPlugin::Private
        {
            std::string name;
            std::map<std::string, FileType> extensions;
        };

        void IPlugin::_init(
            const std::string& name,
            const std::map<std::string, FileType>& extensions,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            DTK_P();
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
    }
}
