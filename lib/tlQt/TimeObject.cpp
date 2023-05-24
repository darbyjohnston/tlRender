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
        QString sizeHintString(timeline::TimeUnits units)
        {
            QString out;
            switch (units)
            {
            case timeline::TimeUnits::Frames:
                out = "000000";
                break;
            case timeline::TimeUnits::Seconds:
                out = "000000.00";
                break;
            case timeline::TimeUnits::Timecode:
                out = "00:00:00;00";
                break;
            default: break;
            }
            return out;
        }

        QString validator(timeline::TimeUnits units)
        {
            QString out;
            switch (units)
            {
            case timeline::TimeUnits::Frames:
                out = "[0-9]*";
                break;
            case timeline::TimeUnits::Seconds:
                out = "[0-9]*\\.[0-9]+|[0-9]+";
                break;
            case timeline::TimeUnits::Timecode:
                out = "[0-9][0-9]:[0-9][0-9]:[0-9][0-9]:[0-9][0-9]";
                break;
            default: break;
            }
            return out;
        }

        QString timeToText(const otime::RationalTime& time, timeline::TimeUnits units)
        {
            QString out;
            switch (units)
            {
            case timeline::TimeUnits::Frames:
                out = QString::fromStdString(string::Format("{0}").
                    arg(time::isValid(time) ? time.to_frames() : 0));
                break;
            case timeline::TimeUnits::Seconds:
                out = QString::fromStdString(string::Format("{0}").
                    arg(time::isValid(time) ? time.to_seconds() : 0.0, 2));
                break;
            case timeline::TimeUnits::Timecode:
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
            timeline::TimeUnits units,
            otime::ErrorStatus* errorStatus)
        {
            otime::RationalTime out = time::invalidTime;
            switch (units)
            {
            case timeline::TimeUnits::Frames:
                out = otime::RationalTime::from_frames(text.toInt(), rate);
                break;
            case timeline::TimeUnits::Seconds:
                out = otime::RationalTime::from_seconds(text.toDouble()).rescaled_to(rate);
                break;
            case timeline::TimeUnits::Timecode:
                out = otime::RationalTime::from_timecode(text.toUtf8().data(), rate, errorStatus);
                break;
            default: break;
            }
            return out;
        }

        TimeObject::TimeObject(
            const std::shared_ptr<timeline::TimeUnitsModel>& model,
            QObject* parent) :
            QObject(parent),
            _model(model)
        {}

        timeline::TimeUnits TimeObject::timeUnits() const
        {
            return _model->getTimeUnits();
        }

        void TimeObject::setTimeUnits(timeline::TimeUnits value)
        {
            const timeline::TimeUnits units = _model->getTimeUnits();
            _model->setTimeUnits(value);
            if (units != _model->getTimeUnits())
            {
                Q_EMIT timeUnitsChanged(_model->getTimeUnits());
            }
        }

        QDataStream& operator << (QDataStream& ds, const timeline::TimeUnits& value)
        {
            ds << static_cast<qint32>(value);
            return ds;
        }

        QDataStream& operator >> (QDataStream& ds, timeline::TimeUnits& value)
        {
            qint32 tmp = 0;
            ds >> tmp;
            value = static_cast<timeline::TimeUnits>(tmp);
            return ds;
        }

    }
}
