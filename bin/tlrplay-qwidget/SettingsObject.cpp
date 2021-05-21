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
        _timeObject->setUnits(settings.value("TimeUnits", QVariant::fromValue(_timeObject->units())).value<qt::TimeObject::Units>());
        int size = settings.beginReadArray("RecentFiles");
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            _recentFiles.push_back(settings.value("File").toString().toLatin1().data());
        }
        settings.endArray();
        _frameCacheReadAhead = settings.value("FrameCache/ReadAhead", 100).toInt();
        _frameCacheReadBehind = settings.value("FrameCache/ReadBehind", 10).toInt();
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
        settings.setValue("FrameCache/ReadAhead", _frameCacheReadAhead);
        settings.setValue("FrameCache/ReadBehind", _frameCacheReadBehind);
        settings.setValue("Misc/ToolTipsEnabled", _toolTipsEnabled);
    }

    const QList<QString>& SettingsObject::recentFiles() const
    {
        return _recentFiles;
    }

    int SettingsObject::frameCacheReadAhead() const
    {
        return _frameCacheReadAhead;
    }

    int SettingsObject::frameCacheReadBehind() const
    {
        return _frameCacheReadBehind;
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

    void SettingsObject::setFrameCacheReadAhead(int value)
    {
        if (value == _frameCacheReadAhead)
            return;
        _frameCacheReadAhead = value;
        Q_EMIT frameCacheReadAheadChanged(_frameCacheReadAhead);
    }

    void SettingsObject::setFrameCacheReadBehind(int value)
    {
        if (value == _frameCacheReadBehind)
            return;
        _frameCacheReadBehind = value;
        Q_EMIT frameCacheReadBehindChanged(_frameCacheReadBehind);
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
