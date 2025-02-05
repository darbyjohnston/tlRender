// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
            DTK_NON_COPYABLE(ScrollBar);

        protected:
            void _init(
                Orientation,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ScrollBar();

        public:
            virtual ~ScrollBar();

            //! Create a new widget.
            static std::shared_ptr<ScrollBar> create(
                Orientation,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the scroll size.
            void setScrollSize(int);

            //! Get the scroll position.
            int getScrollPos() const;

            //! Set the scroll position.
            void setScrollPos(int);

            //! Set the scroll position callback.
            void setScrollPosCallback(const std::function<void(int)>&);

            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const DrawEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            dtk::Box2I _getBorderGeometry() const;
            dtk::Box2I _getHandleGeometry() const;
            int _getScrollPosMax() const;
            float _getScrollScale() const;

            DTK_PRIVATE();
        };
    }
}
