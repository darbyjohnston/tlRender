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

        //! File menu.
        class FileMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(FileMenu);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            FileMenu();

        public:
            ~FileMenu();

            static std::shared_ptr<FileMenu> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            void close() override;

        private:
            void _filesUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _aUpdate(const std::shared_ptr<play::FilesModelItem>&);
            void _aIndexUpdate(int);
            void _recentUpdate(const std::vector<file::Path>&);

            TLRENDER_PRIVATE();
        };
    }
}
