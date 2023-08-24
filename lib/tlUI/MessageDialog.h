// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IDialog.h>

#include <tlCore/ISystem.h>

namespace tl
{
    namespace ui
    {
        //!  dialog.
        class MessageDialog : public IDialog
        {
            TLRENDER_NON_COPYABLE(MessageDialog);

        protected:
            void _init(
                const std::string& text,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            MessageDialog();

        public:
            virtual ~MessageDialog();

            //! Create a new widget.
            static std::shared_ptr<MessageDialog> create(
                const std::string& text,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the callback.
            void setCallback(const std::function<void(bool)>&);

        private:
            TLRENDER_PRIVATE();
        };

        //! File browser system.
        class MessageDialogSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(MessageDialogSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            MessageDialogSystem();

        public:
            virtual ~MessageDialogSystem();

            //! Create a new system.
            static std::shared_ptr<MessageDialogSystem> create(const std::shared_ptr<system::Context>&);

            //! Open the message dialog.
            void open(
                const std::string& text,
                const std::shared_ptr<EventLoop>&,
                const std::function<void(bool)>&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
