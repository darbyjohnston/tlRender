// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/FilesTool.h>

#include <tlUI/IButton.h>

namespace tl
{
    namespace play_app
    {
        class FileButton : public ui::IButton
        {
            DTK_NON_COPYABLE(FileButton);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<play::FilesModelItem>&,
                const std::shared_ptr<IWidget>& parent);

            FileButton();

        public:
            virtual ~FileButton();

            static std::shared_ptr<FileButton> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<play::FilesModelItem>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void tickEvent(
                bool,
                bool,
                const ui::TickEvent&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(const dtk::Box2I&, bool) override;
            void drawEvent(const dtk::Box2I&, const ui::DrawEvent&) override;
            void keyPressEvent(ui::KeyEvent&) override;
            void keyReleaseEvent(ui::KeyEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
