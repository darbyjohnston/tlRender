// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IDialog.h>

#include <tlCore/FileInfo.h>

namespace tl
{
    namespace ui
    {
        //! File browser options.
        struct FileBrowserOptions
        {
            std::string filter;
            std::string extension;
            file::ListOptions listOptions;

            bool operator == (const FileBrowserOptions&) const;
            bool operator != (const FileBrowserOptions&) const;
        };

        //! File browser.
        class FileBrowser : public IDialog
        {
            TLRENDER_NON_COPYABLE(FileBrowser);

        protected:
            void _init(
                const std::string&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            FileBrowser();

        public:
            ~FileBrowser() override;

            //! Create a new widget.
            static std::shared_ptr<FileBrowser> create(
                const std::string&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the file callback.
            void setFileCallback(const std::function<void(const file::Path&)>&);

            //! Get the options.
            const FileBrowserOptions& getOptions() const;

            //! Set the options.
            void setOptions(const FileBrowserOptions&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
