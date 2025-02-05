// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IPopup.h>

namespace tl
{
    namespace ui
    {
        class IWindow;

        //! Base class for popup widgets.
        class IWidgetPopup : public IPopup
        {
            DTK_NON_COPYABLE(IWidgetPopup);

        protected:
            void _init(
                const std::string& objectName,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IWidgetPopup();

        public:
            virtual ~IWidgetPopup() = 0;

            //! Open the popup.
            void open(
                const std::shared_ptr<IWindow>&,
                const dtk::Box2I& buttonGeometry);

            //! Get whether the popup is open.
            bool isOpen() const;

            //! Close the popup.
            void close() override;

            //! Set the close callback.
            void setCloseCallback(const std::function<void(void)>&);

            //! Set the popup color role.
            void setPopupRole(ColorRole);

            //! Set the widget.
            void setWidget(const std::shared_ptr<IWidget>&);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const DrawEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
