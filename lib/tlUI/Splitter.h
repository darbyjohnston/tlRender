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
                const std::shared_ptr<IWidget>& parent);

            Splitter();

        public:
            virtual ~Splitter();

            //! Create a new widget.
            static std::shared_ptr<Splitter> create(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the split amount.
            float getSplit() const;

            //! Set the split amount.
            void setSplit(float);

            //! Set the spacing role.
            void setSpacingRole(SizeRole);

            void setGeometry(const math::Box2i&) override;
            void setVisible(bool) override;
            void setEnabled(bool) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::Box2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::Box2i&,
                const DrawEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
