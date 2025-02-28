// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/USDPrivate.h>

#include <dtk/core/Error.h>
#include <dtk/core/Format.h>

namespace tl
{
    namespace usd
    {
        DTK_ENUM_IMPL(
            DrawMode,
            "Points",
            "Wireframe",
            "WireframeOnSurface",
            "ShadedFlat",
            "ShadedSmooth",
            "GeomOnly",
            "GeomFlat",
            "GeomSmooth");

        bool Options::operator == (const Options& other) const
        {
            return
                renderWidth == other.renderWidth &&
                complexity == other.complexity &&
                drawMode == other.drawMode &&
                enableLighting == other.enableLighting &&
                sRGB == other.sRGB &&
                stageCache == other.stageCache &&
                diskCache == other.diskCache;
        }

        bool Options::operator != (const Options& other) const
        {
            return !(*this == other);
        }

        io::Options getOptions(const Options& value)
        {
            io::Options out;
            out["USD/RenderWidth"] = dtk::Format("{0}").arg(value.renderWidth);
            out["USD/Complexity"] = dtk::Format("{0}").arg(value.complexity);
            out["USD/DrawMode"] = dtk::Format("{0}").arg(value.drawMode);
            out["USD/EnableLighting"] = dtk::Format("{0}").arg(value.enableLighting);
            out["USD/sRGB"] = dtk::Format("{0}").arg(value.sRGB);
            out["USD/StageCacheCount"] = dtk::Format("{0}").arg(value.stageCache);
            out["USD/DiskCacheByteCount"] = dtk::Format("{0}").arg(value.diskCache);
            return out;
        }

        struct Plugin::Private
        {
            int64_t id = -1;
            std::mutex mutex;
            std::shared_ptr<Render> render;
        };
        
        void Plugin::_init(
            const std::shared_ptr<io::Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
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
            DTK_P();
            p.render = Render::create(cache, logSystem);
        }
        
        Plugin::Plugin() :
            _p(new Private)
        {}
        
        Plugin::~Plugin()
        {}

        std::shared_ptr<Plugin> Plugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(cache, logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            DTK_P();
            int64_t id = -1;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                ++(p.id);
                id = p.id;
            }
            return Read::create(id, p.render, path, options, _cache, _logSystem.lock());
        }
        
        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const io::Options& options)
        {
            DTK_P();
            int64_t id = -1;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                ++(p.id);
                id = p.id;
            }
            return Read::create(id, p.render, path, options, _cache, _logSystem.lock());
        }
        
        dtk::ImageInfo Plugin::getWriteInfo(
            const dtk::ImageInfo&,
            const io::Options&) const
        {
            return dtk::ImageInfo();
        }
        
        std::shared_ptr<io::IWrite> Plugin::write(
            const file::Path&,
            const io::Info&,
            const io::Options&)
        {
            return nullptr;
        }

        void to_json(nlohmann::json& json, const Options& value)
        {
            json["renderWidth"] = value.renderWidth;
            json["complexity"] = value.complexity;
            json["drawMode"] = to_string(value.drawMode);
            json["enableLighting"] = value.enableLighting;
            json["sRGB"] = value.sRGB;
            json["stageCache"] = value.stageCache;
            json["diskCache"] = value.diskCache;
        }

        void from_json(const nlohmann::json& json, Options& value)
        {
            json.at("renderWidth").get_to(value.renderWidth);
            json.at("complexity").get_to(value.complexity);
            from_string(json.at("drawMode").get<std::string>(), value.drawMode);
            json.at("enableLighting").get_to(value.enableLighting);
            json.at("sRGB").get_to(value.sRGB);
            json.at("stageCache").get_to(value.stageCache);
            json.at("diskCache").get_to(value.diskCache);
        }
    }
}

