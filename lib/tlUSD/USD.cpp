// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUSD/USD.h>

namespace tl
{
    namespace usd
    {
        Plugin::Plugin()
        {}
        
        std::shared_ptr<Plugin> Plugin::create(const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
                "USD",
                {
                    { ".usd", io::FileType::Sequence },
                    { ".usda", io::FileType::Sequence },
                    { ".usdc", io::FileType::Sequence }
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
        
        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            return Read::create(path, io::merge(options, _options), _logSystem);
        }
        
        imaging::Info Plugin::getWriteInfo(
            const imaging::Info&,
            const io::Options&) const
        {
            return imaging::Info();
        }
        
        std::shared_ptr<io::IWrite> Plugin::write(
            const file::Path&,
            const io::Info&,
            const io::Options&)
        {
            return nullptr;
        }
    }
}

