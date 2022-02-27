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

            qRegisterMetaType<log::Item>("tl::log::Item");
            qRegisterMetaType<log::Type>("tl::log::Type");
            QMetaType::registerComparators<log::Type>();

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
            qRegisterMetaType<imaging::Size>("tl::imaging::Size");
            qRegisterMetaType<imaging::YUVRange>("tl::imaging::YUVRange");
            QMetaType::registerComparators<imaging::FontFamily>();

            qRegisterMetaType<memory::Endian>("tl::memory::Endian");
            QMetaType::registerComparators<memory::Endian>();

            qRegisterMetaType<observer::CallbackAction>("tl::observer::CallbackAction");

            qRegisterMetaType<os::EnvListSeparator>("tl::os::EnvListSeparator");
            qRegisterMetaType<os::SystemInfo>("tl::os::SystemInfo");
            QMetaType::registerComparators<os::EnvListSeparator>();

            qRegisterMetaType<io::FileExtensionType>("tl::io::FileExtensionType");
            qRegisterMetaType<io::Info>("tl::io::Info");
            qRegisterMetaType<io::VideoData>("tl::io::VideoData");
            qRegisterMetaType<io::AudioData>("tl::io::AudioData");
            qRegisterMetaType<io::VideoType>("tl::io::VideoType");
            QMetaType::registerComparators<io::FileExtensionType>();
            QMetaType::registerComparators<io::VideoType>();

            qRegisterMetaType<timeline::AlphaBlend>("tl::timeline::AlphaBlend");
            qRegisterMetaType<timeline::AudioBufferFrameCount>("tl::timeline::AudioBufferFrameCount");
            qRegisterMetaType<timeline::AudioData>("tl::timeline::AudioData");
            qRegisterMetaType<timeline::AudioLayer>("tl::timeline::AudioLayer");
            qRegisterMetaType<timeline::Channels>("tl::timeline::Channels");
            qRegisterMetaType<timeline::Color>("tl::timeline::Color");
            qRegisterMetaType<timeline::CompareMode>("tl::timeline::CompareMode");
            qRegisterMetaType<timeline::CompareOptions>("tl::timeline::CompareOptions");
            qRegisterMetaType<timeline::ImageOptions>("tl::timeline::ImageOptions");
            qRegisterMetaType<timeline::Exposure>("tl::timeline::Exposure");
            qRegisterMetaType<timeline::FileSequenceAudio>("tl::timeline::FileSequenceAudio");
            qRegisterMetaType<timeline::Levels>("tl::timeline::Levels");
            qRegisterMetaType<timeline::Loop>("tl::timeline::Loop");
            qRegisterMetaType<timeline::Options>("tl::timeline::Options");
            qRegisterMetaType<timeline::Playback>("tl::timeline::Playback");
            qRegisterMetaType<timeline::PlayerOptions>("tl::timeline::PlayerOptions");
            qRegisterMetaType<timeline::TimeAction>("tl::timeline::TimeAction");
            qRegisterMetaType<timeline::TimerMode>("tl::timeline::TimerMode");
            qRegisterMetaType<timeline::Transition>("tl::timeline::Transition");
            qRegisterMetaType<timeline::VideoData>("tl::timeline::VideoData");
            qRegisterMetaType<timeline::VideoLayer>("tl::timeline::VideoLayer");
            qRegisterMetaType<timeline::YUVRange>("tl::timeline::YUVRange");
            QMetaType::registerComparators<timeline::AlphaBlend>();
            QMetaType::registerComparators<timeline::AudioBufferFrameCount>();
            QMetaType::registerComparators<timeline::Channels>();
            QMetaType::registerComparators<timeline::CompareMode>();
            QMetaType::registerComparators<timeline::FileSequenceAudio>();
            QMetaType::registerComparators<timeline::Loop>();
            QMetaType::registerComparators<timeline::Playback>();
            QMetaType::registerComparators<timeline::TimeAction>();
            QMetaType::registerComparators<timeline::TimerMode>();
            QMetaType::registerComparators<timeline::Transition>();
            QMetaType::registerComparators<timeline::YUVRange>();

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
    }
}

