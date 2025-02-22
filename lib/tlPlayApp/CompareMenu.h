// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/FilesModel.h>

#include <dtk/ui/Menu.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Compare menu.
        class CompareMenu : public dtk::Menu
        {
            DTK_NON_COPYABLE(CompareMenu);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<dtk::Action> >&,
                const std::shared_ptr<IWidget>& parent);

            CompareMenu();

        public:
            ~CompareMenu();

            static std::shared_ptr<CompareMenu> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<dtk::Action> >&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void close() override;

        private:
            void _filesUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _bUpdate(const std::vector<int>&);
            void _compareUpdate(const timeline::CompareOptions&);
            void _compareTimeUpdate(timeline::CompareTimeMode);

            DTK_PRIVATE();
        };
    }
}
