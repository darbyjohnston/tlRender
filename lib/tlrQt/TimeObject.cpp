// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrQt/TimeObject.h>

#include <tlrCore/StringFormat.h>

#include <QDataStream>
#include <QRegExpValidator>

namespace tlr
{
    namespace qt
    {
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
                out = QString::fromStdString(string::Format("{0}").arg(time.to_frames()));
                break;
            case TimeUnits::Seconds:
                out = QString::fromStdString(string::Format("{0}").arg(time.to_seconds()));
                break;
            case TimeUnits::Timecode:
            {
                otime::ErrorStatus errorStatus;
                out = QString::fromStdString(time.to_timecode(&errorStatus));
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
                out = otime::RationalTime::from_seconds(text.toDouble());
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
