// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IPopup.h>

namespace tl
{
    namespace ui
    {
        //! Popup style.
        enum class MenuPopupStyle
        {
            Menu,
            SubMenu
        };

        //! Base class for menu popup widgets.
        class IMenuPopup : public IPopup
        {
            TLRENDER_NON_COPYABLE(IMenuPopup);

        protected:
            void _init(
                const std::string& name,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IMenuPopup();

        public:
            ~IMenuPopup() override = 0;

            //! Open the menu popup.
            void open(
                const std::shared_ptr<EventLoop>&,
                const math::BBox2i& buttonGeometry);

            //! Get whether the menu popup is open.
            bool isOpen() const;

            //! Close the menu popup.
            void close() override;

            //! Set the close callback.
            void setCloseCallback(const std::function<void(void)>&);

            //! Set the menu popup style.
            void setPopupStyle(MenuPopupStyle);

            //! Set the menu popup color role.
            void setPopupRole(ColorRole);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
