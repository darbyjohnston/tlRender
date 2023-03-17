// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Style.h>

#include <tlTimeline/IRender.h>

#include <tlCore/FontSystem.h>

#include <list>

namespace tl
{
    namespace ui
    {
        //! Size hint data.
        struct SizeHintData
        {
            std::shared_ptr<Style> style;
            float contentScale = 1.F;
            std::shared_ptr<imaging::FontSystem> fontSystem;
        };

        //! Drawing data.
        struct DrawData
        {
            math::BBox2i bbox;
            std::shared_ptr<Style> style;
            float contentScale = 1.F;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            std::shared_ptr<timeline::IRender> render;
        };

        //! Base class for widgets.
        class IWidget : public std::enable_shared_from_this<IWidget>
        {
            TLRENDER_NON_COPYABLE(IWidget);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IWidget();

        public:
            virtual ~IWidget() = 0;

            //! Set the parent widget.
            void setParent(const std::shared_ptr<IWidget>&);

            //! Get the children widgets.
            const std::list<std::shared_ptr<IWidget> >& getChildren() const;

            //! Calculate the size hint.
            virtual void sizeHint(const SizeHintData&);

            //! Get the size hint.
            const math::Vector2i& getSizeHint() const;

            //! Get the geometry.
            const math::BBox2i& getGeometry() const;

            //! Set the geometry.
            virtual void setGeometry(const math::BBox2i&);
 
            //! Draw the widget.
            virtual void draw(const DrawData&);

        protected:
            std::weak_ptr<system::Context> _context;
            std::weak_ptr<IWidget> _parent;
            std::list<std::shared_ptr<IWidget> > _children;
            math::Vector2i _sizeHint;
            math::BBox2i _geometry;
        };
    }
}
