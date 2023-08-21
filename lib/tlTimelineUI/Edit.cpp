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

        otio::SerializableObject::Retainer<otio::Clip> getAssociatedClip(
            const otio::SerializableObject::Retainer<otio::Clip>& clip)
        {
            otio::SerializableObject::Retainer<otio::Clip> out;
            const auto timeRangeOpt = clip->trimmed_range_in_parent();
            if (timeRangeOpt.has_value())
            {
                const otime::TimeRange timeRange = timeRangeOpt.value();
                if (auto track = dynamic_cast<otio::Track*>(clip->parent()))
                {
                    if (otio::Track::Kind::video == track->kind())
                    {
                        if (auto nextTrack = otio::dynamic_retainer_cast<otio::Track>(getNextSibling(track)))
                        {
                            if (otio::Track::Kind::audio == nextTrack->kind())
                            {
                                for (const auto& child : nextTrack->children())
                                {
                                    if (auto audioClip = otio::dynamic_retainer_cast<otio::Clip>(child))
                                    {
                                        const auto audioTimeRangeOpt = audioClip->trimmed_range_in_parent();
                                        if (audioTimeRangeOpt.has_value())
                                        {
                                            const otime::TimeRange audioTimeRange = audioTimeRangeOpt.value();
                                            if (audioTimeRange.start_time() == timeRange.start_time())
                                            {
                                                out = audioClip;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (otio::Track::Kind::audio == track->kind())
                    {
                        if (auto prevTrack = otio::dynamic_retainer_cast<otio::Track>(getPrevSibling(track)))
                        {
                            if (otio::Track::Kind::video == prevTrack->kind())
                            {
                                for (const auto& child : prevTrack->children())
                                {
                                    if (auto videoClip = otio::dynamic_retainer_cast<otio::Clip>(child))
                                    {
                                        const auto videoTimeRangeOpt = videoClip->trimmed_range_in_parent();
                                        if (videoTimeRangeOpt.has_value())
                                        {
                                            const otime::TimeRange videoTimeRange = videoTimeRangeOpt.value();
                                            if (videoTimeRange.start_time() == timeRange.start_time())
                                            {
                                                out = videoClip;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return out;
        }

        otio::SerializableObject::Retainer<otio::Timeline> insert(
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline,
            const std::vector<InsertData>& inserts)
        {
            const std::string s = timeline->to_json_string();
            otio::SerializableObject::Retainer<otio::Timeline> out(
                dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_string(s)));

            for (const auto& insert : inserts)
            {
                const int oldIndex = getIndex(insert.composable);
                const int oldTrackIndex = getIndex(insert.composable->parent());
                if (oldIndex != -1 &&
                    oldTrackIndex != -1 &&
                    insert.trackIndex >= 0 &&
                    insert.trackIndex < out->tracks()->children().size())
                {
                    int insertIndex = insert.insertIndex;
                    if (oldTrackIndex == insert.trackIndex && oldIndex < insertIndex)
                    {
                        --insertIndex;
                    }
                    if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                        out->tracks()->children()[oldTrackIndex]))
                    {
                        auto child = track->children()[oldIndex];
                        track->remove_child(oldIndex);

                        if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                            out->tracks()->children()[insert.trackIndex]))
                        {
                            track->insert_child(insertIndex, child);
                        }
                    }
                }
            }

            return out;
        }
    }
}
