// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQt/Init.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>

#include <tlTimeline/Init.h>

#include <tlCore/Mesh.h>

#include <dtk/core/Context.h>

#include <QSurfaceFormat>

namespace tl
{
    namespace qt
    {
        void init(
            const std::shared_ptr<dtk::Context>& context,
            DefaultSurfaceFormat defaultSurfaceFormat)
        {
            timeline::init(context);
            System::create(context, defaultSurfaceFormat);
        }

        System::System(
            const std::shared_ptr<dtk::Context>& context,
            DefaultSurfaceFormat defaultSurfaceFormat) :
            ISystem(context, "tl::qt::System")
        {
            qRegisterMetaType<OTIO_NS::RationalTime>("OTIO_NS::RationalTime");
            qRegisterMetaType<OTIO_NS::TimeRange>("OTIO_NS::TimeRange");
            qRegisterMetaType<std::vector<OTIO_NS::TimeRange> >("std::vector<OTIO_NS::TimeRange>");

            qRegisterMetaType<audio::DataType>("tl::audio::DataType");
            qRegisterMetaType<audio::DeviceID>("tl::audio::DeviceID");
            qRegisterMetaType<audio::DeviceInfo>("tl::audio::DeviceInfo");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<audio::DataType>();
#endif // QT_VERSION

            qRegisterMetaType<file::Mode>("tl::file::Mode");
            qRegisterMetaType<file::PathOptions>("tl::file::PathOptions");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<file::Mode>();
#endif // QT_VERSION

            qRegisterMetaType<geom::Triangle2>("tl::geom::Triangle2");
            qRegisterMetaType<geom::Triangle3>("tl::geom::Triangle3");
            qRegisterMetaType<geom::Triangle2>("tl::geom::TriangleMesh2");
            qRegisterMetaType<geom::Triangle3>("tl::geom::TriangleMesh3");
            qRegisterMetaType<geom::Vertex2>("tl::geom::Vertex2");
            qRegisterMetaType<geom::Vertex3>("tl::geom::Vertex3");

            qRegisterMetaType<image::FontInfo>("tl::image::FontInfo");
            qRegisterMetaType<image::FontMetrics>("tl::image::FontMetrics");
            qRegisterMetaType<image::GlyphInfo>("tl::image::GlyphInfo");
            qRegisterMetaType<image::Glyph>("tl::image::Glyph");
            qRegisterMetaType<image::PixelType>("tl::image::PixelType");
            qRegisterMetaType<image::Size>("tl::image::Size");
            qRegisterMetaType<image::VideoLevels>("tl::image::VideoLevels");

            qRegisterMetaType<math::Size2i>("tl::math::Size2i");

            qRegisterMetaType<memory::Endian>("tl::memory::Endian");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<memory::Endian>();
#endif // QT_VERSION

            qRegisterMetaType<io::FileType>("tl::io::FileType");
            qRegisterMetaType<io::Info>("tl::io::Info");
            qRegisterMetaType<io::VideoData>("tl::io::VideoData");
            qRegisterMetaType<io::AudioData>("tl::io::AudioData");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<io::FileType>();
#endif // QT_VERSION

            qRegisterMetaType<timeline::AlphaBlend>("tl::timeline::AlphaBlend");
            qRegisterMetaType<timeline::AudioData>("tl::timeline::AudioData");
            qRegisterMetaType<timeline::AudioLayer>("tl::timeline::AudioLayer");
            qRegisterMetaType<timeline::Channels>("tl::timeline::Channels");
            qRegisterMetaType<timeline::Color>("tl::timeline::Color");
            qRegisterMetaType<timeline::CompareMode>("tl::timeline::CompareMode");
            qRegisterMetaType<timeline::CompareTimeMode>("tl::timeline::CompareTimeMode");
            qRegisterMetaType<timeline::CompareOptions>("tl::timeline::CompareOptions");
            qRegisterMetaType<timeline::EXRDisplay>("tl::timeline::EXRDisplay");
            qRegisterMetaType<timeline::FileSequenceAudio>("tl::timeline::FileSequenceAudio");
            qRegisterMetaType<timeline::ImageFilter>("tl::timeline::ImageFilter");
            qRegisterMetaType<timeline::ImageOptions>("tl::timeline::ImageOptions");
            qRegisterMetaType<timeline::InputVideoLevels>("tl::timeline::InputVideoLevels");
            qRegisterMetaType<timeline::LUTOptions>("tl::timeline::LUTOptions");
            qRegisterMetaType<timeline::Levels>("tl::timeline::Levels");
            qRegisterMetaType<timeline::Loop>("tl::timeline::Loop");
            qRegisterMetaType<timeline::OCIOOptions>("tl::timeline::OCIOOptions");
            qRegisterMetaType<timeline::Options>("tl::timeline::Options");
            qRegisterMetaType<timeline::Playback>("tl::timeline::Playback");
            qRegisterMetaType<timeline::PlayerCacheInfo>("tl::timeline::PlayerCacheInfo");
            qRegisterMetaType<timeline::PlayerCacheOptions>("tl::timeline::PlayerCacheOptions");
            qRegisterMetaType<timeline::PlayerOptions>("tl::timeline::PlayerOptions");
            qRegisterMetaType<timeline::TimeAction>("tl::timeline::TimeAction");
            qRegisterMetaType<timeline::TimeUnits>("tl::timeline::TimeUnits");
            qRegisterMetaType<timeline::Transition>("tl::timeline::Transition");
            qRegisterMetaType<timeline::VideoData>("tl::timeline::VideoData");
            qRegisterMetaType<timeline::VideoLayer>("tl::timeline::VideoLayer");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<timeline::AlphaBlend>();
            QMetaType::registerComparators<timeline::Channels>();
            QMetaType::registerComparators<timeline::CompareMode>();
            QMetaType::registerComparators<timeline::CompareTimeMode>();
            QMetaType::registerComparators<timeline::FileSequenceAudio>();
            QMetaType::registerComparators<timeline::ImageFilter>();
            QMetaType::registerComparators<timeline::InputVideoLevels>();
            QMetaType::registerComparators<timeline::Loop>();
            QMetaType::registerComparators<timeline::Playback>();
            QMetaType::registerComparators<timeline::TimeAction>();
            QMetaType::registerComparators<timeline::TimeUnits>();
            QMetaType::registerComparators<timeline::Transition>();
#endif // QT_VERSION

            switch (defaultSurfaceFormat)
            {
            case DefaultSurfaceFormat::OpenGL_4_1_CoreProfile:
            {
                QSurfaceFormat surfaceFormat;
                surfaceFormat.setMajorVersion(4);
                surfaceFormat.setMinorVersion(1);
                surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
                QSurfaceFormat::setDefaultFormat(surfaceFormat);
                break;
            }
            default: break;
            }
        }

        System::~System()
        {}

        std::shared_ptr<System> System::create(
            const std::shared_ptr<dtk::Context>& context,
            DefaultSurfaceFormat defaultSurfaceFormat)
        {
            auto out = context->getSystem<System>();
            if (!out)
            {
                out = std::shared_ptr<System>(new System(context, defaultSurfaceFormat));
                context->addSystem(out);
            }
            return out;
        }
    }
}

