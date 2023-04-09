// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace ui
    {
        //! Scroll area type.
        enum class ScrollAreaType
        {
            Horizontal,
            Vertical,
            Both
        };

        //! Scroll area.
        class ScrollArea : public IWidget
        {
            TLRENDER_NON_COPYABLE(ScrollArea);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                ScrollAreaType,
                const std::shared_ptr<IWidget>& parent = nullptr);

            ScrollArea();

        public:
            ~ScrollArea() override;

            //! Create a new scroll area.
            static std::shared_ptr<ScrollArea> create(
                const std::shared_ptr<system::Context>&,
                ScrollAreaType = ScrollAreaType::Both,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Observe the scroll size.
            std::shared_ptr<observer::IValue<math::Vector2i> > observeScrollSize() const;

            //! Set the scroll position.
            void setScrollPos(const math::Vector2i&);

            //! Observe the scroll position.
            std::shared_ptr<observer::IValue<math::Vector2i> > observeScrollPos() const;

            //! Set whether the scroll area has a border.
            void setBorder(bool);

            void setGeometry(const math::BBox2i&) override;
            void sizeEvent(const SizeEvent&) override;
            void drawEvent(const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
