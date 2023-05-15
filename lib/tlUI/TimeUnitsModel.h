// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>
#include <tlCore/Time.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace ui
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
        TLRENDER_ENUM(TimeUnits);
        TLRENDER_ENUM_SERIALIZE(TimeUnits);

        //! Time units model.
        class TimeUnitsModel : public std::enable_shared_from_this<TimeUnitsModel>
        {
            TLRENDER_NON_COPYABLE(TimeUnitsModel);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&);

            TimeUnitsModel();

        public:
            ~TimeUnitsModel();

            //! Create a new model.
            static std::shared_ptr<TimeUnitsModel> create(
                const std::shared_ptr<system::Context>&);

            //! Get the time units.
            TimeUnits getTimeUnits() const;

            //! Observer the time units.
            std::shared_ptr<observer::IValue<TimeUnits> > observeTimeUnits() const;
            
            //! Set the time units.
            void setTimeUnits(TimeUnits);

            TLRENDER_PRIVATE();
        };
    }
}
