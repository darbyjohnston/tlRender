// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/DPX.h>

#include <sstream>

namespace tlr
{
    namespace dpx
    {
        void Plugin::_init()
        {
            IPlugin::_init(
                "DPX",
                { ".dpx" });
        }

        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create()
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init();
            return out;
        }

        std::shared_ptr<avio::IRead> Plugin::read(
            const std::string& fileName,
            const avio::Options& options)
        {
            return Read::create(fileName, options);
        }

        std::vector<imaging::PixelType> Plugin::getWritePixelTypes() const
        {
            return
            {
                imaging::PixelType::RGB_U10
            };
        }

        std::shared_ptr<avio::IWrite> Plugin::write(
            const std::string& fileName,
            const avio::Info& info,
            const avio::Options& options)
        {
            return Write::create(fileName, info, options);
        }
    }
}
