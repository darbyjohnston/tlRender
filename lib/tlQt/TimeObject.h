// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/Util.h>

#include <tlCore/Time.h>

#include <QMetaType>
#include <QObject>

namespace tl
{
    namespace qt
    {
        Q_NAMESPACE

        //! Time units.
        enum class TimeUnits
        {
            Frames,
            Seconds,
            Timecode,

            Count,
            First = Frames
        };
        TLRENDER_ENUM(TimeUnits);
        TLRENDER_ENUM_SERIALIZE(TimeUnits);
        Q_ENUM_NS(TimeUnits);
        QDataStream& operator << (QDataStream&, const TimeUnits&);
        QDataStream& operator >> (QDataStream&, TimeUnits&);

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
            Q_PROPERTY(
                tl::qt::TimeUnits units
                READ units
                WRITE setUnits
                NOTIFY unitsChanged)

        public:
            TimeObject(QObject* parent = nullptr);

            //! Get the time units.
            TimeUnits units() const;

        public Q_SLOTS:
            //! Set the time units.
            void setUnits(tl::qt::TimeUnits);

        Q_SIGNALS:
            //! This signal is emitted when the time units are changed.
            void unitsChanged(tl::qt::TimeUnits);

        private:
            TimeUnits _units = TimeUnits::Timecode;
        };
    }
}

Q_DECLARE_METATYPE(tl::qt::TimeUnits);

