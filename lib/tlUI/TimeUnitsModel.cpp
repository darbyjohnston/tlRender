// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimeUnitsModel.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace ui
    {
        TLRENDER_ENUM_IMPL(
            TimeUnits,
            "Frames",
            "Seconds",
            "Timecode");
        TLRENDER_ENUM_SERIALIZE_IMPL(TimeUnits);

        struct TimeUnitsModel::Private
        {
            std::shared_ptr<observer::Value<TimeUnits> > timeUnits;
        };

        void TimeUnitsModel::_init(
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.timeUnits = observer::Value<TimeUnits>::create(TimeUnits::Timecode);
        }

        TimeUnitsModel::TimeUnitsModel() :
            _p(new Private)
        {}

        TimeUnitsModel::~TimeUnitsModel()
        {}

        std::shared_ptr<TimeUnitsModel> TimeUnitsModel::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<TimeUnitsModel>(new TimeUnitsModel);
            out->_init(context);
            return out;
        }

        TimeUnits TimeUnitsModel::getTimeUnits() const
        {
            return _p->timeUnits->get();
        }

        std::shared_ptr<observer::IValue<TimeUnits> > TimeUnitsModel::observeTimeUnits() const
        {
            return _p->timeUnits;
        }

        void TimeUnitsModel::setTimeUnits(TimeUnits value)
        {
            _p->timeUnits->setIfChanged(value);
        }
    }
}
