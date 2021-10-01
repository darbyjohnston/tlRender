// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/AVIO.h>
#include <tlrCore/ISystem.h>

namespace tlr
{
    namespace avio
    {
        //! I/O system.
        class System : public core::ISystem
        {
            TLR_NON_COPYABLE(System);

        protected:
            void _init(const std::shared_ptr<core::Context>&);
            System();

        public:
            ~System() override;

            //! Create a new I/O system.
            static std::shared_ptr<System> create(const std::shared_ptr<core::Context>&);

            //! Set the plugin options.
            void setOptions(const Options&);

            //! Get the list of plugins.
            const std::vector<std::shared_ptr<IPlugin> >& getPlugins() const;

            //! Get a plugin.
            template<typename T>
            std::shared_ptr<T> getPlugin() const;

            //! Get a plugin for the given path.
            std::shared_ptr<IPlugin> getPlugin(const file::Path&) const;

            //! Get the supported file extensions.
            std::set<std::string> getExtensions() const;

            // Create a reader for the given path.
            std::shared_ptr<IRead> read(
                const file::Path&,
                const Options& = Options());

            // Create a writer for the given path.
            std::shared_ptr<IWrite> write(
                const file::Path&,
                const Info&,
                const Options& = Options());

        private:
            std::vector<std::shared_ptr<IPlugin> > _plugins;
        };
    }
}

#include <tlrCore/AVIOSystemInline.h>
