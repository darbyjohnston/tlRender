// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQt/Init.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>

#include <tlTimeline/Init.h>

#include <feather-tk/core/Context.h>

#include <QSurfaceFormat>

namespace tl
{
    namespace qt
    {
        void init(
            const std::shared_ptr<feather_tk::Context>& context,
            DefaultSurfaceFormat defaultSurfaceFormat)
        {
            timeline::init(context);
            System::create(context, defaultSurfaceFormat);
        }

        System::System(
            const std::shared_ptr<feather_tk::Context>& context,
            DefaultSurfaceFormat defaultSurfaceFormat) :
            ISystem(context, "tl::qt::System")
        {
            qRegisterMetaType<OTIO_NS::RationalTime>("OTIO_NS::RationalTime");
            qRegisterMetaType<OTIO_NS::TimeRange>("OTIO_NS::TimeRange");
            qRegisterMetaType<std::vector<OTIO_NS::TimeRange> >("std::vector<OTIO_NS::TimeRange>");

            qRegisterMetaType<feather_tk::AlphaBlend>("feather_tk::AlphaBlend");
            qRegisterMetaType<feather_tk::ChannelDisplay>("feather_tk::ChannelDisplay");
            qRegisterMetaType<feather_tk::ImageType>("feather_tk::ImageType");
            qRegisterMetaType<feather_tk::ImageFilter>("feather_tk::ImageFilter");
            qRegisterMetaType<feather_tk::InputVideoLevels>("feather_tk::InputVideoLevels");
            qRegisterMetaType<feather_tk::Size2I>("feather_tk::Size2I");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<feather_tk::AlphaBlend>();
            QMetaType::registerComparators<feather_tk::ChannelDisplay>();
            QMetaType::registerComparators<feather_tk::ImageType>();
            QMetaType::registerComparators<feather_tk::ImageFilter>();
            QMetaType::registerComparators<feather_tk::InputVideoLevels>();
#endif // QT_VERSION

            qRegisterMetaType<audio::DataType>("tl::audio::DataType");
            qRegisterMetaType<audio::DeviceID>("tl::audio::DeviceID");
            qRegisterMetaType<audio::DeviceInfo>("tl::audio::DeviceInfo");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<audio::DataType>();
#endif // QT_VERSION

            qRegisterMetaType<file::PathOptions>("tl::file::PathOptions");

            qRegisterMetaType<io::FileType>("tl::io::FileType");
            qRegisterMetaType<io::Info>("tl::io::Info");
            qRegisterMetaType<io::VideoData>("tl::io::VideoData");
            qRegisterMetaType<io::AudioData>("tl::io::AudioData");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<io::FileType>();
#endif // QT_VERSION

            qRegisterMetaType<timeline::AudioData>("tl::timeline::AudioData");
            qRegisterMetaType<timeline::AudioLayer>("tl::timeline::AudioLayer");
            qRegisterMetaType<timeline::Color>("tl::timeline::Color");
            qRegisterMetaType<timeline::Compare>("tl::timeline::Compare");
            qRegisterMetaType<timeline::CompareTime>("tl::timeline::CompareTime");
            qRegisterMetaType<timeline::CompareOptions>("tl::timeline::CompareOptions");
            qRegisterMetaType<timeline::EXRDisplay>("tl::timeline::EXRDisplay");
            qRegisterMetaType<timeline::ImageSequenceAudio>("tl::timeline::ImageSequenceAudio");
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
            QMetaType::registerComparators<timeline::Compare>();
            QMetaType::registerComparators<timeline::CompareTime>();
            QMetaType::registerComparators<timeline::ImageuenceAudio>();
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
            const std::shared_ptr<feather_tk::Context>& context,
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

