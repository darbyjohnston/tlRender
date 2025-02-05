// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <dtk/core/ObservableValue.h>

namespace tl
{
    namespace ui
    {
        //! Scroll type.
        enum class ScrollType
        {
            Horizontal,
            Vertical,
            Both,
            Menu
        };

        //! Scroll area.
        class ScrollArea : public IWidget
        {
            DTK_NON_COPYABLE(ScrollArea);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                ScrollType,
                const std::shared_ptr<IWidget>& parent);

            ScrollArea();

        public:
            virtual ~ScrollArea();

            //! Create a new widget.
            static std::shared_ptr<ScrollArea> create(
                const std::shared_ptr<dtk::Context>&,
                ScrollType = ScrollType::Both,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the scroll size.
            const dtk::V2I& getScrollSize() const;

            //! Set the scroll size callback.
            void setScrollSizeCallback(const std::function<void(const dtk::V2I&)>&);

            //! Get the scroll position.
            const dtk::V2I& getScrollPos() const;

            //! Set the scroll position.
            void setScrollPos(const dtk::V2I&, bool clamp = true);

            //! Set the scroll position callback.
            void setScrollPosCallback(const std::function<void(const dtk::V2I&)>&);

            //! Set whether the scroll area has a border.
            void setBorder(bool);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const DrawEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
