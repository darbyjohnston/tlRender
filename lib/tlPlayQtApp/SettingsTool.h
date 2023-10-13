// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

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

            virtual ~CacheSettingsWidget();

        private:
            TLRENDER_PRIVATE();
        };

        //! File sequence settings widget.
        class FileSequenceSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            FileSequenceSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            virtual ~FileSequenceSettingsWidget();

        private:
            TLRENDER_PRIVATE();
        };

#if defined(TLRENDER_FFMPEG)
        //! FFmpeg settings widget.
        class FFmpegSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            FFmpegSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            virtual ~FFmpegSettingsWidget();

        private:
            TLRENDER_PRIVATE();
        };
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        //! USD settings widget.
        class USDSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            USDSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            virtual ~USDSettingsWidget();

        private:
            TLRENDER_PRIVATE();
        };

        //! File browser settings widget.
        class FileBrowserSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            FileBrowserSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            virtual ~FileBrowserSettingsWidget();

        private:
            TLRENDER_PRIVATE();
        };
#endif // TLRENDER_USD

        //! Performance settings widget.
        class PerformanceSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            PerformanceSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            virtual ~PerformanceSettingsWidget();

        private:
            TLRENDER_PRIVATE();
        };

        //! Miscellaneous settings widget.
        class MiscSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            MiscSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            virtual ~MiscSettingsWidget();

        private:
            TLRENDER_PRIVATE();
        };

        //! Settings tool.
        class SettingsTool : public IToolWidget
        {
            Q_OBJECT

        public:
            SettingsTool(App*, QWidget* parent = nullptr);
        };

        //! Settings tool dock widget.
        class SettingsDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            SettingsDockWidget(SettingsTool*, QWidget* parent = nullptr);
        };
    }
}
