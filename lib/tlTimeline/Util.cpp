// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>
#include <tlIO/Util.h>

#include <tlCore/Context.h>
#include <tlCore/FileInfo.h>

namespace tl
{
    namespace timeline
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            io::init(context);
        }

        std::vector<otime::TimeRange> toRanges(std::vector<otime::RationalTime> frames)
        {
            std::vector<otime::TimeRange> out;
            if (!frames.empty())
            {
                std::sort(frames.begin(), frames.end());
                auto i = frames.begin();
                auto j = i;
                do
                {
                    auto k = j + 1;
                    if (k != frames.end() && (*k - *j).value() > 1)
                    {
                        out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else if (k == frames.end())
                    {
                        out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else
                    {
                        ++j;
                    }
                } while (j != frames.end());
            }
            return out;
        }

        const otio::Composable* getRoot(const otio::Composable* composable)
        {
            const otio::Composable* out = composable;
            for (; out->parent(); out = out->parent())
                ;
            return out;
        }

        otio::optional<otime::RationalTime> getDuration(const otio::Timeline* timeline, const std::string& kind)
        {
            otio::optional<otime::RationalTime> out;
            otio::ErrorStatus errorStatus;
            for (auto track : timeline->children_if<otio::Track>(&errorStatus))
            {
                if (kind == track->kind())
                {
                    const otime::RationalTime duration = track->duration(&errorStatus);
                    if (out.has_value())
                    {
                        out = std::max(out.value(), duration);
                    }
                    else
                    {
                        out = duration;
                    }
                }
            }
            return out;
        }

        std::vector<file::Path> getPaths(
            const std::string& fileName,
            const file::PathOptions& pathOptions,
            const std::shared_ptr<system::Context>& context)
        {
            std::vector<file::Path> out;
            const auto path = file::Path(fileName);
            const auto fileInfo = file::FileInfo(path);
            switch (fileInfo.getType())
            {
            case file::Type::Directory:
            {
                auto ioSystem = context->getSystem<io::System>();
                for (const auto& fileInfo : file::dirList(fileName, pathOptions))
                {
                    const auto& path = fileInfo.getPath();
                    const auto& extension = path.getExtension();
                    switch (ioSystem->getFileType(extension))
                    {
                    case io::FileType::Sequence:
                    {
                        if (out.empty() || path.getNumber().empty())
                        {
                            out.push_back(path);
                        }
                        else
                        {
                            bool exists = false;
                            for (const auto& i : out)
                            {
                                if (i.getDirectory() == path.getDirectory() &&
                                    i.getBaseName() == path.getBaseName() &&
                                    !i.getNumber().empty() &&
                                    i.getPadding() == path.getPadding())
                                {
                                    exists = true;
                                    break;
                                }
                            }
                            if (!exists)
                            {
                                out.push_back(path);
                            }
                        }
                        break;
                    }
                    case io::FileType::Movie:
                    case io::FileType::Audio:
                        out.push_back(path);
                        break;
                    default:
                        //! \todo Get extensions for the Python adapters.
                        if (".otio" == extension)
                        {
                            out.push_back(path);
                        }
                        break;
                    }
                }
                break;
            }
            default:
                out.push_back(path);
                break;
            }
            return out;
        }
    }
}