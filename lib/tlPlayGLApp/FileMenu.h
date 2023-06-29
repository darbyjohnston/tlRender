// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Menu.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! File menu.
        class FileMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(FileMenu);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            FileMenu();

        public:
            ~FileMenu();

            static std::shared_ptr<FileMenu> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            void close() override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
