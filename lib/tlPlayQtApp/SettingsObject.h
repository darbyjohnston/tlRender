// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>

#include <QObject>
#include <QVariant>

#include <memory>

namespace tl
{
    namespace play_qt
    {
        //! Settings object.
        class SettingsObject : public QObject
        {
            Q_OBJECT

        public:
            SettingsObject(
                bool reset,
                qt::TimeObject*,
                QObject* parent = nullptr);

            ~SettingsObject() override;

            //! Get a settings value.
            QVariant value(const QString&) const;

            //! Set a default value.
            void setDefaultValue(const QString&, const QVariant&);

            //! Get the list of recent files.
            const QList<QString>& recentFiles() const;

        public Q_SLOTS:
            //! Set a settings value.
            void setValue(const QString&, const QVariant&);

            //! Reset the settings to defaults.
            void reset();

            //! Set the recent files.
            void setRecentFiles(const QList<QString>&);

        Q_SIGNALS:
            //! This signal is emitted when a settings value is changed.
            void valueChanged(const QString&, const QVariant&);

            //! This signal is emitted when the recent files list is changed.
            void recentFilesChanged(const QList<QString>&);

        private:
            void _toolTipsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
