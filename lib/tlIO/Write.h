// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Plugin.h>

namespace tl
{
    namespace io
    {
        //! Base class for writers.
        class IWrite : public IIO
        {
        protected:
            void _init(
                const file::Path&,
                const Options&,
                const Info&,
                const std::shared_ptr<dtk::LogSystem>&);

            IWrite();

        public:
            virtual ~IWrite();

            //! Write video data.
            virtual void writeVideo(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<dtk::Image>&,
                const Options& = Options()) = 0;

        protected:
            Info _info;
        };

        //! Base class for write plugins.
        class IWritePlugin : public IPlugin
        {
            DTK_NON_COPYABLE(IWritePlugin);

        protected:
            void _init(
                const std::string& name,
                const std::map<std::string, FileType>& extensions,
                const std::shared_ptr<dtk::LogSystem>&);

            IWritePlugin();

        public:
            virtual ~IWritePlugin() = 0;

            //! Get information for writing.
            virtual dtk::ImageInfo getInfo(
                const dtk::ImageInfo&,
                const Options& = Options()) const = 0;

            //! Create a writer for the given path.
            virtual std::shared_ptr<IWrite> write(
                const file::Path&,
                const Info&,
                const Options& = Options()) = 0;

        protected:
            bool _isCompatible(const dtk::ImageInfo&, const Options&) const;

        private:
            DTK_PRIVATE();
        };
    }
}
