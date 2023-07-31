// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Stack layout.
        class StackLayout : public IWidget
        {
            TLRENDER_NON_COPYABLE(StackLayout);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            StackLayout();

        public:
            virtual ~StackLayout();

            //! Create a new layout.
            static std::shared_ptr<StackLayout> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the current index.
            int getCurrentIndex() const;

            //! Set the current index.
            void setCurrentIndex(int);

            //! Set the current widget.
            void setCurrentWidget(const std::shared_ptr<IWidget>&);

            //! Set the margin role.
            void setMarginRole(SizeRole);

            void setGeometry(const math::BBox2i&) override;
            math::BBox2i getChildrenClipRect() const override;
            void childAddedEvent(const ChildEvent&) override;
            void childRemovedEvent(const ChildEvent&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            std::shared_ptr<IWidget> _getCurrentWidget() const;

            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
