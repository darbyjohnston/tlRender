// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
        //! 
        //! \todo What happens when an empty combo box is clicked?
        class ComboBox : public IWidget
        {
            DTK_NON_COPYABLE(ComboBox);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ComboBox();

        public:
            virtual ~ComboBox();

            //! Create a new widget.
            static std::shared_ptr<ComboBox> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            static std::shared_ptr<ComboBox> create(
                const std::vector<std::string>&,
                const std::shared_ptr<dtk::Context>&,
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

            void tickEvent(
                bool,
                bool,
                const TickEvent&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const DrawEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            ComboBoxItem _getItem(int) const;

            void _click();

            void _commitIndex(int);

            DTK_PRIVATE();
        };
    }
}
