// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/ToolWidget.h>

#include <tlQt/TimeObject.h>

#include <tlCore/TimelinePlayer.h>

namespace tl
{
    namespace play
    {
        class SettingsObject;

        //! Cache settings widget.
        class CacheSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            CacheSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

            ~CacheSettingsWidget() override;

        private Q_SLOTS:
            void _readAheadCallback(double);
            void _readBehindCallback(double);

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

        private Q_SLOTS:
            void _audioCallback(int);
            void _audioCallback(tl::timeline::FileSequenceAudio);
            void _audioFileNameCallback(const QString&);
            void _audioDirectoryCallback(const QString&);
            void _maxDigitsCallback(int);

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

        private Q_SLOTS:
            void _timerModeCallback(int);
            void _timerModeCallback(tl::timeline::TimerMode);
            void _audioBufferFrameCountCallback(int);
            void _audioBufferFrameCountCallback(tl::timeline::AudioBufferFrameCount);
            void _videoRequestCountCallback(int);
            void _audioRequestCountCallback(int);
            void _sequenceThreadCountCallback(int);
            void _ffmpegThreadCountCallback(int);

        private:
            TLRENDER_PRIVATE();
        };

        //! Time settings widget.
        class TimeSettingsWidget : public QWidget
        {
            Q_OBJECT

        public:
            TimeSettingsWidget(qt::TimeObject*, QWidget* parent = nullptr);

            ~TimeSettingsWidget() override;

        private Q_SLOTS:
            void _unitsCallback(const QVariant&);
            void _unitsCallback(tl::qt::TimeUnits);

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

        private Q_SLOTS:
            void _toolTipsCallback(int);
            void _toolTipsCallback(bool);

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
    }
}
