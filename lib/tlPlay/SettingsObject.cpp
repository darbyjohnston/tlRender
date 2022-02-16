// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/SettingsObject.h>

#include <QApplication>
#include <QSettings>

namespace tl
{
    namespace play
    {
        struct SettingsObject::Private
        {
            QList<QString> recentFiles;
            const int recentFilesMax = 10;
            bool timelineThumbnails = true;
            double cacheReadAhead = 4.0;
            double cacheReadBehind = 0.4;
            timeline::FileSequenceAudio fileSequenceAudio = timeline::FileSequenceAudio::BaseName;
            QString fileSequenceAudioFileName;
            QString fileSequenceAudioDirectory;
            timeline::TimerMode timerMode = timeline::TimerMode::System;
            timeline::AudioBufferFrameCount audioBufferFrameCount = timeline::AudioBufferFrameCount::_256;
            int videoRequestCount = 16;
            int audioRequestCount = 16;
            int sequenceThreadCount = 16;
            int ffmpegThreadCount = 4;
            qt::TimeObject* timeObject = nullptr;
            int maxFileSequenceDigits = 9;
            bool toolTipsEnabled = true;
            qt::ToolTipsFilter* toolTipsFilter = nullptr;
        };

        SettingsObject::SettingsObject(qt::TimeObject* timeObject, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.timeObject = timeObject;

            p.toolTipsFilter = new qt::ToolTipsFilter(this);

            QSettings settings;
            p.timeObject->setUnits(settings.value("Settings/TimeUnits", QVariant::fromValue(p.timeObject->units())).value<qt::TimeUnits>());
            int size = settings.beginReadArray("Settings/RecentFiles");
            for (int i = 0; i < size; ++i)
            {
                settings.setArrayIndex(i);
                p.recentFiles.push_back(settings.value("File").toString().toUtf8().data());
            }
            settings.endArray();
            p.timelineThumbnails = settings.value("Settings/Timeline/Thumbnails", true).toBool();
            p.cacheReadAhead = settings.value("Settings/Cache/ReadAhead", 4.0).toDouble();
            p.cacheReadBehind = settings.value("Settings/Cache/ReadBehind", 0.4).toDouble();
            p.fileSequenceAudio = static_cast<timeline::FileSequenceAudio>(settings.value(
                "Settings/FileSequence/Audio",
                static_cast<int>(timeline::FileSequenceAudio::BaseName)).toInt());
            p.fileSequenceAudioFileName = settings.value("Settings/FileSequence/AudioFileName", "").toString();
            p.fileSequenceAudioDirectory = settings.value("Settings/FileSequence/AudioDirectory", "").toString();
            p.timerMode = static_cast<timeline::TimerMode>(settings.value(
                "Settings/Performance/TimerMode",
                static_cast<int>(timeline::TimerMode::System)).toInt());
            p.audioBufferFrameCount = static_cast<timeline::AudioBufferFrameCount>(settings.value(
                "Settings/Performance/AudioBufferFrameCount",
                static_cast<int>(timeline::AudioBufferFrameCount::_256)).toInt());
            p.videoRequestCount = settings.value("Settings/Performance/VideoRequestCount", 16).toInt();
            p.audioRequestCount = settings.value("Settings/Performance/AudioRequestCount", 16).toInt();
            p.sequenceThreadCount = settings.value("Settings/Performance/SequenceThreadCount", 16).toInt();
            p.ffmpegThreadCount = settings.value("Settings/Performance/FFmpegThreadCount", 4).toInt();
            p.maxFileSequenceDigits = settings.value("Settings/Misc/MaxFileSequenceDigits", 9).toInt();
            p.toolTipsEnabled = settings.value("Settings/Misc/ToolTipsEnabled", true).toBool();

            _toolTipsUpdate();
        }

