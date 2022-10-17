// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlViewApp/TimelineScene.h>

#include <tlViewApp/ClipItem.h>
#include <tlViewApp/GapItem.h>
#include <tlViewApp/StackItem.h>
#include <tlViewApp/TimelineItem.h>
#include <tlViewApp/TrackItem.h>
#include <tlViewApp/TransitionItem.h>

namespace tl
{
    namespace view
    {
        std::shared_ptr<TimelineItem> createScene(otio::Timeline* timeline)
        {
            auto timelineItem = TimelineItem::create(timeline);

            auto stackItem = StackItem::create(timeline->tracks(), timelineItem);

            for (auto i : timeline->tracks()->children())
            {
                if (auto track = dynamic_cast<otio::Track*>(i.value))
                {
                    auto trackItem = TrackItem::create(track, stackItem);

                    for (auto j : track->children())
                    {
                        if (auto clip = dynamic_cast<otio::Clip*>(j.value))
                        {
                            auto clipItem = ClipItem::create(clip, trackItem);
                        }
                        else if (auto gap = dynamic_cast<otio::Gap*>(j.value))
                        {
                            auto gapItem = GapItem::create(gap, trackItem);
                        }
                        else if (auto transition = dynamic_cast<otio::Transition*>(j.value))
                        {
                            auto transitionItem = TransitionItem::create(transition, trackItem);
                        }
                    }
                }
            }

            return timelineItem;
        }

        void drawScene(
            const std::shared_ptr<TimelineItem>& timelineItem,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<timeline::IRender>& render)
        {
            math::Vector2i pos(sceneMargin, sceneMargin);

            const math::Vector2i& timelineSize = timelineItem->getSize(fontSystem);
            const math::BBox2i timelineRect(pos.x, pos.y, timelineSize.x, timelineSize.y);
            timelineItem->draw(timelineRect, fontSystem, render);

            pos.y += timelineRect.h() + sceneSpacing;

            const auto& timelineChildren = timelineItem->getChildren();
            if (!timelineChildren.empty())
            {
                const auto& stackItem = timelineChildren.front();
                const math::Vector2i stackSize = stackItem->getSize(fontSystem);
                const math::BBox2i stackRect(pos.x, pos.y, stackSize.x, stackSize.y);
                stackItem->draw(stackRect, fontSystem, render);

                pos.y += stackRect.h() + sceneSpacing;

                for (const auto& trackItem : stackItem->getChildren())
                {
                    const math::Vector2i trackSize = trackItem->getSize(fontSystem);
                    const math::BBox2i trackRect(pos.x, pos.y, trackSize.x, trackSize.y);
                    trackItem->draw(trackRect, fontSystem, render);

                    pos.y += trackRect.h() + sceneSpacing;

                    math::Vector2i itemPos = pos;
                    int itemHeight = 0;
                    for (const auto& item : trackItem->getChildren())
                    {
                        const math::Vector2i& itemSize = item->getSize(fontSystem);
                        itemHeight = std::max(itemHeight, itemSize.y);
                        const math::BBox2i itemRect(itemPos.x, itemPos.y, itemSize.x, itemSize.y);
                        item->draw(itemRect, fontSystem, render);

                        itemPos.x += itemRect.w();
                    }

                    pos.y += itemHeight + sceneSpacing;
                }
            }
        }
    }
}
