// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        class MainWindow;

        //! Color picker tool.
        class ColorPickerTool : public IToolWidget
        {
            Q_OBJECT

        public:
            ColorPickerTool(MainWindow*, App*, QWidget* parent = nullptr);

            virtual ~ColorPickerTool();

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Color picker tool dock widget.
        class ColorPickerDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            ColorPickerDockWidget(ColorPickerTool*, QWidget* parent = nullptr);
        };
    }
}
