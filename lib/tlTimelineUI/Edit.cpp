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
            int getIndex(const otio::Composable* composable)
            {
                int out = -1;
                if (composable && composable->parent())
                {
                    const auto& children = composable->parent()->children();
                    for (int i = 0; i < children.size(); ++i)
                    {
                        if (composable == children[i].value)
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
            const otio::Composable* composable,
            int trackIndex,
            int insertIndex)
        {
            const std::string s = timeline->to_json_string();
            otio::SerializableObject::Retainer<otio::Timeline> out(
                dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_string(s)));

            const int oldIndex = getIndex(composable);
            const int oldTrackIndex = getIndex(composable->parent());
            if (oldIndex != -1 &&
                oldTrackIndex != -1 &&
                trackIndex >= 0 &&
                trackIndex < out->tracks()->children().size())
            {
                if (oldTrackIndex == trackIndex && oldIndex < insertIndex)
                {
                    --insertIndex;
                }
                if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                    out->tracks()->children()[oldTrackIndex]))
                {
                    auto child = track->children()[oldIndex];
                    track->remove_child(oldIndex);

                    if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                        out->tracks()->children()[trackIndex]))
                    {
                        track->insert_child(insertIndex, child);
                    }
                }
            }

            return out;
        }
    }
}
