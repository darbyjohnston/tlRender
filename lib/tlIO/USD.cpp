// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlIO/USDPrivate.h>

#include <tlCore/Error.h>

namespace tl
{
    namespace usd
    {
        TLRENDER_ENUM_IMPL(
            DrawMode,
            "Points",
            "Wireframe",
            "WireframeOnSurface",
            "ShadedFlat",
            "ShadedSmooth",
            "GeomOnly",
            "GeomFlat",
            "GeomSmooth");
        TLRENDER_ENUM_SERIALIZE_IMPL(DrawMode);
        
        bool RenderOptions::operator == (const RenderOptions& other) const
        {
            return
                renderWidth == other.renderWidth &&
                complexity == other.complexity &&
                drawMode == other.drawMode &&
                enableLighting == other.enableLighting &&
                sRGB == other.sRGB &&
                stageCacheCount == other.stageCacheCount &&
                diskCacheByteCount == other.diskCacheByteCount;
        }
        
        bool RenderOptions::operator != (const RenderOptions& other) const
        {
            return !(*this == other);
        }

        struct Plugin::Private
        {
            int64_t id = -1;
            std::shared_ptr<Render> render;
        };
        
        void Plugin::_init(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            IPlugin::_init(
                "USD",
                {
                    { ".usd", io::FileType::Sequence },
                    { ".usda", io::FileType::Sequence },
                    { ".usdc", io::FileType::Sequence },
                    { ".usdz", io::FileType::Sequence }
                },
                cache,
                logSystem);
            TLRENDER_P();
            p.render = Render::create(cache, logSystem);
        }
        
        Plugin::Plugin() :
            _p(new Private)
        {}
        
        std::shared_ptr<Plugin> Plugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(cache, logSystem);
            return out;
        }
        
        void Plugin::setOptions(const io::Options& value)
        {
            const bool changed = value != _options;
            IPlugin::setOptions(value);
            TLRENDER_P();
            if (changed)
            {
                RenderOptions renderOptions;
                auto i = _options.find("USD/renderWidth");
                if (i != _options.end())
                {
                    std::stringstream ss(i->second);
                    ss >> renderOptions.renderWidth;
                }
                i = _options.find("USD/complexity");
                if (i != _options.end())
                {
                    std::stringstream ss(i->second);
                    ss >> renderOptions.complexity;
                }
                i = _options.find("USD/drawMode");
                if (i != _options.end())
                {
                    std::stringstream ss(i->second);
                    ss >> renderOptions.drawMode;
                }
                i = _options.find("USD/enableLighting");
                if (i != _options.end())
                {
                    std::stringstream ss(i->second);
                    ss >> renderOptions.enableLighting;
                }
                i = _options.find("USD/stageCacheCount");
                if (i != _options.end())
                {
                    std::stringstream ss(i->second);
                    ss >> renderOptions.stageCacheCount;
                }
                i = _options.find("USD/diskCacheByteCount");
                if (i != _options.end())
                {
                    std::stringstream ss(i->second);
                    ss >> renderOptions.diskCacheByteCount;
                }
                p.render->setRenderOptions(renderOptions);
            }
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            TLRENDER_P();
            ++(p.id);
            return Read::create(
                p.id,
                p.render,
                path,
                io::merge(options, _options),
                _cache,
                _logSystem);
        }
        
        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            TLRENDER_P();
            ++(p.id);
            return Read::create(
                p.id,
                p.render,
                path,
                io::merge(options, _options),
                _cache,
                _logSystem);
        }
        
        image::Info Plugin::getWriteInfo(
            const image::Info&,
            const io::Options&) const
        {
            return image::Info();
        }
        
        std::shared_ptr<io::IWrite> Plugin::write(
            const file::Path&,
            const io::Info&,
            const io::Options&)
        {
            return nullptr;
        }
    }
}

