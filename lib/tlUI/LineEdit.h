// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Text line edit.
        //! 
        //! \todo Scroll the view with the cursor.
        //! \todo Double click to select text.
        class LineEdit : public IWidget
        {
            TLRENDER_NON_COPYABLE(LineEdit);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            LineEdit();

        public:
            virtual ~LineEdit();

            //! Create a new widget
            static std::shared_ptr<LineEdit> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the text.
            const std::string& getText() const;

            //! Set the text.
            void setText(const std::string&);

            //! Clear the text.
            void clearText();

            //! Set the text callback.
            void setTextCallback(const std::function<void(const std::string&)>&);

            //! Set the text changed callback.
            void setTextChangedCallback(const std::function<void(const std::string&)>&);

            //! Set the formatting text.
            void setFormat(const std::string&);

            //! Set the lost focus callback.
            void setFocusCallback(const std::function<void(bool)>&);

            //! Set the font role.
            void setFontRole(FontRole);

            void setVisible(bool) override;
            void setEnabled(bool) override;
            void tickEvent(
                bool,
                bool,
                const TickEvent&) override;
            void clipEvent(
                const math::Box2i&,
                bool,
                const ClipEvent&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(
                const math::Box2i&,
                const DrawEvent&) override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void keyFocusEvent(bool) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;
            void textEvent(TextEvent&) override;

        private:
            math::Box2i _getAlignGeometry() const;

            int _getCursorPos(const math::Vector2i&);

            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
