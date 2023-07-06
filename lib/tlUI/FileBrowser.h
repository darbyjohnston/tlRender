// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IDialog.h>

#include <tlCore/Path.h>

namespace tl
{
    namespace ui
    {
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

        private:
            TLRENDER_PRIVATE();
        };
    }
}
