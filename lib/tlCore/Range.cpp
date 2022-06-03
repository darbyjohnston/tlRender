// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/Range.h>

namespace tl
{
    namespace math
    {
        void to_json(nlohmann::json& json, const IntRange& value)
        {
            json = { value.getMin(), value.getMax() };
        }

        void to_json(nlohmann::json& json, const SizeTRange& value)
        {
            json = { value.getMin(), value.getMax() };
        }

        void to_json(nlohmann::json& json, const FloatRange& value)
        {
            json = { value.getMin(), value.getMax() };
        }

        void from_json(const nlohmann::json& json, IntRange& value)
        {
            int min = 0;
            int max = 0;
            json.at(0).get_to(min);
            json.at(1).get_to(max);
            value = IntRange(min, max);
        }

        void from_json(const nlohmann::json& json, SizeTRange& value)
        {
            size_t min = 0;
            size_t max = 0;
            json.at(0).get_to(min);
            json.at(1).get_to(max);
            value = SizeTRange(min, max);
        }

        void from_json(const nlohmann::json& json, FloatRange& value)
        {
            float min = 0.F;
            float max = 0.F;
            json.at(0).get_to(min);
            json.at(1).get_to(max);
            value = FloatRange(min, max);
        }
    }
}