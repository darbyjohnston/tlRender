// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>

namespace tl
{
    namespace ui
    {
        //! Action.
        class Action : public std::enable_shared_from_this<Action>
        {
            TLRENDER_NON_COPYABLE(Action);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            Action();

        public:
            ~Action();

            //! Create a new action.
            static std::shared_ptr<Action> create(
                const std::shared_ptr<system::Context>&);

            //! Get the text.
            const std::string& getText() const;

            //! Set the text.
            void setText(const std::string&);

            //! Set the clicked callback.
            void setClickedCallback(const std::function<void(void)>&);

            //! Click the action.
            void click();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