        SettingsObject::~SettingsObject()
        {
            TLRENDER_P();
            QSettings settings;
            settings.setValue("Settings/TimeUnits", QVariant::fromValue(p.timeObject->units()));
            settings.beginWriteArray("Settings/RecentFiles");
            for (size_t i = 0; i < p.recentFiles.size(); ++i)
            {
                settings.setArrayIndex(i);
                settings.setValue("File", p.recentFiles[i]);
            }
            settings.endArray();
            settings.setValue("Settings/Timeline/Thumbnails", p.timelineThumbnails);
            settings.setValue("Settings/Cache/ReadAhead", p.cacheReadAhead);
            settings.setValue("Settings/Cache/ReadBehind", p.cacheReadBehind);
            settings.setValue("Settings/FileSequence/Audio", static_cast<int>(p.fileSequenceAudio));
            settings.setValue("Settings/FileSequence/AudioFileName", p.fileSequenceAudioFileName);
            settings.setValue("Settings/FileSequence/AudioDirectory", p.fileSequenceAudioDirectory);
            settings.setValue("Settings/Performance/TimerMode", static_cast<int>(p.timerMode));
            settings.setValue("Settings/Performance/AudioBufferFrameCount", static_cast<int>(p.audioBufferFrameCount));
            settings.setValue("Settings/Performance/VideoRequestCount", p.videoRequestCount);
            settings.setValue("Settings/Performance/VideoRequestCount", p.videoRequestCount);
            settings.setValue("Settings/Performance/VideoRequestCount", p.videoRequestCount);
            settings.setValue("Settings/Performance/AudioRequestCount", p.audioRequestCount);
            settings.setValue("Settings/Performance/SequenceThreadCount", p.sequenceThreadCount);
            settings.setValue("Settings/Performance/FFmpegThreadCount", p.ffmpegThreadCount);
            settings.setValue("Settings/Misc/MaxFileSequenceDigits", p.maxFileSequenceDigits);
            settings.setValue("Settings/Misc/ToolTipsEnabled", p.toolTipsEnabled);
        }

        const QList<QString>& SettingsObject::recentFiles() const
        {
            return _p->recentFiles;
        }

        bool SettingsObject::hasTimelineThumbnails() const
        {
            return _p->timelineThumbnails;
        }

        double SettingsObject::cacheReadAhead() const
        {
            return _p->cacheReadAhead;
        }

        double SettingsObject::cacheReadBehind() const
        {
            return _p->cacheReadBehind;
        }

        timeline::FileSequenceAudio SettingsObject::fileSequenceAudio() const
        {
            return _p->fileSequenceAudio;
        }

        const QString& SettingsObject::fileSequenceAudioFileName() const
        {
            return _p->fileSequenceAudioFileName;
        }

        const QString& SettingsObject::fileSequenceAudioDirectory() const
        {
            return _p->fileSequenceAudioDirectory;
        }

        timeline::TimerMode SettingsObject::timerMode()
        {
            return _p->timerMode;
        }

        timeline::AudioBufferFrameCount SettingsObject::audioBufferFrameCount() const
        {
            return _p->audioBufferFrameCount;
        }

        int SettingsObject::videoRequestCount() const
        {
            return _p->videoRequestCount;
        }

        int SettingsObject::audioRequestCount() const
        {
            return _p->audioRequestCount;
        }

        int SettingsObject::sequenceThreadCount() const
        {
            return _p->sequenceThreadCount;
        }

        int SettingsObject::ffmpegThreadCount() const
        {
            return _p->ffmpegThreadCount;
        }

        int SettingsObject::maxFileSequenceDigits() const
        {
            return _p->maxFileSequenceDigits;
        }

        bool SettingsObject::hasToolTipsEnabled() const
        {
            return _p->toolTipsEnabled;
        }

        void SettingsObject::addRecentFile(const QString& fileName)
        {
            TLRENDER_P();
            p.recentFiles.removeAll(fileName);
            p.recentFiles.insert(p.recentFiles.begin(), fileName);
            while (p.recentFiles.size() > p.recentFilesMax)
            {
                p.recentFiles.pop_back();
            }
            Q_EMIT recentFilesChanged(p.recentFiles);
        }

        void SettingsObject::setTimelineThumbnails(bool value)
        {
            TLRENDER_P();
            if (value == p.timelineThumbnails)
                return;
            p.timelineThumbnails = value;
            Q_EMIT timelineThumbnailsChanged(p.timelineThumbnails);
        }

