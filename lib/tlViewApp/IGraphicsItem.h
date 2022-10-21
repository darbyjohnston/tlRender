// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include <list>

namespace tl
{
    namespace view
    {
        const imaging::FontInfo itemTitleFontInfo = imaging::FontInfo("NotoSans-Regular", 14);
        const imaging::FontInfo itemSmallFontInfo = imaging::FontInfo("NotoSans-Regular", 11);

        const double secondsSize = 100.0;

        const double sceneMargin = 10.0;
        const double sceneSpacing = 10.0;

        const double itemBorder = 1.0;
        const double itemMargin = 5.0;

        //! Base class for graphics items.
        class IGraphicsItem : public std::enable_shared_from_this<IGraphicsItem>
        {
            TLRENDER_NON_COPYABLE(IGraphicsItem);

        protected:
            void _init(const std::shared_ptr<IGraphicsItem>& parent = nullptr);

            IGraphicsItem();

        public:
            virtual ~IGraphicsItem() = 0;

            const std::weak_ptr<IGraphicsItem>& getParent() const;

            std::list<std::shared_ptr<IGraphicsItem> > getChildren() const;

            const otime::RationalTime& getDuration() const;

            virtual math::Vector2i getSize(
                const std::shared_ptr<imaging::FontSystem>&) const = 0;

            virtual void draw(
                const math::BBox2i&,
                const std::shared_ptr<imaging::FontSystem>&,
                const std::shared_ptr<timeline::IRender>&) = 0;

        protected:
            std::weak_ptr<IGraphicsItem> _parent;
            std::list<std::shared_ptr<IGraphicsItem> > _children;
            std::string _type;
            std::string _name;
            otime::RationalTime _duration;
            std::string _trimmedRange;
            std::string _sourceRange;
        };
    }
}
