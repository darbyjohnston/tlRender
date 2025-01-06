// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! MDI resize directions.
        enum class MDIResize
        {
            None,
            North,
            NorthEast,
            East,
            SouthEast,
            South,
            SouthWest,
            West,
            NorthWest,

            Count,
            First = North
        };
        TLRENDER_ENUM(MDIResize);
        TLRENDER_ENUM_SERIALIZE(MDIResize);

        //! MDI widget.
        class MDIWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(MDIWidget);

        protected:
            void _init(
                const std::string& title,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            MDIWidget();

        public:
            virtual ~MDIWidget();

            //! Create a new widget.
            static std::shared_ptr<MDIWidget> create(
                const std::string& title,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the title.
            void setTitle(const std::string&);

            //! Set the widget.
            void setWidget(const std::shared_ptr<IWidget>&);

            //! Set the press callback.
            void setPressCallback(const std::function<void(bool)>&);

            //! Set the move callback.
            void setMoveCallback(const std::function<void(const math::Vector2i&)>&);

            //! Set the resize callback.
            void setResizeCallback(const std::function<void(MDIResize, const math::Vector2i&)>&);

            //! Get the inside geometry.
            const math::Box2i& getInsideGeometry() const;

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const math::Box2i&, const DrawEvent&) override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
