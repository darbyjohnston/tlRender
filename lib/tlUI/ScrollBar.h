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
        class ScrollBar : public IWidget
        {
            TLRENDER_NON_COPYABLE(ScrollBar);

        protected:
            void _init(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            ScrollBar();

        public:
            ~ScrollBar() override;

            //! Create a new scroll bar.
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
            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            int _getScrollPosMax() const;
            float _getScrollScale() const;
            math::BBox2i _getHandleGeometry() const;

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
