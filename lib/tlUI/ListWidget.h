// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/ButtonGroup.h>
#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! List widget.
        class ListWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(ListWidget);

        protected:
            void _init(
                ButtonGroupType,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ListWidget();

        public:
            virtual ~ListWidget();

            //! Create a new widget.
            static std::shared_ptr<ListWidget> create(
                ButtonGroupType,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the items.
            void setItems(const std::vector<std::string>&);

            //! Set the current item.
            void setCurrentItem(int);

            //! Set the callback.
            void setCallback(const std::function<void(int)>&);

            //! Set the search.
            void setSearch(const std::string&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            void _widgetUpdate();
            void _searchUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
