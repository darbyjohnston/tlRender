// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayGLApp/IToolWidget.h>

#include <tlPlay/FilesModel.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! Files tool widget.
        class FilesToolWidget : public IToolWidget
        {
            TLRENDER_NON_COPYABLE(FilesToolWidget);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            FilesToolWidget();

        public:
            virtual ~FilesToolWidget();

            static std::shared_ptr<FilesToolWidget> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            void _filesUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _aUpdate(const std::shared_ptr<play::FilesModelItem>&);
            void _bUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _layersUpdate(const std::vector<int>&);
            void _compareUpdate(const timeline::CompareOptions&);

            TLRENDER_PRIVATE();
        };
    }
}