        void SettingsObject::setCacheReadAhead(double value)
        {
            TLRENDER_P();
            if (value == p.cacheReadAhead)
                return;
            p.cacheReadAhead = value;
            Q_EMIT cacheReadAheadChanged(p.cacheReadAhead);
        }

        void SettingsObject::setCacheReadBehind(double value)
        {
            TLRENDER_P();
            if (value == p.cacheReadBehind)
                return;
            p.cacheReadBehind = value;
            Q_EMIT cacheReadBehindChanged(p.cacheReadBehind);
        }

        void SettingsObject::setFileSequenceAudio(timeline::FileSequenceAudio value)
        {
            TLRENDER_P();
            if (value == p.fileSequenceAudio)
                return;
            p.fileSequenceAudio = value;
            Q_EMIT fileSequenceAudioChanged(p.fileSequenceAudio);
        }

        void SettingsObject::setFileSequenceAudioFileName(const QString& value)
        {
            TLRENDER_P();
            if (value == p.fileSequenceAudioFileName)
                return;
            p.fileSequenceAudioFileName = value;
            Q_EMIT fileSequenceAudioFileNameChanged(p.fileSequenceAudioFileName);
        }

        void SettingsObject::setFileSequenceAudioDirectory(const QString& value)
        {
            TLRENDER_P();
            if (value == p.fileSequenceAudioDirectory)
                return;
            p.fileSequenceAudioDirectory = value;
            Q_EMIT fileSequenceAudioDirectoryChanged(p.fileSequenceAudioDirectory);
        }

        void SettingsObject::setTimerMode(timeline::TimerMode value)
        {
            TLRENDER_P();
            if (value == p.timerMode)
                return;
            p.timerMode = value;
            Q_EMIT timerModeChanged(p.timerMode);
        }

        void SettingsObject::setAudioBufferFrameCount(timeline::AudioBufferFrameCount value)
        {
            TLRENDER_P();
            if (value == p.audioBufferFrameCount)
                return;
            p.audioBufferFrameCount = value;
            Q_EMIT audioBufferFrameCountChanged(p.audioBufferFrameCount);
        }

        void SettingsObject::setVideoRequestCount(int value)
        {
            TLRENDER_P();
            if (value == p.videoRequestCount)
                return;
            p.videoRequestCount = value;
            Q_EMIT videoRequestCountChanged(p.videoRequestCount);
        }

        void SettingsObject::setAudioRequestCount(int value)
        {
            TLRENDER_P();
            if (value == p.audioRequestCount)
                return;
            p.audioRequestCount = value;
            Q_EMIT audioRequestCountChanged(p.audioRequestCount);
        }

        void SettingsObject::setSequenceThreadCount(int value)
        {
            TLRENDER_P();
            if (value == p.sequenceThreadCount)
                return;
            p.sequenceThreadCount = value;
            Q_EMIT sequenceThreadCountChanged(p.sequenceThreadCount);
        }

        void SettingsObject::setFFmpegThreadCount(int value)
        {
            TLRENDER_P();
            if (value == p.ffmpegThreadCount)
                return;
            p.ffmpegThreadCount = value;
            Q_EMIT ffmpegThreadCountChanged(p.ffmpegThreadCount);
        }

        void SettingsObject::setMaxFileSequenceDigits(int value)
        {
            TLRENDER_P();
            if (value == p.maxFileSequenceDigits)
                return;
            p.maxFileSequenceDigits = value;
            Q_EMIT maxFileSequenceDigitsChanged(p.maxFileSequenceDigits);
        }

        void SettingsObject::setToolTipsEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.toolTipsEnabled)
                return;
            p.toolTipsEnabled = value;
            _toolTipsUpdate();
            Q_EMIT toolTipsEnabledChanged(p.toolTipsEnabled);
        }

        void SettingsObject::_toolTipsUpdate()
        {
            TLRENDER_P();
            if (p.toolTipsEnabled)
            {
                qApp->removeEventFilter(p.toolTipsFilter);
            }
            else
            {
                qApp->installEventFilter(p.toolTipsFilter);
            }
        }
    }
}
