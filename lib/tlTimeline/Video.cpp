// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/Video.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <opentimelineio/transition.h>

#include <array>

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

        bool VideoLayer::operator == (const VideoLayer& other) const
        {
            return image == other.image &&
                imageB == other.imageB &&
                transition == other.transition &&
                transitionValue == other.transitionValue;
        }

        bool VideoLayer::operator != (const VideoLayer& other) const
        {
            return !(*this == other);
        }

        bool VideoData::operator == (const VideoData& other) const
        {
            return time == other.time &&
                layers == other.layers;
        }

        bool VideoData::operator != (const VideoData& other) const
        {
            return !(*this == other);
        }
    }
}