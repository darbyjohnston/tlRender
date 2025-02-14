// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/SettingsTool.h>

namespace tl
{
    namespace play_qt
    {
        //! Cache settings widget.
        class CacheSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            CacheSettingsWidget(App*, QWidget* parent = nullptr);

            virtual ~CacheSettingsWidget();

        private:
            DTK_PRIVATE();
        };

        //! File sequence settings widget.
        class FileSequenceSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            FileSequenceSettingsWidget(App*, QWidget* parent = nullptr);

            virtual ~FileSequenceSettingsWidget();

        private:
            DTK_PRIVATE();
        };

#if defined(TLRENDER_FFMPEG)
        //! FFmpeg settings widget.
        class FFmpegSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            FFmpegSettingsWidget(App*, QWidget* parent = nullptr);

            virtual ~FFmpegSettingsWidget();

        private:
            DTK_PRIVATE();
        };
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        //! USD settings widget.
        class USDSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            USDSettingsWidget(App*, QWidget* parent = nullptr);

            virtual ~USDSettingsWidget();

        private:
            DTK_PRIVATE();
        };
#endif // TLRENDER_USD

        //! File browser settings widget.
        class FileBrowserSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            FileBrowserSettingsWidget(App*, QWidget* parent = nullptr);

            virtual ~FileBrowserSettingsWidget();

        private:
            DTK_PRIVATE();
        };

        //! Performance settings widget.
        class PerformanceSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            PerformanceSettingsWidget(App*, QWidget* parent = nullptr);

            virtual ~PerformanceSettingsWidget();

        private:
            DTK_PRIVATE();
        };

        //! Miscellaneous settings widget.
        class MiscSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            MiscSettingsWidget(App*, QWidget* parent = nullptr);

            virtual ~MiscSettingsWidget();

        private:
            DTK_PRIVATE();
        };
    }
}
