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

            //! Set the scroll size callback.
            void setScrollSizeCallback(const std::function<void(const math::Vector2i&)>&);

            //! Get the scroll position.
            const math::Vector2i& getScrollPos() const;

            //! Set the scroll position.
            void setScrollPos(const math::Vector2i&);

            //! Set the scroll position callback.
            void setScrollPosCallback(const std::function<void(const math::Vector2i&)>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
