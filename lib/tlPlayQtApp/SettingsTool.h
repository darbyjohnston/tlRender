// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/ToolWidget.h>

#include <tlQt/TimeObject.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        class SettingsObject;

        //! Cache settings widget.
        class CacheSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            CacheSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            ~CacheSettingsWidget() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! File sequence settings widget.
        class FileSequenceSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            FileSequenceSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            ~FileSequenceSettingsWidget() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Performance settings widget.
        class PerformanceSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            PerformanceSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            ~PerformanceSettingsWidget() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Miscellaneous settings widget.
        class MiscSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            MiscSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            ~MiscSettingsWidget() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Settings tool.
        class SettingsTool : public ToolWidget
        {
            Q_OBJECT

        public:
            SettingsTool(
                SettingsObject*,
                qt::TimeObject*,
                QWidget* parent = nullptr);
        };

        //! Settings tool dock widget.
        class SettingsDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            SettingsDockWidget(
                SettingsTool*,
                QWidget* parent = nullptr);
        };
    }
}
