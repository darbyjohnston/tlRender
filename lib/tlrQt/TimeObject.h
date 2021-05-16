// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Timeline.h>

#include <QObject>
#include <QMetaType>

namespace tlr
{
    namespace qt
    {
        //! Time object.
        class TimeObject : public QObject
        {
            Q_OBJECT

        public:
            TimeObject(QObject* parent = nullptr);

            //! Time units.
            enum class Units
            {
                Frames,
                Seconds,
                Timecode
            };
            Q_ENUM(Units);

            //! Get the time units.
            Units units() const;

            //! Get the time units size hint string.
            static QString unitsSizeHintString(Units);

            //! Get the time units validator regular expression.
            static QString unitsValidator(Units);

            //! Convert a time value to text.
            static QString timeToText(const otime::RationalTime&, Units);

            //! Convert text to a time value.
            static otime::RationalTime textToTime(
                const QString& text,
                double rate,
                Units,
                otime::ErrorStatus*);

        public Q_SLOTS:
            //! Set the time units.
            void setUnits(qt::TimeObject::Units);

        Q_SIGNALS:
            //! This signal is emitted when the time units are changed.
            void unitsChanged(qt::TimeObject::Units);

        private:
            Units _units = Units::Timecode;
        };
    }
}

Q_DECLARE_METATYPE(tlr::qt::TimeObject::Units);
