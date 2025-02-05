// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Splitter();

        public:
            virtual ~Splitter();

            //! Create a new widget.
            static std::shared_ptr<Splitter> create(
                Orientation,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the split amount.
            float getSplit() const;

            //! Set the split amount.
            void setSplit(float);

            //! Set the spacing role.
            void setSpacingRole(SizeRole);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const DrawEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        protected:
            void _releaseMouse() override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
