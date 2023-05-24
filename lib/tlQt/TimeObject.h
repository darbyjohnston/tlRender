// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/Util.h>

#include <tlTimeline/TimeUnits.h>

#include <QMetaType>
#include <QObject>

namespace tl
{
    namespace qt
    {
        Q_NAMESPACE

        //! Get the time units size hint string.
        QString sizeHintString(timeline::TimeUnits);

        //! Get the time units validator regular expression.
        QString validator(timeline::TimeUnits);

        //! Convert a time value to text.
        QString timeToText(const otime::RationalTime&, timeline::TimeUnits);

        //! Convert text to a time value.
        otime::RationalTime textToTime(
            const QString& text,
            double rate,
            timeline::TimeUnits,
            otime::ErrorStatus*);

        //! Time object.
        class TimeObject : public QObject
        {
            Q_OBJECT
            Q_PROPERTY(
                tl::timeline::TimeUnits timeUnits
                READ timeUnits
                WRITE setTimeUnits
                NOTIFY timeUnitsChanged)

        public:
            TimeObject(
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                QObject* parent = nullptr);

            //! Get the time units.
            timeline::TimeUnits timeUnits() const;

        public Q_SLOTS:
            //! Set the time units.
            void setTimeUnits(tl::timeline::TimeUnits);

        Q_SIGNALS:
            //! This signal is emitted when the time units are changed.
            void timeUnitsChanged(tl::timeline::TimeUnits);

        private:
            std::shared_ptr<timeline::TimeUnitsModel> _model;
        };

        QDataStream& operator << (QDataStream&, const timeline::TimeUnits&);
        QDataStream& operator >> (QDataStream&, timeline::TimeUnits&);
    }
}
