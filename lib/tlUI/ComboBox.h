// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Combo box item.
        struct ComboBoxItem
        {
            std::string text;
            std::string icon;

            bool operator == (const ComboBoxItem&) const;
            bool operator != (const ComboBoxItem&) const;
        };

        //! Combo box.
        class ComboBox : public IWidget
        {
            TLRENDER_NON_COPYABLE(ComboBox);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            ComboBox();

        public:
            ~ComboBox() override;

            //! Create a new widget.
            static std::shared_ptr<ComboBox> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the items.
            void setItems(const std::vector<ComboBoxItem>&);

            //! Set the items.
            void setItems(const std::vector<std::string>&);

            //! Set the current index.
            void setCurrentIndex(int);

            //! Set the current index callback.
            void setIndexCallback(const std::function<void(int)>&);

            //! Set the current item callback.
            void setItemCallback(const std::function<void(const ComboBoxItem&)>&);

            //! Set the font role.
            void setFontRole(FontRole);

            void setVisible(bool) override;
            void setEnabled(bool) override;
            void tickEvent(
                bool,
                bool,
                const TickEvent&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            ComboBoxItem _getItem(int) const;

            void _click();
            void _resetMouse();

            void _commitIndex(int);

            TLRENDER_PRIVATE();
        };
    }
}
