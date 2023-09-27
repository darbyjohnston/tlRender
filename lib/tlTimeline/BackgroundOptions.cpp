// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/BackgroundOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            Background,
            "Solid",
            "Checkers");
        TLRENDER_ENUM_SERIALIZE_IMPL(Background);

        void to_json(nlohmann::json& json, const BackgroundOptions& in)
        {
            json = nlohmann::json
            {
                { "type", in.type },
                { "solidColor", in.solidColor },
                { "checkersColor0", in.checkersColor0 },
                { "checkersColor1", in.checkersColor1 },
                { "checkersSize", in.checkersSize },
            };
        }

        void from_json(const nlohmann::json& json, BackgroundOptions& out)
        {
            json.at("type").get_to(out.type);
            json.at("solidColor").get_to(out.solidColor);
            json.at("checkersColor0").get_to(out.checkersColor0);
            json.at("checkersColor1").get_to(out.checkersColor1);
            json.at("checkersSize").get_to(out.checkersSize);
        }
    }
}
