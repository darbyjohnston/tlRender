// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Plugin.h>

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
        DTK_ENUM(DrawMode);

        //! USD options.
        struct Options
        {
            int renderWidth = 1920;
            float complexity = 1.F;
            usd::DrawMode drawMode = usd::DrawMode::ShadedSmooth;
            bool enableLighting = true;
            bool sRGB = true;
            size_t stageCache = 10;
            size_t diskCache = 0;

            bool operator == (const Options&) const;
            bool operator != (const Options&) const;
        };

        //! Get USD options.
        io::Options getOptions(const Options&);
        
        //! USD reader.
        class Read : public io::IRead
        {
        protected:
            void _init(
                int64_t id,
                const std::shared_ptr<Render>&,
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                int64_t id,
                const std::shared_ptr<Render>&,
                const file::Path&,
                const io::Options&,
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            std::future<io::Info> getInfo() override;
            std::future<io::VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const io::Options&) override;
            void cancelRequests() override;

        private:
            DTK_PRIVATE();
        };

        //! USD plugin.
        class Plugin : public io::IPlugin
        {
        protected:
            void _init(
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);
            
            Plugin();

        public:
            virtual ~Plugin();

            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);
            
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const io::Options & = io::Options()) override;
            dtk::ImageInfo getWriteInfo(
                const dtk::ImageInfo&,
                const io::Options& = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options& = io::Options()) override;
                
        private:
            DTK_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Options&);

        void from_json(const nlohmann::json&, Options&);

        ///@}
    }
}

