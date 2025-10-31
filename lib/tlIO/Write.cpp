// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/Write.h>

#include <ftk/Core/LogSystem.h>

namespace tl
{
    namespace io
    {
        void IWrite::_init(
            const file::Path& path,
            const Options& options,
            const Info& info,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
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
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IPlugin::_init(name, extensions, logSystem);
        }

        IWritePlugin::IWritePlugin() :
            _p(new Private)
        {}

        IWritePlugin::~IWritePlugin()
        {}

        bool IWritePlugin::_isCompatible(const ftk::ImageInfo& info, const Options& options) const
        {
            return info.type != ftk::ImageType::None && info == getInfo(info, options);
        }
    }
}
