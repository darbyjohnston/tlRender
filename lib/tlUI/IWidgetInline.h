// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace ui
    {
        inline const std::string& IWidget::getName() const
        {
            return _name;
        }

        inline int IWidget::getUpdates() const
        {
            return _updates;
        }

        inline const std::weak_ptr<IWidget>& IWidget::getParent() const
        {
            return _parent;
        }

        inline const std::list<std::shared_ptr<IWidget> >& IWidget::getChildren() const
        {
            return _children;
        }

        inline const std::weak_ptr<EventLoop>& IWidget::getEventLoop()
        {
            return getTopLevel()->_eventLoop;
        }

        inline const math::Vector2i& IWidget::getSizeHint() const
        {
            return _sizeHint;
        }

        inline Stretch IWidget::getHStretch() const
        {
            return _hStretch;
        }

        inline Stretch IWidget::getVStretch() const
        {
            return _vStretch;
        }

        inline HAlign IWidget::getHAlign() const
        {
            return _hAlign;
        }

        inline VAlign IWidget::getVAlign() const
        {
            return _vAlign;
        }

        inline const math::BBox2i& IWidget::getGeometry() const
        {
            return _geometry;
        }

        inline bool IWidget::isVisible() const
        {
            return _visible;
        }

        inline bool IWidget::isClipped() const
        {
            return _clipped;
        }

        inline math::BBox2i IWidget::getChildrenClipRect() const
        {
            return _geometry;
        }

        inline bool IWidget::isEnabled() const
        {
            return _enabled;
        }

        inline bool IWidget::acceptsKeyFocus() const
        {
            return false;
        }
        
        inline bool IWidget::hasKeyFocus() const
        {
            return _keyFocus;
        }
    }
}
