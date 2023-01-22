// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQt/TimeObject.h>

#include <tlCore/StringFormat.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <QDataStream>

#include <array>

namespace tl
{
    namespace qt
    {
        TLRENDER_ENUM_IMPL(
            TimeUnits,
            "Frames",
            "Seconds",
            "Timecode");
        TLRENDER_ENUM_SERIALIZE_IMPL(TimeUnits);

        QDataStream& operator << (QDataStream& ds, const TimeUnits& value)
        {
            ds << static_cast<qint32>(value);
            return ds;
        }
        
        QDataStream& operator >> (QDataStream& ds, TimeUnits& value)
        {
            qint32 tmp = 0;
            ds >> tmp;
            value = static_cast<TimeUnits>(tmp);
            return ds;
        }
        
        QString sizeHintString(TimeUnits units)
        {
            QString out;
            switch (units)
            {
            case TimeUnits::Frames:
                out = "000000";
                break;
            case TimeUnits::Seconds:
                out = "000000.00";
                break;
            case TimeUnits::Timecode:
                out = "00:00:00:00";
                break;
            default: break;
            }
            return out;
        }

        QString validator(TimeUnits units)
        {
            QString out;
            switch (units)
            {
            case TimeUnits::Frames:
                out = "[0-9]*";
                break;
            case TimeUnits::Seconds:
                out = "[0-9]*\\.[0-9]+|[0-9]+";
                break;
            case TimeUnits::Timecode:
                out = "[0-9][0-9]:[0-9][0-9]:[0-9][0-9]:[0-9][0-9]";
                break;
            default: break;
            }
            return out;
        }

        QString timeToText(const otime::RationalTime& time, TimeUnits units)
        {
            QString out;
            switch (units)
            {
            case TimeUnits::Frames:
                out = QString::fromStdString(string::Format("{0}").
                    arg(time::isValid(time) ? time.to_frames() : 0));
                break;
            case TimeUnits::Seconds:
                out = QString::fromStdString(string::Format("{0}").
                    arg(time::isValid(time) ? time.to_seconds() : 0.0, 2));
                break;
            case TimeUnits::Timecode:
            {
                otime::ErrorStatus errorStatus;
                out = QString::fromStdString(
                    time::isValid(time) ?
                    time.to_timecode(&errorStatus) :
                    "00:00:00:00");
                break;
            }
            default: break;
            }
            return out;
        }

        otime::RationalTime textToTime(
            const QString& text,
            double rate,
            TimeUnits units,
            otime::ErrorStatus* errorStatus)
        {
            otime::RationalTime out = time::invalidTime;
            switch (units)
            {
            case TimeUnits::Frames:
                out = otime::RationalTime::from_frames(text.toInt(), rate);
                break;
            case TimeUnits::Seconds:
                out = otime::RationalTime::from_seconds(text.toDouble()).rescaled_to(rate);
                break;
            case TimeUnits::Timecode:
                out = otime::RationalTime::from_timecode(text.toUtf8().data(), rate, errorStatus);
                break;
            default: break;
            }
            return out;
        }

        TimeObject::TimeObject(QObject* parent) :
            QObject(parent)
        {}

        TimeUnits TimeObject::units() const
        {
            return _units;
        }

        void TimeObject::setUnits(TimeUnits units)
        {
            if (_units == units)
                return;
            _units = units;
            Q_EMIT unitsChanged(_units);
        }
    }
}
