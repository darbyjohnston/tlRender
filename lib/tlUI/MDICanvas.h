// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/MDIWidget.h>

namespace tl
{
    namespace ui
    {
        //! MDI canvas.
        //!
        //! \todo Add support for maximizing MDI widgets.
        class MDICanvas : public IWidget
        {
            DTK_NON_COPYABLE(MDICanvas);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            MDICanvas();

        public:
            virtual ~MDICanvas();

            //! Create a new widget.
            static std::shared_ptr<MDICanvas> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Add a widget to the canvas.
            std::shared_ptr<MDIWidget> addWidget(
                const std::string& title,
                const std::shared_ptr<IWidget>&);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
