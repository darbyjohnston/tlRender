// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/ToolWidget.h>

namespace tl
{
    namespace core
    {
        class Context;
    }

    namespace play
    {
        class FilesModel;

        //! Files tool.
        class FilesTool : public ToolWidget
        {
            Q_OBJECT

        public:
            FilesTool(
                const std::shared_ptr<FilesModel>&,
                const std::shared_ptr<core::Context>&,
                QWidget* parent = nullptr);

            ~FilesTool() override;

        private Q_SLOTS:
            void _activatedCallback(const QModelIndex&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
