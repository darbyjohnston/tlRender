// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Action.h>
#include <tlUI/IWidget.h>

#include <tlPlay/FilesModel.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! File tool bar.
        class FileToolBar : public ui::IWidget
        {
            DTK_NON_COPYABLE(FileToolBar);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<IWidget>& parent);

            FileToolBar();

        public:
            ~FileToolBar();

            static std::shared_ptr<FileToolBar> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _filesUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);

            DTK_PRIVATE();
        };
    }
}
