// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/USD.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

namespace tl
{
    namespace usd
    {
        //! USD renderer options.
        struct RenderOptions
        {
            int      renderWidth = 1920;
            float    complexity = 1.F;
            DrawMode drawMode = DrawMode::ShadedSmooth;
            bool     enableLighting = true;
            bool     sRGB = true;
            size_t   stageCacheCount = 10;
            size_t   diskCacheByteCount = 0;

            bool operator == (const RenderOptions&) const;
            bool operator != (const RenderOptions&) const;
        };

        //! USD renderer.
        class Render : public std::enable_shared_from_this<Render>
        {
        protected:
            void _init(
                const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            Render();

        public:
            ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(
                const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

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
                const io::Options&);

            //! Cancel requests.
            void cancelRequests(int64_t id);

        private:
            void _open(
                const std::string&,
                PXR_NS::UsdStageRefPtr&,
                std::shared_ptr<PXR_NS::UsdImagingGLEngine>&);
            void _run();
            void _finish();

            TLRENDER_PRIVATE();
        };
    }
}

