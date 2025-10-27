// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/WMF.h>

namespace tl
{
    namespace wmf
    {
        bool Options::operator == (const Options& other) const
        {
            return true;
        }

        bool Options::operator != (const Options& other) const
        {
            return !(*this == other);
        }

        io::Options getOptions(const Options& value)
        {
            io::Options out;
            return out;
        }

        struct ReadPlugin::Private
        {
        };

        void ReadPlugin::_init(const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            FTK_P();

            std::map<std::string, io::FileType> extensions;
            extensions[".avi"] = io::FileType::Media;
            extensions[".mp3"] = io::FileType::Media;
            extensions[".m4a"] = io::FileType::Media;
            extensions[".m4v"] = io::FileType::Media;
            extensions[".mov"] = io::FileType::Media;
            extensions[".mp4"] = io::FileType::Media;
            extensions[".wav"] = io::FileType::Media;

            IReadPlugin::_init("WMF", extensions, logSystem);
        }

        ReadPlugin::ReadPlugin() :
            _p(new Private)
        {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            return Read::create(path, options, _logSystem.lock());
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _logSystem.lock());
        }

        void to_json(nlohmann::json& json, const Options& value)
        {
        }

        void from_json(const nlohmann::json& json, Options& value)
        {
        }
    }
}
