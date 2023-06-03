// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUSD/USD.h>

namespace tl
{
    namespace usd
    {
        //! USD renderer.
        class Renderer : public std::enable_shared_from_this<Renderer>
        {
        protected:
            void _init(const std::weak_ptr<log::System>&);

            Renderer();

        public:
            ~Renderer();

            //! Create a new renderer.
            static std::shared_ptr<Renderer> create(const std::weak_ptr<log::System>&);

            //! Get information.
            std::future<io::Info> getInfo(
                int64_t id,
                const file::Path& path);
            
            //! Render an image.
            std::future<io::VideoData> render(
                int64_t id,
                const file::Path& path,
                const otime::RationalTime& time,
                uint16_t layer = 0);

            //! Cancel requests.
            void cancelRequests(int64_t id);

            //! Cancel requests.
            void cancelRequests();

        private:
            void _createWindow();
            void _run();

            TLRENDER_PRIVATE();
        };
    }
}

