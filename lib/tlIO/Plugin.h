// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/Path.h>

#include <dtk/core/FileIO.h>

#include <future>
#include <set>

namespace dtk
{
    class LogSystem;
}

namespace tl
{
    namespace io
    {
        //! Options.
        typedef std::map<std::string, std::string> Options;

        //! Merge options.
        Options merge(const Options&, const Options&);

        //! Base class for readers and writers.
        class IIO : public std::enable_shared_from_this<IIO>
        {
            DTK_NON_COPYABLE(IIO);

        protected:
            void _init(
                const file::Path&,
                const Options&,
                const std::shared_ptr<dtk::LogSystem>&);

            IIO();

        public:
            virtual ~IIO() = 0;

            //! Get the path.
            const file::Path& getPath() const;

        protected:
            file::Path _path;
            Options _options;
            std::weak_ptr<dtk::LogSystem> _logSystem;
        };

        //! Base class for I/O plugins.
        class IPlugin : public std::enable_shared_from_this<IPlugin>
        {
            DTK_NON_COPYABLE(IPlugin);

        protected:
            void _init(
                const std::string& name,
                const std::map<std::string, FileType>& extensions,
                const std::shared_ptr<dtk::LogSystem>&);

            IPlugin();

        public:
            virtual ~IPlugin() = 0;

            //! Get the plugin name.
            const std::string& getName() const;

            //! Get the supported file extensions.
            std::set<std::string> getExtensions(int types =
                static_cast<int>(FileType::Media) |
                static_cast<int>(FileType::Sequence)) const;

        protected:
            std::weak_ptr<dtk::LogSystem> _logSystem;

        private:
            DTK_PRIVATE();
        };
    }
}
