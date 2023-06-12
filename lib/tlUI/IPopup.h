// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Base class for popup widgets.
        class IPopup : public IWidget
        {
            TLRENDER_NON_COPYABLE(IPopup);

        protected:
            void _init(
                const std::string& name,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IPopup();

        public:
            ~IPopup() override = 0;

            //! Open the popup the widget.
            void open(
                const std::shared_ptr<EventLoop>&,
                const math::BBox2i& buttonGeometry);

            //! Close the popup widget.
            void close();

            //! Set the close callback.
            void setCloseCallback(const std::function<void(void)>&);

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
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
