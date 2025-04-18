// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Tools/IToolWidget.h>

#include <tlPlayApp/Models/FilesModel.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Files tool.
        class FilesTool : public IToolWidget
        {
            DTK_NON_COPYABLE(FilesTool);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            FilesTool();

        public:
            virtual ~FilesTool();

            static std::shared_ptr<FilesTool> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            void _filesUpdate(const std::vector<std::shared_ptr<FilesModelItem> >&);
            void _aUpdate(const std::shared_ptr<FilesModelItem>&);
            void _bUpdate(const std::vector<std::shared_ptr<FilesModelItem> >&);
            void _layersUpdate(const std::vector<int>&);
            void _compareUpdate(const timeline::CompareOptions&);

            DTK_PRIVATE();
        };
    }
}
