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
        SettingsObject::SettingsObject(qt::TimeObject* timeObject, QObject* parent) :
            QObject(parent),
            _timeObject(timeObject)
        {
            _toolTipsFilter = new qt::ToolTipsFilter(this);

            QSettings settings;
            _timeObject->setUnits(settings.value("Settings/TimeUnits", QVariant::fromValue(_timeObject->units())).value<qt::TimeUnits>());
            int size = settings.beginReadArray("Settings/RecentFiles");
            for (int i = 0; i < size; ++i)
            {
                settings.setArrayIndex(i);
                _recentFiles.push_back(settings.value("File").toString().toUtf8().data());
            }
            settings.endArray();
            _cacheReadAhead = settings.value("Settings/Cache/ReadAhead", 4.0).toDouble();
            _cacheReadBehind = settings.value("Settings/Cache/ReadBehind", 0.4).toDouble();
            _fileSequenceAudio = static_cast<timeline::FileSequenceAudio>(settings.value(
                "Settings/FileSequence/Audio",
                static_cast<int>(timeline::FileSequenceAudio::BaseName)).toInt());
            _fileSequenceAudioFileName = settings.value("Settings/FileSequence/AudioFileName", "").toString();
            _fileSequenceAudioDirectory = settings.value("Settings/FileSequence/AudioDirectory", "").toString();
            _timerMode = static_cast<timeline::TimerMode>(settings.value(
                "Settings/Performance/TimerMode",
                static_cast<int>(timeline::TimerMode::System)).toInt());
            _audioBufferFrameCount = static_cast<timeline::AudioBufferFrameCount>(settings.value(
                "Settings/Performance/AudioBufferFrameCount",
                static_cast<int>(timeline::AudioBufferFrameCount::_256)).toInt());
            _videoRequestCount = settings.value("Settings/Performance/VideoRequestCount", 16).toInt();
            _audioRequestCount = settings.value("Settings/Performance/AudioRequestCount", 16).toInt();
            _sequenceThreadCount = settings.value("Settings/Performance/SequenceThreadCount", 16).toInt();
            _ffmpegThreadCount = settings.value("Settings/Performance/FFmpegThreadCount", 4).toInt();
            _maxFileSequenceDigits = settings.value("Settings/Misc/MaxFileSequenceDigits", 9).toInt();
            _toolTipsEnabled = settings.value("Settings/Misc/ToolTipsEnabled", true).toBool();

            _toolTipsUpdate();
        }

        SettingsObject::~SettingsObject()
        {
            QSettings settings;
            settings.setValue("Settings/TimeUnits", QVariant::fromValue(_timeObject->units()));
            settings.beginWriteArray("Settings/RecentFiles");
            for (size_t i = 0; i < _recentFiles.size(); ++i)
            {
                settings.setArrayIndex(i);
                settings.setValue("File", _recentFiles[i]);
            }
            settings.endArray();
            settings.setValue("Settings/Cache/ReadAhead", _cacheReadAhead);
            settings.setValue("Settings/Cache/ReadBehind", _cacheReadBehind);
            settings.setValue("Settings/FileSequence/Audio", static_cast<int>(_fileSequenceAudio));
            settings.setValue("Settings/FileSequence/AudioFileName", _fileSequenceAudioFileName);
            settings.setValue("Settings/FileSequence/AudioDirectory", _fileSequenceAudioDirectory);
            settings.setValue("Settings/Performance/TimerMode", static_cast<int>(_timerMode));
            settings.setValue("Settings/Performance/AudioBufferFrameCount", static_cast<int>(_audioBufferFrameCount));
            settings.setValue("Settings/Performance/VideoRequestCount", _videoRequestCount);
            settings.setValue("Settings/Performance/VideoRequestCount", _videoRequestCount);
            settings.setValue("Settings/Performance/VideoRequestCount", _videoRequestCount);
            settings.setValue("Settings/Performance/AudioRequestCount", _audioRequestCount);
            settings.setValue("Settings/Performance/SequenceThreadCount", _sequenceThreadCount);
            settings.setValue("Settings/Performance/FFmpegThreadCount", _ffmpegThreadCount);
            settings.setValue("Settings/Misc/MaxFileSequenceDigits", _maxFileSequenceDigits);
            settings.setValue("Settings/Misc/ToolTipsEnabled", _toolTipsEnabled);
        }

        const QList<QString>& SettingsObject::recentFiles() const
        {
            return _recentFiles;
        }

        double SettingsObject::cacheReadAhead() const
        {
            return _cacheReadAhead;
        }

        double SettingsObject::cacheReadBehind() const
        {
            return _cacheReadBehind;
        }

        timeline::FileSequenceAudio SettingsObject::fileSequenceAudio() const
        {
            return _fileSequenceAudio;
        }

        const QString& SettingsObject::fileSequenceAudioFileName() const
        {
            return _fileSequenceAudioFileName;
        }

        const QString& SettingsObject::fileSequenceAudioDirectory() const
        {
            return _fileSequenceAudioDirectory;
        }

        timeline::TimerMode SettingsObject::timerMode()
        {
            return _timerMode;
        }

        timeline::AudioBufferFrameCount SettingsObject::audioBufferFrameCount() const
        {
            return _audioBufferFrameCount;
        }

        int SettingsObject::videoRequestCount() const
        {
            return _videoRequestCount;
        }

        int SettingsObject::audioRequestCount() const
        {
            return _audioRequestCount;
        }

        int SettingsObject::sequenceThreadCount() const
        {
            return _sequenceThreadCount;
        }

        int SettingsObject::ffmpegThreadCount() const
        {
            return _ffmpegThreadCount;
        }

        int SettingsObject::maxFileSequenceDigits() const
        {
            return _maxFileSequenceDigits;
        }

        bool SettingsObject::hasToolTipsEnabled() const
        {
            return _toolTipsEnabled;
        }

        void SettingsObject::addRecentFile(const QString& fileName)
        {
            _recentFiles.removeAll(fileName);
            _recentFiles.insert(_recentFiles.begin(), fileName);
            while (_recentFiles.size() > _recentFilesMax)
            {
                _recentFiles.pop_back();
            }
            Q_EMIT recentFilesChanged(_recentFiles);
        }

        void SettingsObject::setCacheReadAhead(double value)
        {
            if (value == _cacheReadAhead)
                return;
            _cacheReadAhead = value;
            Q_EMIT cacheReadAheadChanged(_cacheReadAhead);
        }

        void SettingsObject::setCacheReadBehind(double value)
        {
            if (value == _cacheReadBehind)
                return;
            _cacheReadBehind = value;
            Q_EMIT cacheReadBehindChanged(_cacheReadBehind);
        }

        void SettingsObject::setFileSequenceAudio(timeline::FileSequenceAudio value)
        {
            if (value == _fileSequenceAudio)
                return;
            _fileSequenceAudio = value;
            Q_EMIT fileSequenceAudioChanged(_fileSequenceAudio);
        }

        void SettingsObject::setFileSequenceAudioFileName(const QString& value)
        {
            if (value == _fileSequenceAudioFileName)
                return;
            _fileSequenceAudioFileName = value;
            Q_EMIT fileSequenceAudioFileNameChanged(_fileSequenceAudioFileName);
        }

        void SettingsObject::setFileSequenceAudioDirectory(const QString& value)
        {
            if (value == _fileSequenceAudioDirectory)
                return;
            _fileSequenceAudioDirectory = value;
            Q_EMIT fileSequenceAudioDirectoryChanged(_fileSequenceAudioDirectory);
        }

        void SettingsObject::setTimerMode(timeline::TimerMode value)
        {
            if (value == _timerMode)
                return;
            _timerMode = value;
            Q_EMIT timerModeChanged(_timerMode);
        }

        void SettingsObject::setAudioBufferFrameCount(timeline::AudioBufferFrameCount value)
        {
            if (value == _audioBufferFrameCount)
                return;
            _audioBufferFrameCount = value;
            Q_EMIT audioBufferFrameCountChanged(_audioBufferFrameCount);
        }

        void SettingsObject::setVideoRequestCount(int value)
        {
            if (value == _videoRequestCount)
                return;
            _videoRequestCount = value;
            Q_EMIT videoRequestCountChanged(_videoRequestCount);
        }

        void SettingsObject::setAudioRequestCount(int value)
        {
            if (value == _audioRequestCount)
                return;
            _audioRequestCount = value;
            Q_EMIT audioRequestCountChanged(_audioRequestCount);
        }

        void SettingsObject::setSequenceThreadCount(int value)
        {
            if (value == _sequenceThreadCount)
                return;
            _sequenceThreadCount = value;
            Q_EMIT sequenceThreadCountChanged(_sequenceThreadCount);
        }

        void SettingsObject::setFFmpegThreadCount(int value)
        {
            if (value == _ffmpegThreadCount)
                return;
            _ffmpegThreadCount = value;
            Q_EMIT ffmpegThreadCountChanged(_ffmpegThreadCount);
        }

        void SettingsObject::setMaxFileSequenceDigits(int value)
        {
            if (value == _maxFileSequenceDigits)
                return;
            _maxFileSequenceDigits = value;
            Q_EMIT maxFileSequenceDigitsChanged(_maxFileSequenceDigits);
        }

        void SettingsObject::setToolTipsEnabled(bool value)
        {
            if (value == _toolTipsEnabled)
                return;
            _toolTipsEnabled = value;
            _toolTipsUpdate();
            Q_EMIT toolTipsEnabledChanged(_toolTipsEnabled);
        }

        void SettingsObject::_toolTipsUpdate()
        {
            if (_toolTipsEnabled)
            {
                qApp->removeEventFilter(_toolTipsFilter);
            }
            else
            {
                qApp->installEventFilter(_toolTipsFilter);
            }
        }
    }
}
