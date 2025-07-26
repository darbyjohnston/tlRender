// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/Cache.h>

#include <feather-tk/core/Format.h>
#include <feather-tk/core/LRUCache.h>
#include <feather-tk/core/String.h>

#include <mutex>

namespace tl
{
    namespace io
    {
        std::string getInfoCacheKey(
            const file::Path& path,
            const Options& options)
        {
            std::vector<std::string> s;
            s.push_back(path.get());
            s.push_back(path.getNumber());
            for (const auto& i : options)
            {
                s.push_back(feather_tk::Format("{0}:{1}").arg(i.first).arg(i.second));
            }
            return feather_tk::join(s, ';');
        }

        std::string getVideoCacheKey(
            const file::Path& path,
            const OTIO_NS::RationalTime& time,
            const Options& initOptions,
            const Options& frameOptions)
        {
            std::stringstream ss;
            ss << path.get() << ";" << path.getNumber() << ";" << time << ";";
            for (const auto& i : initOptions)
            {
                ss << i.first << ":" << i.second << ";";
            }
            for (const auto& i : frameOptions)
            {
                ss << i.first << ":" << i.second << ";";
            }
            return ss.str();
        }

        std::string getAudioCacheKey(
            const file::Path& path,
            const OTIO_NS::TimeRange& timeRange,
            const Options& initOptions,
            const Options& frameOptions)
        {
            std::stringstream ss;
            ss << path.get() << ";" << path.getNumber() << ";" << timeRange << ";";
            for (const auto& i : initOptions)
            {
                ss << i.first << ":" << i.second << ";";
            }
            for (const auto& i : frameOptions)
            {
                ss << i.first << ":" << i.second << ";";
            }
            return ss.str();
        }
    }
}
