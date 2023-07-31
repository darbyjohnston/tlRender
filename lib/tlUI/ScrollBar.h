// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Scroll bar.
        //! 
        //! \todo Handle clicks not on the scroll bar handle.
        //! \todo Set a minimum scroll bar handle size.
        class ScrollBar : public IWidget
        {
            TLRENDER_NON_COPYABLE(ScrollBar);

        protected:
            void _init(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ScrollBar();

        public:
            ~ScrollBar() override;

            //! Create a new widget.
            static std::shared_ptr<ScrollBar> create(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the scroll size.
            void setScrollSize(int);

            //! Get the scroll position.
            int getScrollPos() const;

            //! Set the scroll position.
            void setScrollPos(int);

            //! Set the scroll position callback.
            void setScrollPosCallback(const std::function<void(int)>&);

            void setVisible(bool) override;
            void setEnabled(bool) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            math::BBox2i _getBorderGeometry() const;
            math::BBox2i _getHandleGeometry() const;
            int _getScrollPosMax() const;
            float _getScrollScale() const;

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
