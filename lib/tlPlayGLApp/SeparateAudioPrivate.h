
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayGLApp/SeparateAudioDialog.h>

namespace tl
{
    namespace play_gl
    {
        class SeparateAudioWidget : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(SeparateAudioWidget);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            SeparateAudioWidget();

        public:
            virtual ~SeparateAudioWidget();

            static std::shared_ptr<SeparateAudioWidget> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setFileCallback(const std::function<void(
                const file::Path&,
                const file::Path&)>&);

            void setCancelCallback(const std::function<void(void)>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
