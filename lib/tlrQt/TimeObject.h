// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Time.h>

#include <QObject>
#include <QMetaType>

namespace tlr
{
    namespace qt
    {
        Q_NAMESPACE

        //! Time units.
        enum class TimeUnits
        {
            Frames,
            Seconds,
            Timecode
        };
        Q_ENUM_NS(TimeUnits);

        //! Get the time units size hint string.
        QString sizeHintString(TimeUnits);

        //! Get the time units validator regular expression.
        QString validator(TimeUnits);

        //! Convert a time value to text.
        QString timeToText(const otime::RationalTime&, TimeUnits);

        //! Convert text to a time value.
        otime::RationalTime textToTime(
            const QString& text,
            double rate,
            TimeUnits,
            otime::ErrorStatus*);

        //! Time object.
        class TimeObject : public QObject
        {
            Q_OBJECT

        public:
            TimeObject(QObject* parent = nullptr);

            //! Get the time units.
            TimeUnits units() const;

        public Q_SLOTS:
            //! Set the time units.
            void setUnits(qt::TimeUnits);

        Q_SIGNALS:
            //! This signal is emitted when the time units are changed.
            void unitsChanged(qt::TimeUnits);

        private:
            TimeUnits _units = TimeUnits::Timecode;
        };
    }
}

Q_DECLARE_METATYPE(tlr::qt::TimeUnits);
