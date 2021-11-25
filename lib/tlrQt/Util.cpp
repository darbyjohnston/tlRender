// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/Util.h>

#include <tlrQt/TimeObject.h>

#include <tlrCore/Context.h>
#include <tlrCore/TimelinePlayer.h>

#include <QSurfaceFormat>

namespace tlr
{
    namespace qt
    {
        void init()
        {
            qRegisterMetaType<timeline::TimerMode>("tlr::timeline::SeparateAudio");
            qRegisterMetaType<timeline::VideoData>("tlr::timeline::VideoData");
            qRegisterMetaType<timeline::TimerMode>("tlr::timeline::TimerMode");
            qRegisterMetaType<timeline::AudioBufferFrameCount>("tlr::timeline::AudioBufferFrameCount");
            qRegisterMetaType<TimeUnits>("tlr::qt::TimeUnits");
            qRegisterMetaTypeStreamOperators<TimeUnits>("tlr::qt::TimeUnits");

            QSurfaceFormat surfaceFormat;
            surfaceFormat.setMajorVersion(4);
            surfaceFormat.setMinorVersion(1);
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            QSurfaceFormat::setDefaultFormat(surfaceFormat);
        }
    }
}

