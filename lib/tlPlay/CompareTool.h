// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/ToolWidget.h>

#include <tlQt/MetaTypes.h>

namespace tl
{
    namespace play
    {
        class FilesModel;

        //! Compare tool.
        class CompareTool : public ToolWidget
        {
            Q_OBJECT

        public:
            CompareTool(
                const std::shared_ptr<FilesModel>&,
                const std::shared_ptr<core::Context>&,
                QWidget* parent = nullptr);

            ~CompareTool() override;

        public Q_SLOTS:
            void setCompareOptions(const tl::render::CompareOptions&);

        private Q_SLOTS:
            void _activatedCallback(const QModelIndex&);
            void _modeCallback(const QVariant&);
            void _wipeXSpinBoxCallback(double);
            void _wipeXSliderCallback(int);
            void _wipeYSpinBoxCallback(double);
            void _wipeYSliderCallback(int);
            void _wipeRotationSpinBoxCallback(double);
            void _wipeRotationSliderCallback(int);

        Q_SIGNALS:
            void compareOptionsChanged(const tl::render::CompareOptions&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
