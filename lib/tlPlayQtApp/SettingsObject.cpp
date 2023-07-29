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

            int size = p.settings.beginReadArray(version("RecentFiles"));
            for (int i = 0; i < size; ++i)
            {
                p.settings.setArrayIndex(i);
                p.recentFiles.push_back(p.settings.value("File").toString());
            }
            p.settings.endArray();

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
                version("TimeUnits2"),
                static_cast<int>(p.timeObject->timeUnits()));
        }

        QVariant SettingsObject::value(const QString& name) const
        {
            return _p->settings.value(version(name));
        }

        void SettingsObject::setDefaultValue(const QString& name, const QVariant& value)
        {
            TLRENDER_P();
            if (!p.settings.contains(name))
            {
                p.settings.setValue(name, value);
            }
            p.defaultValues[name] = value;
        }

        const QList<QString>& SettingsObject::recentFiles() const
        {
            return _p->recentFiles;
        }

        void SettingsObject::setValue(const QString& name, const QVariant& value)
        {
            TLRENDER_P();
            if (!p.settings.contains(name))
            {
                p.defaultValues[name] = value;
            }
            p.settings.setValue(version(name), value);
            if (name == "Misc/ToolTipsEnabled")
            {
                _toolTipsUpdate();
            }
            Q_EMIT valueChanged(name, value);
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

        void SettingsObject::_toolTipsUpdate()
        {
            TLRENDER_P();
            if (value("Misc/ToolTipsEnabled").toBool())
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
