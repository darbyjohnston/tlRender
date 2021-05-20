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

        //! Get the frame cache read ahead.
        int frameCacheReadAhead() const;

        //! Get the frame cache read behind.
        int frameCacheReadBehind() const;

        //! Get whether tool tips are enabled.
        bool hasToolTipsEnabled() const;

    public Q_SLOTS:
        //! Add a recent file.
        void addRecentFile(const QString&);

        //! Set the frame cache read ahead.
        void setFrameCacheReadAhead(int);

        //! Set the frame cache read behind.
        void setFrameCacheReadBehind(int);

        //! Set whether tool tips are enabled.
        void setToolTipsEnabled(bool);

    Q_SIGNALS:
        //! This signal is emitted when the recent files list is changed.
        void recentFilesChanged(const QList<QString>&);

        //! This signal is emitted when the frame cache read ahead is changed.
        void frameCacheReadAheadChanged(int);

        //! This signal is emitted when the frame cache read nehind is changed.
        void frameCacheReadBehindChanged(int);

        //! This signal is emitted when tool tips are enabled or disabled.
        void toolTipsEnabledChanged(bool);

    private:
        void _toolTipsUpdate();

        QList<QString> _recentFiles;
        const int _recentFilesMax = 10;
        int _frameCacheReadAhead = 100;
        int _frameCacheReadBehind = 10;
        qt::TimeObject* _timeObject = nullptr;
        bool _toolTipsEnabled = true;
        qt::ToolTipsFilter* _toolTipsFilter = nullptr;
    };
}
