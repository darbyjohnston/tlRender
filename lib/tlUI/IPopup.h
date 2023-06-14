// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Popup style.
        enum class PopupStyle
        {
            Menu,
            SubMenu
        };

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

            //! Open the popup widget.
            void open(
                const std::shared_ptr<EventLoop>&,
                const math::BBox2i& buttonGeometry);

            //! Is the popup widget open.
            bool isOpen() const;

            //! Close the popup widget.
            virtual void close();

            //! Set the close callback.
            void setCloseCallback(const std::function<void(void)>&);

            //! Set the popup style.
            void setPopupStyle(PopupStyle);

            //! Set the popup color role.
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
