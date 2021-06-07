// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/OpenEXR.h>

namespace tlr
{
    namespace exr
    {
        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create()
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init({ ".exr" });
            return out;
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const std::string& fileName,
            const io::Options& options)
        {
            return Read::create(fileName, options);
        }

        std::vector<imaging::PixelType> Plugin::getWritePixelTypes() const
        {
            return
            {
                imaging::PixelType::RGBA_F16
            };
        }

        std::shared_ptr<io::IWrite> Plugin::write(
            const std::string& fileName,
            const io::Info& info,
            const io::Options& options)
        {
            return Write::create(fileName, info, options);
        }
    }
}
