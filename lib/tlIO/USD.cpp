// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/USDPrivate.h>

#include <ftk/Core/Error.h>
#include <ftk/Core/Format.h>

namespace tl
{
    namespace usd
    {
        FTK_ENUM_IMPL(
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
            out["USD/RenderWidth"] = ftk::Format("{0}").arg(value.renderWidth);
            out["USD/Complexity"] = ftk::Format("{0}").arg(value.complexity);
            out["USD/DrawMode"] = ftk::Format("{0}").arg(value.drawMode);
            out["USD/EnableLighting"] = ftk::Format("{0}").arg(value.enableLighting);
            out["USD/sRGB"] = ftk::Format("{0}").arg(value.sRGB);
            out["USD/StageCache"] = ftk::Format("{0}").arg(value.stageCache);
            out["USD/DiskCache"] = ftk::Format("{0}").arg(value.diskCache);
            return out;
        }

        struct ReadPlugin::Private
        {
            int64_t id = -1;
            std::mutex mutex;
            std::shared_ptr<Render> render;
        };
        
        void ReadPlugin::_init(const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IReadPlugin::_init(
                "USD",
                {
                    { ".usd", io::FileType::Sequence },
                    { ".usda", io::FileType::Sequence },
                    { ".usdc", io::FileType::Sequence },
                    { ".usdz", io::FileType::Sequence }
                },
                logSystem);
            FTK_P();
            p.render = Render::create(logSystem);
        }
        
        ReadPlugin::ReadPlugin() :
            _p(new Private)
        {}
        
        ReadPlugin::~ReadPlugin()
        {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            FTK_P();
            int64_t id = -1;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                ++(p.id);
                id = p.id;
            }
            return Read::create(id, p.render, path, options, _logSystem.lock());
        }
        
        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options)
        {
            FTK_P();
            int64_t id = -1;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                ++(p.id);
                id = p.id;
            }
            return Read::create(id, p.render, path, options, _logSystem.lock());
        }

        void to_json(nlohmann::json& json, const Options& value)
        {
            json["RenderWidth"] = value.renderWidth;
            json["Complexity"] = value.complexity;
            json["DrawMode"] = to_string(value.drawMode);
            json["EnableLighting"] = value.enableLighting;
            json["sRGB"] = value.sRGB;
            json["StageCache"] = value.stageCache;
            json["DiskCache"] = value.diskCache;
        }

        void from_json(const nlohmann::json& json, Options& value)
        {
            json.at("RenderWidth").get_to(value.renderWidth);
            json.at("Complexity").get_to(value.complexity);
            from_string(json.at("DrawMode").get<std::string>(), value.drawMode);
            json.at("EnableLighting").get_to(value.enableLighting);
            json.at("sRGB").get_to(value.sRGB);
            json.at("StageCache").get_to(value.stageCache);
            json.at("DiskCache").get_to(value.diskCache);
        }
    }
}

