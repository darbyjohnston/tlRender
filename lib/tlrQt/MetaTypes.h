// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/AudioSystem.h>
#include <tlrCore/FileIO.h>
#include <tlrCore/FontSystem.h>
#include <tlrCore/IRender.h>
#include <tlrCore/OCIO.h>
#include <tlrCore/OS.h>
#include <tlrCore/TimelinePlayer.h>

#include <QMetaType>

Q_DECLARE_METATYPE(tlr::audio::DataType);
Q_DECLARE_METATYPE(tlr::audio::DeviceFormat);

Q_DECLARE_METATYPE(tlr::avio::FileExtensionType);
Q_DECLARE_METATYPE(tlr::avio::VideoType);

Q_DECLARE_METATYPE(tlr::core::LogType);

Q_DECLARE_METATYPE(tlr::file::Mode);

Q_DECLARE_METATYPE(tlr::imaging::FontFamily);

Q_DECLARE_METATYPE(tlr::memory::Endian);

Q_DECLARE_METATYPE(tlr::os::EnvListSeparator);

Q_DECLARE_METATYPE(tlr::render::AlphaBlend);
Q_DECLARE_METATYPE(tlr::render::Channels);
Q_DECLARE_METATYPE(tlr::render::CompareMode);
Q_DECLARE_METATYPE(tlr::render::YUVRange);

Q_DECLARE_METATYPE(tlr::timeline::AudioBufferFrameCount);
Q_DECLARE_METATYPE(tlr::timeline::FileSequenceAudio);
Q_DECLARE_METATYPE(tlr::timeline::Loop);
Q_DECLARE_METATYPE(tlr::timeline::Playback);
Q_DECLARE_METATYPE(tlr::timeline::TimeAction);
Q_DECLARE_METATYPE(tlr::timeline::TimerMode);
Q_DECLARE_METATYPE(tlr::timeline::Transition);
