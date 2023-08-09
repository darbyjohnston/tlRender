// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/Edit.h>

namespace tl
{
    namespace timelineui
    {
        namespace
        {
            int getChildIndex(const otio::Item* item)
            {
                int out = -1;
                if (item && item->parent())
                {
                    const auto& children = item->parent()->children();
                    for (int i = 0; i < children.size(); ++i)
                    {
                        if (item == children[i].value)
                        {
                            out = i;
                            break;
                        }
                    }
                }
                return out;
            }
        }

        otio::SerializableObject::Retainer<otio::Timeline> insert(
            const otio::Timeline* timeline,
            const otio::Item* item,
            int trackIndex,
            int insertIndex)
        {
            const std::string s = timeline->to_json_string();
            otio::SerializableObject::Retainer<otio::Timeline> out(
                dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_string(s)));

            auto tracks = out->tracks()->children();
            const int itemTrackIndex = getChildIndex(item->parent());
            if (itemTrackIndex >= 0 && itemTrackIndex < tracks.size())
            {
                if (auto track = dynamic_cast<otio::Track*>(tracks[itemTrackIndex].value))
                {
                    auto children = track->children();
                    const int itemIndex = getChildIndex(item);
                    if (itemIndex >= 0 && itemIndex < children.size())
                    {
                        auto newItem = children[itemIndex];
                        track->remove_child(itemIndex);
                        if (trackIndex >= 0 && trackIndex < tracks.size())
                        {
                            if (auto track = dynamic_cast<otio::Track*>(tracks[trackIndex].value))
                            {
                                track->insert_child(insertIndex, newItem);
                            }
                        }
                    }
                }
            }

            return out;
        }
    }
}
