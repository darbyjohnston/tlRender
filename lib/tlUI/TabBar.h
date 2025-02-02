// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Tab bar widget.
        class TabBar : public IWidget
        {
            TLRENDER_NON_COPYABLE(TabBar);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TabBar();

        public:
            virtual ~TabBar();

            //! Create a new widget.
            static std::shared_ptr<TabBar> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the tabs.
            void setTabs(const std::vector<std::string>&);

            //! Add a tab.
            void addTab(const std::string&);

            //! Clear the tabs.
            void clearTabs();

            //! Get the current tab.
            int getCurrentTab() const;

            //! Set the current tab.
            void setCurrentTab(int);

            //! Set the callback.
            void setCallback(const std::function<void(int)>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
