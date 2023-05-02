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
        //! Scroll type.
        enum class ScrollType
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
                ScrollType,
                const std::shared_ptr<IWidget>& parent = nullptr);

            ScrollArea();

        public:
            ~ScrollArea() override;

            //! Create a new scroll area.
            static std::shared_ptr<ScrollArea> create(
                const std::shared_ptr<system::Context>&,
                ScrollType = ScrollType::Both,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the scroll size.
            const math::Vector2i& getScrollSize() const;

            //! Observe the scroll size.
            std::shared_ptr<observer::IValue<math::Vector2i> > observeScrollSize() const;

            //! Get the scroll position.
            const math::Vector2i& getScrollPos() const;

            //! Observe the scroll position.
            std::shared_ptr<observer::IValue<math::Vector2i> > observeScrollPos() const;

            //! Set the scroll position.
            void setScrollPos(const math::Vector2i&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
