// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Event.h>
#include <tlUI/IWidgetOptions.h>

#include <list>

namespace tl
{
    namespace ui
    {
        class EventLoop;

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

            //! Set the background role.
            void setBackgroundRole(ColorRole);

            //! Get whether updates are needed.
            int getUpdates() const;

            //! Hierarchy
            ///@{

            //! Get the parent widget.
            const std::weak_ptr<IWidget>& getParent() const;

            //! Set the parent widget.
            void setParent(const std::shared_ptr<IWidget>&);

            //! Get the children widgets.
            const std::list<std::shared_ptr<IWidget> >& getChildren() const;

            //! Get the top level widget.
            std::shared_ptr<IWidget> getTopLevel() const;

            //! Set the event loop.
            void setEventLoop(const std::weak_ptr<EventLoop>&);

            //! Get the event loop.
            const std::weak_ptr<EventLoop>& getEventLoop() const;

            ///@}

            //! Geometry
            ///@{

            //! Get the size hint.
            const math::Vector2i& getSizeHint() const;

            //! Get the horizontal layout stretch.
            Stretch getHStretch() const;

            //! Set the horizontal layout stretch.
            void setHStretch(Stretch);

            //! Get the vertical layout stretch.
            Stretch getVStretch() const;

            //! Set the vertical layout stretch.
            void setVStretch(Stretch);

            //! Set the horizontal and vertical layout stretch.
            void setStretch(Stretch horizontal, Stretch vertical);

            //! Set the horizontal and vertical layout stretch.
            void setStretch(Stretch);

            //! Get the horizontal layout alignment.
            HAlign getHAlign() const;

            //! Set the horizontal layout alignment.
            void setHAlign(HAlign);

            //! Get the vertical layout alignment.
            VAlign getVAlign() const;

            //! Set the vertical layout alignment.
            void setVAlign(VAlign);

            //! Set the horizontal and vertical layout alignment.
            void setAlign(HAlign, VAlign);

            //! Get the geometry.
            const math::BBox2i& getGeometry() const;

            //! Set the geometry.
            virtual void setGeometry(const math::BBox2i&);

            ///@}

            //! Visibility
            ///@{

            //! Is the widget visible?
            bool isVisible() const;

            //! Set whether the widget is visible.
            virtual void setVisible(bool);

            //! Is the widget clipped?
            bool isClipped() const;

            //! Get the clipping rect applied to the child widgets. By
            //! default this is the same as the widget geometry.
            virtual math::BBox2i getChildrenClipRect() const;

            ///@}

            //! Enabled
            ///@{

            //! Is the widget enabled?
            bool isEnabled() const;

            //! Set whether the widget is enabled.
            virtual void setEnabled(bool);

            ///@}

            //! Key Focus
            ///@{

            //! Does this widget accept key focus?
            virtual bool acceptsKeyFocus() const;

            //! Does this widget have key focus?
            bool hasKeyFocus() const;

            //! Take the key focus.
            void takeFocus();

            //! Release the key focus.
            void releaseFocus();

            ///@}

            //! Events
            ///@{

            //! Child added event.
            virtual void childAddedEvent(const ChildEvent&);

            //! Child removed event.
            virtual void childRemovedEvent(const ChildEvent&);

            //! Tick event.
            virtual void tickEvent(const TickEvent&);

            //! Size hint event.
            virtual void sizeHintEvent(const SizeHintEvent&);

            //! Clip event.
            virtual void clipEvent(
                const math::BBox2i&,
                bool clipped,
                const ClipEvent&);

            //! Draw event.
            virtual void drawEvent(
                const math::BBox2i&,
                const DrawEvent&);

            //! Enter event.
            virtual void enterEvent();

            //! Leave event.
            virtual void leaveEvent();

            //! Mouse move event.
            virtual void mouseMoveEvent(MouseMoveEvent&);

            //! Mouse press event.
            virtual void mousePressEvent(MouseClickEvent&);

            //! Mouse release event.
            virtual void mouseReleaseEvent(MouseClickEvent&);

            //! Key press event.
            virtual void keyPressEvent(KeyEvent&);

            //! Key release event.
            virtual void keyReleaseEvent(KeyEvent&);

            //! Text event.
            virtual void textEvent(TextEvent&);

            ///@}

        protected:
            std::weak_ptr<system::Context> _context;
            std::string _name;
            std::weak_ptr<IWidget> _parent;
            std::weak_ptr<EventLoop> _eventLoop;
            std::list<std::shared_ptr<IWidget> > _children;
            math::Vector2i _sizeHint;
            Stretch _hStretch = Stretch::Fixed;
            Stretch _vStretch = Stretch::Fixed;
            HAlign _hAlign = HAlign::Center;
            VAlign _vAlign = VAlign::Center;
            math::BBox2i _geometry;
            bool _visible = true;
            bool _clipped = false;
            bool _enabled = true;
            ColorRole _backgroundRole = ColorRole::None;
            int _updates = 0;
        };
    }
}

#include <tlUI/IWidgetInline.h>
