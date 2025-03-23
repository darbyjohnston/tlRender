// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerOptions.h>

namespace tl
{
    namespace timeline
    {
        bool PlayerCacheOptions::operator == (const PlayerCacheOptions& other) const
        {
            return
                videoGB == other.videoGB &&
                audioGB == other.audioGB &&
                readBehind == other.readBehind;
        }

        bool PlayerCacheOptions::operator != (const PlayerCacheOptions& other) const
        {
            return !(*this == other);
        }

        bool PlayerOptions::operator == (const PlayerOptions& other) const
        {
            return
                audioDevice == other.audioDevice &&
                cache == other.cache &&
                audioBufferFrameCount == other.audioBufferFrameCount &&
                muteTimeout == other.muteTimeout &&
                sleepTimeout == other.sleepTimeout &&
                currentTime == other.currentTime;
        }

        bool PlayerOptions::operator != (const PlayerOptions& other) const
        {
            return !(*this == other);
        }

        void to_json(nlohmann::json& json, const PlayerCacheOptions& value)
        {
            json["VideoGB"] = value.videoGB;
            json["AudioGB"] = value.audioGB;
            json["ReadBehind"] = value.readBehind;
        }

        void from_json(const nlohmann::json& json, PlayerCacheOptions& value)
        {
            json.at("VideoGB").get_to(value.videoGB);
            json.at("AudioGB").get_to(value.audioGB);
            json.at("ReadBehind").get_to(value.readBehind);
        }
    }
}
