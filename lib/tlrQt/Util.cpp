// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrQt/Util.h>

#include <tlrQt/TimeObject.h>

#include <tlrCore/AudioSystem.h>
#include <tlrCore/Context.h>
#include <tlrCore/FileIO.h>
#include <tlrCore/FontSystem.h>
#include <tlrCore/IRender.h>
#include <tlrCore/Mesh.h>
#include <tlrCore/OCIO.h>
#include <tlrCore/OS.h>
#include <tlrCore/TimelinePlayer.h>

#include <QSurfaceFormat>

namespace tlr
{
    namespace qt
    {
        void init()
        {
            qRegisterMetaType<audio::DataType>("tlr::audio::DataType");
            qRegisterMetaType<audio::DeviceFormat>("tlr::audio::DeviceFormat");
            qRegisterMetaType<audio::Device>("tlr::audio::Device");

            qRegisterMetaType<avio::FileExtensionType>("tlr::avio::FileExtensionType");
            qRegisterMetaType<avio::Info>("tlr::avio::Info");
            qRegisterMetaType<avio::VideoData>("tlr::avio::VideoData");
            qRegisterMetaType<avio::AudioData>("tlr::avio::AudioData");
            qRegisterMetaType<avio::VideoType>("tlr::avio::VideoType");

            qRegisterMetaType<core::LogItem>("tlr::core::LogItem");
            qRegisterMetaType<core::LogType>("tlr::core::LogType");

            qRegisterMetaType<file::Mode>("tlr::file::Mode");
            qRegisterMetaType<file::PathOptions>("tlr::file::PathOptions");

            qRegisterMetaType<geom::Triangle2>("tlr::geom::Triangle2");
            qRegisterMetaType<geom::Triangle3>("tlr::geom::Triangle3");
            qRegisterMetaType<geom::Triangle2>("tlr::geom::TriangleMesh2");
            qRegisterMetaType<geom::Triangle3>("tlr::geom::TriangleMesh3");
            qRegisterMetaType<geom::Vertex2>("tlr::geom::Vertex2");
            qRegisterMetaType<geom::Vertex3>("tlr::geom::Vertex3");

            qRegisterMetaType<imaging::ColorConfig>("tlr::imaging::ColorConfig");
            qRegisterMetaType<imaging::FontInfo>("tlr::imaging::FontInfo");
            qRegisterMetaType<imaging::FontFamily>("tlr::imaging::FontFamily");
            qRegisterMetaType<imaging::FontMetrics>("tlr::imaging::FontMetrics");
            qRegisterMetaType<imaging::GlyphInfo>("tlr::imaging::GlyphInfo");
            qRegisterMetaType<imaging::Glyph>("tlr::imaging::Glyph");
            qRegisterMetaType<imaging::PixelType>("tlr::imaging::PixelType");
            qRegisterMetaType<imaging::YUVRange>("tlr::imaging::YUVRange");

            qRegisterMetaType<memory::Endian>("tlr::memory::Endian");

            qRegisterMetaType<observer::CallbackAction>("tlr::observer::CallbackAction");

            qRegisterMetaType<os::EnvListSeparator>("tlr::os::EnvListSeparator");
            qRegisterMetaType<os::SystemInfo>("tlr::os::SystemInfo");

            qRegisterMetaType<render::AlphaBlend>("tlr::render::AlphaBlend");
            qRegisterMetaType<render::Channels>("tlr::render::Channels");
            qRegisterMetaType<render::CompareMode>("tlr::render::CompareMode");
            qRegisterMetaType<render::CompareOptions>("tlr::render::CompareOptions");
            qRegisterMetaType<render::Color>("tlr::render::Color");
            qRegisterMetaType<render::CompareMode>("tlr::render::CompareMode");
            qRegisterMetaType<render::ImageOptions>("tlr::render::ImageOptions");
            qRegisterMetaType<render::Exposure>("tlr::render::Exposure");
            qRegisterMetaType<render::Levels>("tlr::render::Levels");
            qRegisterMetaType<render::YUVRange>("tlr::render::YUVRange");

            qRegisterMetaType<timeline::AudioBufferFrameCount>("tlr::timeline::AudioBufferFrameCount");
            qRegisterMetaType<timeline::AudioData>("tlr::timeline::AudioData");
            qRegisterMetaType<timeline::AudioLayer>("tlr::timeline::AudioLayer");
            qRegisterMetaType<timeline::FileSequenceAudio>("tlr::timeline::FileSequenceAudio");
            qRegisterMetaType<timeline::Loop>("tlr::timeline::Loop");
            qRegisterMetaType<timeline::Options>("tlr::timeline::Options");
            qRegisterMetaType<timeline::Playback>("tlr::timeline::Playback");
            qRegisterMetaType<timeline::PlayerOptions>("tlr::timeline::PlayerOptions");
            qRegisterMetaType<timeline::TimeAction>("tlr::timeline::TimeAction");
            qRegisterMetaType<timeline::TimerMode>("tlr::timeline::TimerMode");
            qRegisterMetaType<timeline::Transition>("tlr::timeline::Transition");
            qRegisterMetaType<timeline::VideoData>("tlr::timeline::VideoData");
            qRegisterMetaType<timeline::VideoLayer>("tlr::timeline::VideoLayer");

            qRegisterMetaType<TimeUnits>("tlr::qt::TimeUnits");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            qRegisterMetaTypeStreamOperators<TimeUnits>("tlr::qt::TimeUnits");
#endif

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

