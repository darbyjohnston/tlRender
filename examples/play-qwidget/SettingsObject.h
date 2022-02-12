// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>
#include <tlQt/ToolTipsFilter.h>

#include <tlCore/TimelinePlayer.h>

#include <QObject>

namespace tl
{
    //! Settings object.
    class SettingsObject : public QObject
    {
        Q_OBJECT

    public:
        SettingsObject(qt::TimeObject*, QObject* parent = nullptr);

        ~SettingsObject() override;

        //! Get the list of recent files.
        const QList<QString>& recentFiles() const;

        //! Get the cache read ahead.
        double cacheReadAhead() const;

        //! Get the cache read behind.
        double cacheReadBehind() const;

        //! Get the file sequence audio.
        timeline::FileSequenceAudio fileSequenceAudio() const;

        //! Get the file sequence audio file name.
        const QString& fileSequenceAudioFileName() const;

        //! Get the file sequence audio directory.
        const QString& fileSequenceAudioDirectory() const;

        //! Get the timer mode.
        timeline::TimerMode timerMode();

        //! Get the audio buffer frame count.
        timeline::AudioBufferFrameCount audioBufferFrameCount() const;

        //! Get the video request count.
        int videoRequestCount() const;

        //! Get the audio request count.
        int audioRequestCount() const;

        //! Get the sequence I/O thread count.
        int sequenceThreadCount() const;

        //! Get the FFmpeg I/O thread count.
        int ffmpegThreadCount() const;

        //! Get the maximum number of file seqeunce digits.
        int maxFileSequenceDigits() const;

        //! Get whether tool tips are enabled.
        bool hasToolTipsEnabled() const;

    public Q_SLOTS:
        //! Add a recent file.
        void addRecentFile(const QString&);

        //! Set the cache read ahead.
        void setCacheReadAhead(double);

        //! Set the cache read behind.
        void setCacheReadBehind(double);

        //! Set the file sequence audio.
        void setFileSequenceAudio(tl::timeline::FileSequenceAudio);

        //! Set the file sequence audio file name.
        void setFileSequenceAudioFileName(const QString&);

        //! Set the file sequence audio directory.
        void setFileSequenceAudioDirectory(const QString&);

        //! Set the timer mode.
        void setTimerMode(tl::timeline::TimerMode);

        //! Set the audio buffer frame count.
        void setAudioBufferFrameCount(tl::timeline::AudioBufferFrameCount);

        //! Set the video request count.
        void setVideoRequestCount(int);

        //! Set the audio request count.
        void setAudioRequestCount(int);

        //! Set the sequence I/O thread count.
        void setSequenceThreadCount(int);

        //! Set the FFmepg I/O thread count.
        void setFFmpegThreadCount(int);

        //! Set the maximum number of file sequence digits.
        void setMaxFileSequenceDigits(int);

        //! Set whether tool tips are enabled.
        void setToolTipsEnabled(bool);

    Q_SIGNALS:
        //! This signal is emitted when the recent files list is changed.
        void recentFilesChanged(const QList<QString>&);

        //! This signal is emitted when the cache read ahead is changed.
        void cacheReadAheadChanged(double);

        //! This signal is emitted when the cache read behind is changed.
        void cacheReadBehindChanged(double);

        //! This signal is emitted when the file sequence audio is changed.
        void fileSequenceAudioChanged(tl::timeline::FileSequenceAudio);

        //! This signal is emitted when the file sequence audio file name is changed.
        void fileSequenceAudioFileNameChanged(const QString&);

        //! This signal is emitted when the file sequence audio directory is changed.
        void fileSequenceAudioDirectoryChanged(const QString&);

        //! This signal is emitted when the timer mode is changed.
        void timerModeChanged(tl::timeline::TimerMode);

        //! This signal is emitted when the audio buffer frame count is changed.
        void audioBufferFrameCountChanged(tl::timeline::AudioBufferFrameCount);

        //! This signal is emitted when the video request count is changed.
        void videoRequestCountChanged(int);

        //! This signal is emitted when the audio request count is changed.
        void audioRequestCountChanged(int);

        //! This signal is emitted when the sequence I/O thread count is changed.
        void sequenceThreadCountChanged(int);

        //! This signal is emitted when the FFmpeg thread count is changed.
        void ffmpegThreadCountChanged(int);

        //!This signal is emitted when maximum number of file sequence digits is changed.
        void maxFileSequenceDigitsChanged(int);

        //! This signal is emitted when tool tips are enabled or disabled.
        void toolTipsEnabledChanged(bool);

    private:
        void _toolTipsUpdate();

        QList<QString> _recentFiles;
        const int _recentFilesMax = 10;
        double _cacheReadAhead = 4.0;
        double _cacheReadBehind = 0.4;
        timeline::FileSequenceAudio _fileSequenceAudio = timeline::FileSequenceAudio::BaseName;
        QString _fileSequenceAudioFileName;
        QString _fileSequenceAudioDirectory;
        timeline::TimerMode _timerMode = timeline::TimerMode::System;
        timeline::AudioBufferFrameCount _audioBufferFrameCount = timeline::AudioBufferFrameCount::_256;
        int _videoRequestCount = 16;
        int _audioRequestCount = 16;
        int _sequenceThreadCount = 16;
        int _ffmpegThreadCount = 4;
        qt::TimeObject* _timeObject = nullptr;
        int _maxFileSequenceDigits = 9;
        bool _toolTipsEnabled = true;
        qt::ToolTipsFilter* _toolTipsFilter = nullptr;
    };
}
