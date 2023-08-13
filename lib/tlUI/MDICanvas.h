// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/MDIWidget.h>

namespace tl
{
    namespace ui
    {
        //! MDI canvas.
        class MDICanvas : public IWidget
        {
            TLRENDER_NON_COPYABLE(MDICanvas);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            MDICanvas();

        public:
            virtual ~MDICanvas();

            //! Create a new widget.
            static std::shared_ptr<MDICanvas> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Add a widget to the canvas.
            std::shared_ptr<MDIWidget> addWidget(
                const std::string& title,
                const std::shared_ptr<IWidget>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
