// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
            DTK_NON_COPYABLE(MessageDialog);

        protected:
            void _init(
                const std::string& text,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            MessageDialog();

        public:
            virtual ~MessageDialog();

            //! Create a new widget.
            static std::shared_ptr<MessageDialog> create(
                const std::string& text,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the callback.
            void setCallback(const std::function<void(bool)>&);

        private:
            DTK_PRIVATE();
        };

        //! File browser system.
        class MessageDialogSystem : public system::ISystem
        {
            DTK_NON_COPYABLE(MessageDialogSystem);

        protected:
            MessageDialogSystem(const std::shared_ptr<dtk::Context>&);

        public:
            virtual ~MessageDialogSystem();

            //! Create a new system.
            static std::shared_ptr<MessageDialogSystem> create(const std::shared_ptr<dtk::Context>&);

            //! Open the message dialog.
            void open(
                const std::string& text,
                const std::shared_ptr<IWindow>&,
                const std::function<void(bool)>&);

        private:
            DTK_PRIVATE();
        };
    }
}
