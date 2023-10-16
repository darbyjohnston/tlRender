// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/Edit.h>

namespace tl
{
    namespace timeline
    {
        namespace
        {
            int getIndex(const otio::SerializableObject::Retainer<otio::Composable>& composable)
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

            otio::SerializableObject::Retainer<otio::Composable> getPrevSibling(
                const otio::SerializableObject::Retainer<otio::Composable>& value)
            {
                otio::SerializableObject::Retainer<otio::Composable> out;
                if (auto parent = value->parent())
                {
                    const auto& children = parent->children();
                    for (size_t i = 1; i < children.size(); ++i)
                    {
                        if (children[i].value == value.value)
                        {
                            out = children[i - 1];
                            break;
                        }
                    }
                }
                return out;
            }

            otio::SerializableObject::Retainer<otio::Composable> getNextSibling(
                const otio::SerializableObject::Retainer<otio::Composable>& value)
            {
                otio::SerializableObject::Retainer<otio::Composable> out;
                if (auto parent = value->parent())
                {
                    const auto& children = parent->children();
                    if (!children.empty())
                    {
                        for (size_t i = 0; i < children.size() - 1; ++i)
                        {
                            if (children[i].value == value.value)
                            {
                                out = children[i + 1];
                                break;
                            }
                        }
                    }
                }
                return out;
            }
        }

        otio::SerializableObject::Retainer<otio::Timeline> move(
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline,
            const std::vector<MoveData>& moves)
        {
            const std::string s = timeline->to_json_string();
            otio::SerializableObject::Retainer<otio::Timeline> out(
                dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_string(s)));

            for (const auto& move : moves)
            {
                if (move.fromTrack >= 0 &&
                    move.fromTrack < out->tracks()->children().size() &&
                    move.toTrack >= 0 &&
                    move.toTrack < out->tracks()->children().size())
                {
                    int toIndex = move.toIndex;
                    if (move.fromTrack == move.toTrack && move.fromIndex < toIndex)
                    {
                        --toIndex;
                    }
                    if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                        out->tracks()->children()[move.fromTrack]))
                    {
                        auto child = track->children()[move.fromIndex];
                        track->remove_child(move.fromIndex);

                        if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                            out->tracks()->children()[move.toTrack]))
                        {
                            track->insert_child(toIndex, child);
                        }
                    }
                }
            }

            return out;
        }
    }
}
