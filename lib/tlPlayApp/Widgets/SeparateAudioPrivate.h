
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Widgets/SeparateAudioDialog.h>

namespace tl
{
    namespace play
    {
        class SeparateAudioWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(SeparateAudioWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            SeparateAudioWidget();

        public:
            virtual ~SeparateAudioWidget();

            static std::shared_ptr<SeparateAudioWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setCallback(const std::function<void(
                const file::Path&,
                const file::Path&)>&);

            void setCancelCallback(const std::function<void(void)>&);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
