// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Models/FilesModel.h>

#include <dtk/ui/Action.h>
#include <dtk/ui/IWidget.h>

namespace tl
{
    namespace play
    {
        class App;

        //! File tool bar.
        class FileToolBar : public dtk::IWidget
        {
            DTK_NON_COPYABLE(FileToolBar);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<dtk::Action> >&,
                const std::shared_ptr<IWidget>& parent);

            FileToolBar();

        public:
            ~FileToolBar();

            static std::shared_ptr<FileToolBar> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<dtk::Action> >&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            void _filesUpdate(const std::vector<std::shared_ptr<FilesModelItem> >&);

            DTK_PRIVATE();
        };
    }
}
