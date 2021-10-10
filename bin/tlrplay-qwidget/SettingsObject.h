// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimeObject.h>
#include <tlrQt/ToolTipsFilter.h>

#include <QObject>

namespace tlr
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
        int cacheReadAhead() const;

        //! Get the cache read behind.
        int cacheReadBehind() const;

        //! Get the timeline request count.
        int requestCount() const;

        //! Get the sequence I/O thread count.
        int sequenceThreadCount() const;

        //! Get the FFmpeg I/O thread count.
        int ffmpegThreadCount() const;

        //! Get whether tool tips are enabled.
        bool hasToolTipsEnabled() const;

    public Q_SLOTS:
        //! Add a recent file.
        void addRecentFile(const QString&);

        //! Set the cache read ahead.
        void setCacheReadAhead(int);

        //! Set the cache read behind.
        void setCacheReadBehind(int);

        //! Set the timeline request count.
        void setRequestCount(int);

        //! Set the sequence I/O thread count.
        void setSequenceThreadCount(int);

        //! Set the FFmepg I/O thread count.
        void setFFmpegThreadCount(int);

        //! Set whether tool tips are enabled.
        void setToolTipsEnabled(bool);

    Q_SIGNALS:
        //! This signal is emitted when the recent files list is changed.
        void recentFilesChanged(const QList<QString>&);

        //! This signal is emitted when the cache read ahead is changed.
        void cacheReadAheadChanged(int);

        //! This signal is emitted when the cache read behind is changed.
        void cacheReadBehindChanged(int);

        //! This signal is emitted when the timeline request count is changed.
        void requestCountChanged(int);

        //! This signal is emitted when the sequence I/O thread count is changed.
        void sequenceThreadCountChanged(int);

        //! This signal is emitted when the FFmpeg thread count is changed.
        void ffmpegThreadCountChanged(int);

        //! This signal is emitted when tool tips are enabled or disabled.
        void toolTipsEnabledChanged(bool);

    private:
        void _toolTipsUpdate();

        QList<QString> _recentFiles;
        const int _recentFilesMax = 10;
        int _cacheReadAhead = 100;
        int _cacheReadBehind = 10;
        int _requestCount = 16;
        int _sequenceThreadCount = 16;
        int _ffmpegThreadCount = 4;
        qt::TimeObject* _timeObject = nullptr;
        bool _toolTipsEnabled = true;
        qt::ToolTipsFilter* _toolTipsFilter = nullptr;
    };
}
