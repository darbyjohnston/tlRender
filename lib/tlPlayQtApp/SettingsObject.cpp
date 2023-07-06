// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/SettingsObject.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/ToolTipsFilter.h>

#include <tlTimeline/Player.h>

#include <QApplication>
#include <QMap>
#include <QSettings>

namespace tl
{
    namespace play_qt
    {
        namespace
        {
            const size_t settingsVersion = 3;

            QString version(const QString& value)
            {
                return QString("%1/%2/%3").
                    arg(QT_VERSION < QT_VERSION_CHECK(6, 0, 0) ? "Qt5" : "Qt6").
                    arg(settingsVersion).
                    arg(value);
            }
        }

        struct SettingsObject::Private
        {
            QMap<QString, QVariant> defaultValues;
            QSettings settings;
            QList<QString> recentFiles;
            bool toolTipsEnabled = true;
            qt::TimeObject* timeObject = nullptr;
            qt::ToolTipsFilter* toolTipsFilter = nullptr;
        };

        SettingsObject::SettingsObject(
            bool reset,
            qt::TimeObject* timeObject,
            QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            if (reset)
            {
                p.settings.clear();
            }

            p.defaultValues["Timeline/FrameView"] = true;
            p.defaultValues["Timeline/StopOnScrub"] = false;
            p.defaultValues["Timeline/Thumbnails"] = true;
            const timeline::PlayerCacheOptions playerCacheOptions;
            p.defaultValues["Cache/ReadAhead"] = playerCacheOptions.readAhead.value();
            p.defaultValues["Cache/ReadBehind"] = playerCacheOptions.readBehind.value();
            p.defaultValues["FileSequence/Audio"] =
                static_cast<int>(timeline::FileSequenceAudio::BaseName);
            p.defaultValues["FileSequence/AudioFileName"] = "";
            p.defaultValues["FileSequence/AudioDirectory"] = "";
            const timeline::PlayerOptions playerOptions;
            p.defaultValues["Performance/TimerMode"] =
                static_cast<int>(playerOptions.timerMode);
            p.defaultValues["Performance/AudioBufferFrameCount"] =
                static_cast<int>(playerOptions.audioBufferFrameCount);
            p.defaultValues["Performance/VideoRequestCount"] = 16;
            p.defaultValues["Performance/AudioRequestCount"] = 16;
            p.defaultValues["Performance/SequenceThreadCount"] = 16;
            p.defaultValues["Performance/FFmpegThreadCount"] = 0;
            p.defaultValues["Misc/MaxFileSequenceDigits"] = 9;

            int size = p.settings.beginReadArray(version("RecentFiles"));
            for (int i = 0; i < size; ++i)
            {
                p.settings.setArrayIndex(i);
                p.recentFiles.push_back(p.settings.value("File").toString());
            }
            p.settings.endArray();

            p.toolTipsEnabled = p.settings.value(
                version("Misc/ToolTipsEnabled"), true).toBool();

            p.timeObject = timeObject;
            p.timeObject->setTimeUnits(static_cast<timeline::TimeUnits>(p.settings.value(
                version("TimeUnits2"),
                static_cast<int>(p.timeObject->timeUnits())).toInt()));

            p.toolTipsFilter = new qt::ToolTipsFilter(this);

            _toolTipsUpdate();
        }

        SettingsObject::~SettingsObject()
        {
            TLRENDER_P();

            p.settings.beginWriteArray(version("RecentFiles"));
            for (size_t i = 0; i < p.recentFiles.size(); ++i)
            {
                p.settings.setArrayIndex(i);
                p.settings.setValue("File", p.recentFiles[i]);
            }
            p.settings.endArray();

            p.settings.setValue(
                version("Misc/ToolTipsEnabled"),
                p.toolTipsEnabled);

            p.settings.setValue(
                version("TimeUnits2"),
                static_cast<int>(p.timeObject->timeUnits()));
        }

        QVariant SettingsObject::value(const QString& name)
        {
            TLRENDER_P();
            QVariant defaultValue;
            const auto i = p.defaultValues.find(name);
            if (i != p.defaultValues.end())
            {
                defaultValue = *i;
            }
            return _p->settings.value(version(name), defaultValue);
        }

        const QList<QString>& SettingsObject::recentFiles() const
        {
            return _p->recentFiles;
        }

        bool SettingsObject::hasToolTipsEnabled() const
        {
            return _p->toolTipsEnabled;
        }

        void SettingsObject::setValue(const QString& name, const QVariant& value)
        {
            _p->settings.setValue(version(name), value);
            Q_EMIT valueChanged(name, value);
        }

        void SettingsObject::setDefaultValue(const QString& name, const QVariant& value)
        {
            _p->defaultValues[name] = value;
        }

        void SettingsObject::reset()
        {
            TLRENDER_P();
            for (auto i = p.defaultValues.begin(); i != p.defaultValues.end(); ++i)
            {
                p.settings.setValue(version(i.key()), i.value());
                Q_EMIT valueChanged(i.key(), i.value());
            }
            p.recentFiles.clear();
            Q_EMIT recentFilesChanged(p.recentFiles);
            p.toolTipsEnabled = true;
            Q_EMIT toolTipsEnabledChanged(p.toolTipsEnabled);
            _toolTipsUpdate();
        }

        void SettingsObject::setRecentFiles(const QList<QString>& value)
        {
            TLRENDER_P();
            if (value == p.recentFiles)
                return;
            p.recentFiles = value;
            Q_EMIT recentFilesChanged(p.recentFiles);
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
