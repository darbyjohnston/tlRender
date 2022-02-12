// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQt/Util.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>

#include <tlCore/Context.h>
#include <tlCore/Mesh.h>

#include <QSurfaceFormat>

namespace tl
{
    namespace qt
    {
        void init()
        {
            qRegisterMetaType<otime::RationalTime>("otime::RationalTime");
            qRegisterMetaType<otime::TimeRange>("otime::TimeRange");

            qRegisterMetaType<audio::DataType>("tl::audio::DataType");
            qRegisterMetaType<audio::DeviceFormat>("tl::audio::DeviceFormat");
            qRegisterMetaType<audio::Device>("tl::audio::Device");
            QMetaType::registerComparators<audio::DataType>();
            QMetaType::registerComparators<audio::DeviceFormat>();

            qRegisterMetaType<avio::FileExtensionType>("tl::avio::FileExtensionType");
            qRegisterMetaType<avio::Info>("tl::avio::Info");
            qRegisterMetaType<avio::VideoData>("tl::avio::VideoData");
            qRegisterMetaType<avio::AudioData>("tl::avio::AudioData");
            qRegisterMetaType<avio::VideoType>("tl::avio::VideoType");
            QMetaType::registerComparators<avio::FileExtensionType>();
            QMetaType::registerComparators<avio::VideoType>();

            qRegisterMetaType<core::LogItem>("tl::core::LogItem");
            qRegisterMetaType<core::LogType>("tl::core::LogType");
            QMetaType::registerComparators<core::LogType>();

            qRegisterMetaType<file::Mode>("tl::file::Mode");
            qRegisterMetaType<file::PathOptions>("tl::file::PathOptions");
            QMetaType::registerComparators<file::Mode>();

            qRegisterMetaType<geom::Triangle2>("tl::geom::Triangle2");
            qRegisterMetaType<geom::Triangle3>("tl::geom::Triangle3");
            qRegisterMetaType<geom::Triangle2>("tl::geom::TriangleMesh2");
            qRegisterMetaType<geom::Triangle3>("tl::geom::TriangleMesh3");
            qRegisterMetaType<geom::Vertex2>("tl::geom::Vertex2");
            qRegisterMetaType<geom::Vertex3>("tl::geom::Vertex3");

            qRegisterMetaType<imaging::ColorConfig>("tl::imaging::ColorConfig");
            qRegisterMetaType<imaging::FontInfo>("tl::imaging::FontInfo");
            qRegisterMetaType<imaging::FontFamily>("tl::imaging::FontFamily");
            qRegisterMetaType<imaging::FontMetrics>("tl::imaging::FontMetrics");
            qRegisterMetaType<imaging::GlyphInfo>("tl::imaging::GlyphInfo");
            qRegisterMetaType<imaging::Glyph>("tl::imaging::Glyph");
            qRegisterMetaType<imaging::PixelType>("tl::imaging::PixelType");
            qRegisterMetaType<imaging::YUVRange>("tl::imaging::YUVRange");
            QMetaType::registerComparators<imaging::FontFamily>();

            qRegisterMetaType<memory::Endian>("tl::memory::Endian");
            QMetaType::registerComparators<memory::Endian>();

            qRegisterMetaType<observer::CallbackAction>("tl::observer::CallbackAction");

            qRegisterMetaType<os::EnvListSeparator>("tl::os::EnvListSeparator");
            qRegisterMetaType<os::SystemInfo>("tl::os::SystemInfo");
            QMetaType::registerComparators<os::EnvListSeparator>();

            qRegisterMetaType<render::AlphaBlend>("tl::render::AlphaBlend");
            qRegisterMetaType<render::Channels>("tl::render::Channels");
            qRegisterMetaType<render::Color>("tl::render::Color");
            qRegisterMetaType<render::CompareMode>("tl::render::CompareMode");
            qRegisterMetaType<render::CompareOptions>("tl::render::CompareOptions");
            qRegisterMetaType<render::ImageOptions>("tl::render::ImageOptions");
            qRegisterMetaType<render::Exposure>("tl::render::Exposure");
            qRegisterMetaType<render::Levels>("tl::render::Levels");
            qRegisterMetaType<render::YUVRange>("tl::render::YUVRange");
            QMetaType::registerComparators<render::AlphaBlend>();
            QMetaType::registerComparators<render::Channels>();
            QMetaType::registerComparators<render::CompareMode>();
            QMetaType::registerComparators<render::YUVRange>();

            qRegisterMetaType<timeline::AudioBufferFrameCount>("tl::timeline::AudioBufferFrameCount");
            qRegisterMetaType<timeline::AudioData>("tl::timeline::AudioData");
            qRegisterMetaType<timeline::AudioLayer>("tl::timeline::AudioLayer");
            qRegisterMetaType<timeline::FileSequenceAudio>("tl::timeline::FileSequenceAudio");
            qRegisterMetaType<timeline::Loop>("tl::timeline::Loop");
            qRegisterMetaType<timeline::Options>("tl::timeline::Options");
            qRegisterMetaType<timeline::Playback>("tl::timeline::Playback");
            qRegisterMetaType<timeline::PlayerOptions>("tl::timeline::PlayerOptions");
            qRegisterMetaType<timeline::TimeAction>("tl::timeline::TimeAction");
            qRegisterMetaType<timeline::TimerMode>("tl::timeline::TimerMode");
            qRegisterMetaType<timeline::Transition>("tl::timeline::Transition");
            qRegisterMetaType<timeline::VideoData>("tl::timeline::VideoData");
            qRegisterMetaType<timeline::VideoLayer>("tl::timeline::VideoLayer");
            QMetaType::registerComparators<timeline::AudioBufferFrameCount>();
            QMetaType::registerComparators<timeline::FileSequenceAudio>();
            QMetaType::registerComparators<timeline::Loop>();
            QMetaType::registerComparators<timeline::Playback>();
            QMetaType::registerComparators<timeline::TimeAction>();
            QMetaType::registerComparators<timeline::TimerMode>();
            QMetaType::registerComparators<timeline::Transition>();

            qRegisterMetaType<TimeUnits>("tl::qt::TimeUnits");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            qRegisterMetaTypeStreamOperators<TimeUnits>("tl::qt::TimeUnits");
#endif
            QMetaType::registerComparators<TimeUnits>();

            QSurfaceFormat surfaceFormat;
            surfaceFormat.setMajorVersion(4);
            surfaceFormat.setMinorVersion(1);
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            QSurfaceFormat::setDefaultFormat(surfaceFormat);
        }

        QString versionedSettingsKey(const QString& value)
        {
            return QString("%1/%2").arg(QT_VERSION < QT_VERSION_CHECK(6, 0, 0) ? "5" : "6").arg(value);
        }
    }
}

