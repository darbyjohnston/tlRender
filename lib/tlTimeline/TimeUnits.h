// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Time.h>

#include <dtk/core/ObservableValue.h>

namespace dtk
{
    class Context;
}

namespace tl
{
    namespace timeline
    {
        //! Time units.
        enum class TimeUnits
        {
            Frames,
            Seconds,
            Timecode,

            Count,
            First = Frames
        };
        DTK_ENUM(TimeUnits);

        //! Convert a time value to text.
        std::string timeToText(const OTIO_NS::RationalTime&, timeline::TimeUnits);

        //! Convert text to a time value.
        OTIO_NS::RationalTime textToTime(
            const std::string&     text,
            double                 rate,
            timeline::TimeUnits    units,
            opentime::ErrorStatus* error = nullptr);

        //! Get a time units format string.
        std::string formatString(timeline::TimeUnits);

        //! Get a time units validator regular expression.
        std::string validator(timeline::TimeUnits);

        //! Base class for time units models.
        class ITimeUnitsModel : public std::enable_shared_from_this<ITimeUnitsModel>
        {
            DTK_NON_COPYABLE(ITimeUnitsModel);

        protected:
            void _init(const std::shared_ptr<dtk::Context>&);

            ITimeUnitsModel();

        public:
            virtual ~ITimeUnitsModel() = 0;

            //! Observer when the time units are changed.
            std::shared_ptr<dtk::IObservableValue<bool> > observeTimeUnitsChanged() const;

            //! Get a time label in the current time units.
            virtual std::string getLabel(const OTIO_NS::RationalTime&) const = 0;

        protected:
            std::shared_ptr<dtk::ObservableValue<bool> > _timeUnitsChanged;
        };

        //! Time units model.
        class TimeUnitsModel : public ITimeUnitsModel
        {
            DTK_NON_COPYABLE(TimeUnitsModel);

        protected:
            void _init(const std::shared_ptr<dtk::Context>&);

            TimeUnitsModel();

        public:
            virtual ~TimeUnitsModel();

            //! Create a new model.
            static std::shared_ptr<TimeUnitsModel> create(
                const std::shared_ptr<dtk::Context>&);

            //! Get the time units.
            TimeUnits getTimeUnits() const;

            //! Observer the time units.
            std::shared_ptr<dtk::IObservableValue<TimeUnits> > observeTimeUnits() const;
            
            //! Set the time units.
            void setTimeUnits(TimeUnits);

            std::string getLabel(const OTIO_NS::RationalTime&) const override;

            DTK_PRIVATE();
        };
    }
}
