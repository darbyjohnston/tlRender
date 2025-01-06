// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Search box widget.
        class SearchBox : public IWidget
        {
            TLRENDER_NON_COPYABLE(SearchBox);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            SearchBox();

        public:
            virtual ~SearchBox();

            //! Create a new widget.
            static std::shared_ptr<SearchBox> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the text.
            void setText(const std::string&);

            //! Set the callback.
            void setCallback(const std::function<void(const std::string&)>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
