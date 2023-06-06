// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/USD.h>

namespace tl
{
    namespace usd
    {
        //! USD renderer.
        class Render : public std::enable_shared_from_this<Render>
        {
        protected:
            void _init(const std::weak_ptr<log::System>&);

            Render();

        public:
            ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(const std::weak_ptr<log::System>&);

            //! Set render options.
            void setRenderOptions(const RenderOptions&);
            
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

