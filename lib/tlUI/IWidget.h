// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Event.h>

#include <list>

namespace tl
{
    namespace ui
    {
        //! Orientation.
        enum class Orientation
        {
            Horizontal,
            Vertical
        };

        //! Layout stretch.
        enum class Stretch
        {
            Fixed,
            Expanding
        };

        //! Base class for widgets.
        class IWidget : public std::enable_shared_from_this<IWidget>
        {
            TLRENDER_NON_COPYABLE(IWidget);

        protected:
            void _init(
                const std::string& name,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IWidget();

        public:
            virtual ~IWidget() = 0;

            //! Get the widget name.
            const std::string& getName() const;

            //! Set the widget name.
            void setName(const std::string&);

            //! Set the parent widget.
            void setParent(const std::shared_ptr<IWidget>&);

            //! Get the children widgets.
            const std::list<std::shared_ptr<IWidget> >& getChildren() const;

            //! Get the size hint.
            const math::Vector2i& getSizeHint() const;

            //! Get the layout stretch.
            Stretch getStretch(Orientation) const;

            //! Set the layout stretch.
            void setStretch(Stretch, Orientation);

            //! Get the geometry.
            const math::BBox2i& getGeometry() const;

            //! Set the geometry.
            virtual void setGeometry(const math::BBox2i&);
 
            //! Size hint event.
            virtual void sizeHintEvent(const SizeHintEvent&);

            //! Draw event.
            virtual void drawEvent(const DrawEvent&);

            //! Enter event.
            virtual void enterEvent();

            //! Leave event.
            virtual void leaveEvent();

            //! Mouse move event.
            virtual void mouseMoveEvent(const MouseMoveEvent&);

            //! Mouse press event.
            virtual void mousePressEvent(const MouseClickEvent&);

            //! Mouse release event.
            virtual void mouseReleaseEvent(const MouseClickEvent&);

            //! Key press event.
            virtual void keyPressEvent(const KeyEvent&);

            //! Key release event.
            virtual void keyReleaseEvent(const KeyEvent&);

        protected:
            std::weak_ptr<system::Context> _context;
            std::string _name;
            std::weak_ptr<IWidget> _parent;
            std::list<std::shared_ptr<IWidget> > _children;
            math::Vector2i _sizeHint;
            std::pair<Stretch, Stretch> _stretch = { Stretch::Fixed, Stretch::Fixed };
            math::BBox2i _geometry;
        };
    }
}
