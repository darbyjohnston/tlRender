// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Splitter widget.
        class Splitter : public IWidget
        {
            TLRENDER_NON_COPYABLE(Splitter);

        protected:
            void _init(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            Splitter();

        public:
            ~Splitter() override;

            //! Create a new splitter widget.
            static std::shared_ptr<Splitter> create(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the split amount.
            void setSplit(float);

            void setGeometry(const math::BBox2i&) override;
            void setVisible(bool) override;
            void setEnabled(bool) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(bool, const ClipEvent&) override;
            void drawEvent(const DrawEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
