// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
            DTK_NON_COPYABLE(LineEdit);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            LineEdit();

        public:
            virtual ~LineEdit();

            //! Create a new widget
            static std::shared_ptr<LineEdit> create(
                const std::shared_ptr<dtk::Context>&,
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
            void clipEvent(const dtk::Box2I&, bool) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const DrawEvent&) override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void keyFocusEvent(bool) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;
            void textEvent(TextEvent&) override;

        private:
            dtk::Box2I _getAlignGeometry() const;

            int _getCursorPos(const dtk::V2I&);

            void _textUpdate();

            DTK_PRIVATE();
        };
    }
}
