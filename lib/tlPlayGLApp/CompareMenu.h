// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Menu.h>

#include <tlPlay/FilesModel.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! Compare menu.
        class CompareMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(CompareMenu);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            CompareMenu();

        public:
            ~CompareMenu();

            static std::shared_ptr<CompareMenu> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            void close() override;

        private:
            void _filesUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _currentUpdate(const std::vector<int>&);
            void _compareUpdate(const timeline::CompareOptions&);

            TLRENDER_PRIVATE();
        };
    }
}
