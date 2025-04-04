// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Models/FilesModel.h>

#include <dtk/ui/Menu.h>

#include <filesystem>

namespace tl
{
    namespace play
    {
        class App;
        class FileActions;

        //! File menu.
        class FileMenu : public dtk::Menu
        {
            DTK_NON_COPYABLE(FileMenu);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<FileActions>&,
                const std::shared_ptr<IWidget>& parent);

            FileMenu();

        public:
            ~FileMenu();

            static std::shared_ptr<FileMenu> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<FileActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void close() override;

        private:
            void _filesUpdate(const std::vector<std::shared_ptr<FilesModelItem> >&);
            void _aUpdate(const std::shared_ptr<FilesModelItem>&);
            void _aIndexUpdate(int);
            void _layersUpdate(const std::vector<int>&);
            void _recentUpdate(const std::vector<std::filesystem::path>&);

            DTK_PRIVATE();
        };
    }
}
