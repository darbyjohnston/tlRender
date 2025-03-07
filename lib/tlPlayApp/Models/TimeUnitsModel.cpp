// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Models/TimeUnitsModel.h>

#include <dtk/ui/Settings.h>

namespace tl
{
    namespace play
    {
        struct TimeUnitsModel::Private
        {
            std::shared_ptr<dtk::Settings> settings;
        };

        void TimeUnitsModel::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Settings>& settings)
        {
            timeline::TimeUnitsModel::_init(context);
            DTK_P();

            p.settings = settings;

            timeline::TimeUnits units = timeline::TimeUnits::Timecode;
            std::string s = timeline::to_string(units);
            p.settings->get("/TimeUnits", s);
            timeline::from_string(s, units);
            setTimeUnits(units);
        }

        TimeUnitsModel::TimeUnitsModel() :
            _p(new Private)
        {}

        TimeUnitsModel::~TimeUnitsModel()
        {
            DTK_P();
            p.settings->set("/TimeUnits", timeline::to_string(getTimeUnits()));
        }

        std::shared_ptr<TimeUnitsModel> TimeUnitsModel::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Settings>& settings)
        {
            auto out = std::shared_ptr<TimeUnitsModel>(new TimeUnitsModel);
            out->_init(context, settings);
            return out;
        }
    }
}
