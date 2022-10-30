// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/SettingsObject.h>

#include <tlQt/ToolTipsFilter.h>

#include <tlTimeline/TimelinePlayer.h>

#include <QApplication>
#include <QMap>
#include <QSettings>

namespace tl
{
    namespace play
    {
        namespace
        {
            const size_t settingsVersion = 3;
            const int recentFilesMax = 10;

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

            p.defaultValues["Timeline/Thumbnails"] = true;
            p.defaultValues["Timeline/StopOnScrub"] = false;
            p.defaultValues["Cache/VideoSize"] = static_cast<int>(timeline::PlayerCacheOptions().videoByteCount / memory::gigabyte);
            p.defaultValues["Cache/AudioSize"] = static_cast<int>(timeline::PlayerCacheOptions().audioByteCount / memory::gigabyte);
            p.defaultValues["Cache/ReadAhead"] = timeline::PlayerCacheOptions().readAhead.value();
            p.defaultValues["Cache/ReadBehind"] = timeline::PlayerCacheOptions().readBehind.value();
            p.defaultValues["FileSequence/Audio"] =
                static_cast<int>(timeline::FileSequenceAudio::BaseName);
            p.defaultValues["FileSequence/AudioFileName"] = "";
            p.defaultValues["FileSequence/AudioDirectory"] = "";
            p.defaultValues["Performance/TimerMode"] =
                static_cast<int>(timeline::TimerMode::System);
            p.defaultValues["Performance/AudioBufferFrameCount"] =
                static_cast<int>(timeline::AudioBufferFrameCount::_256);
            p.defaultValues["Performance/VideoRequestCount"] = 16;
            p.defaultValues["Performance/AudioRequestCount"] = 16;
            p.defaultValues["Performance/SequenceThreadCount"] = 16;
            p.defaultValues["Performance/FFmpegThreadCount"] = 0;
            p.defaultValues["Misc/MaxFileSequenceDigits"] = 9;

            int size = p.settings.beginReadArray(version("RecentFiles"));
            for (int i = 0; i < size; ++i)
            {
                p.settings.setArrayIndex(i);
                p.recentFiles.push_back(p.settings.value("File").toString().toUtf8().data());
            }
            p.settings.endArray();

            p.toolTipsEnabled = p.settings.value(
                version("Misc/ToolTipsEnabled"), true).toBool();

            p.timeObject = timeObject;
            p.timeObject->setUnits(p.settings.value(
                version("TimeUnits"),
                QVariant::fromValue(p.timeObject->units())).value<qt::TimeUnits>());

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
                version("TimeUnits"),
                QVariant::fromValue(p.timeObject->units()));
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

        void SettingsObject::addRecentFile(const QString& fileName)
        {
            TLRENDER_P();
            p.recentFiles.removeAll(fileName);
            p.recentFiles.insert(p.recentFiles.begin(), fileName);
            while (p.recentFiles.size() > recentFilesMax)
            {
                p.recentFiles.pop_back();
            }
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
