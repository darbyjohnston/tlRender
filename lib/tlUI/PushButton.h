// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace ui
    {
        //! Push button.
        class PushButton : public IWidget
        {
            TLRENDER_NON_COPYABLE(PushButton);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            PushButton();

        public:
            ~PushButton() override;

            //! Create a new push button.
            static std::shared_ptr<PushButton> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set whether the button is checkable.
            void setCheckable(bool);

            //! Set whether the button is checked.
            void setChecked(bool);

            //! Set the text.
            void setText(const std::string&);

            //! Set the font information.
            void setFontInfo(const imaging::FontInfo&);

            //! Set the icon.
            void setIcon(const std::string&);
            
            //! Set whether the button has a flat appearance.
            void setFlat(bool);

            //! Observe button clicks.
            std::shared_ptr<observer::IValue<bool> > observeClick() const;

            //! Observe the checked state.
            std::shared_ptr<observer::IValue<bool> > observeChecked() const;

            void sizeEvent(const SizeEvent&) override;
            void drawEvent(const DrawEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(const MouseMoveEvent&) override;
            void mousePressEvent(const MouseClickEvent&) override;
            void mouseReleaseEvent(const MouseClickEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
