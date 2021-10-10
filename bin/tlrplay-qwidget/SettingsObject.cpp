// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "SettingsObject.h"

#include <QApplication>
#include <QSettings>

namespace tlr
{
    SettingsObject::SettingsObject(qt::TimeObject* timeObject, QObject* parent) :
        QObject(parent),
        _timeObject(timeObject)
    {
        _toolTipsFilter = new qt::ToolTipsFilter(this);

        QSettings settings;
        _timeObject->setUnits(settings.value("TimeUnits", QVariant::fromValue(_timeObject->units())).value<qt::TimeUnits>());
        int size = settings.beginReadArray("RecentFiles");
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            _recentFiles.push_back(settings.value("File").toString().toUtf8().data());
        }
        settings.endArray();
        _cacheReadAhead = settings.value("Cache/ReadAhead", 100).toInt();
        _cacheReadBehind = settings.value("Cache/ReadBehind", 10).toInt();
        _requestCount = settings.value("Performance/RequestCount", 16).toInt();
        _sequenceThreadCount = settings.value("Performance/SequenceThreadCount", 16).toInt();
        _ffmpegThreadCount = settings.value("Performance/FFmpegThreadCount", 4).toInt();
        _toolTipsEnabled = settings.value("Misc/ToolTipsEnabled", true).toBool();

        _toolTipsUpdate();
    }

    SettingsObject::~SettingsObject()
    {
        QSettings settings;
        settings.setValue("TimeUnits", QVariant::fromValue(_timeObject->units()));
        settings.beginWriteArray("RecentFiles");
        for (size_t i = 0; i < _recentFiles.size(); ++i)
        {
            settings.setArrayIndex(i);
            settings.setValue("File", _recentFiles[i]);
        }
        settings.endArray();
        settings.setValue("Cache/ReadAhead", _cacheReadAhead);
        settings.setValue("Cache/ReadBehind", _cacheReadBehind);
        settings.setValue("Performance/RequestCount", _requestCount);
        settings.setValue("Performance/SequenceThreadCount", _sequenceThreadCount);
        settings.setValue("Performance/FFmpegThreadCount", _ffmpegThreadCount);
        settings.setValue("Misc/ToolTipsEnabled", _toolTipsEnabled);
    }

    const QList<QString>& SettingsObject::recentFiles() const
    {
        return _recentFiles;
    }

    int SettingsObject::cacheReadAhead() const
    {
        return _cacheReadAhead;
    }

    int SettingsObject::cacheReadBehind() const
    {
        return _cacheReadBehind;
    }

    int SettingsObject::requestCount() const
    {
        return _requestCount;
    }

    int SettingsObject::sequenceThreadCount() const
    {
        return _sequenceThreadCount;
    }

    int SettingsObject::ffmpegThreadCount() const
    {
        return _ffmpegThreadCount;
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

    void SettingsObject::setCacheReadAhead(int value)
    {
        if (value == _cacheReadAhead)
            return;
        _cacheReadAhead = value;
        Q_EMIT cacheReadAheadChanged(_cacheReadAhead);
    }

    void SettingsObject::setCacheReadBehind(int value)
    {
        if (value == _cacheReadBehind)
            return;
        _cacheReadBehind = value;
        Q_EMIT cacheReadBehindChanged(_cacheReadBehind);
    }

    void SettingsObject::setRequestCount(int value)
    {
        if (value == _requestCount)
            return;
        _requestCount = value;
        Q_EMIT requestCountChanged(_requestCount);
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
