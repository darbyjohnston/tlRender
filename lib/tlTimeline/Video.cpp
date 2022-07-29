// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/Video.h>

#include <tlTimeline/IRender.h>

#include <tlCore/Error.h>
#include <tlCore/FontSystem.h>
#include <tlCore/String.h>

#include <opentimelineio/transition.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            Transition,
            "None",
            "Dissolve");
        TLRENDER_ENUM_SERIALIZE_IMPL(Transition);

        Transition toTransition(const std::string& value)
        {
            Transition out = Transition::None;
            if (otio::Transition::Type::SMPTE_Dissolve == value)
            {
                out = Transition::Dissolve;
            }
            return out;
        }
    }
}