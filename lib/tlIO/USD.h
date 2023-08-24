// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

namespace tl
{
    //! USD image I/O.
    namespace usd
    {
        class Render;

        //! USD draw modes.
        enum class DrawMode
        {
            Points,
            Wireframe,
            WireframeOnSurface,
            ShadedFlat,
            ShadedSmooth,
            GeomOnly,
            GeomFlat,
            GeomSmooth,
            
            Count,
            First = Points
        };
        TLRENDER_ENUM(DrawMode);
        TLRENDER_ENUM_SERIALIZE(DrawMode);
        
        //! USD renderer options.
        struct RenderOptions
        {
            size_t   renderWidth        = 1920;
            float    complexity         = 1.F;
            DrawMode drawMode           = DrawMode::ShadedSmooth;
            bool     enableLighting     = true;
            bool     sRGB               = true;
            size_t   stageCacheCount    = 10;
            size_t   diskCacheByteCount = 0;
            
            bool operator == (const RenderOptions&) const;
            bool operator != (const RenderOptions&) const;
        };
        
        //! USD reader.
        class Read : public io::IRead
        {
        protected:
            void _init(
                int64_t id,
                const std::shared_ptr<Render>&,
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                int64_t id,
                const std::shared_ptr<Render>&,
                const file::Path&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            std::future<io::Info> getInfo() override;
            std::future<io::VideoData> readVideo(const otime::RationalTime&, uint16_t layer = 0) override;
            void cancelRequests() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! USD plugin.
        class Plugin : public io::IPlugin
        {
        protected:
            void _init(const std::weak_ptr<log::System>&);
            
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(const std::weak_ptr<log::System>&);
            
            void setOptions(const io::Options&) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options & = io::Options()) override;
            image::Info getWriteInfo(
                const image::Info&,
                const io::Options& = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options& = io::Options()) override;
                
        private:
            TLRENDER_PRIVATE();
        };
    }
}

