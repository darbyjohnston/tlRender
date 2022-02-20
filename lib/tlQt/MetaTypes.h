// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/AudioSystem.h>
#include <tlCore/FileIO.h>
#include <tlCore/Image.h>
#include <tlCore/FontSystem.h>
#include <tlCore/IRender.h>
#include <tlCore/OCIO.h>
#include <tlCore/OS.h>
#include <tlCore/TimelinePlayer.h>

#include <QMetaType>

Q_DECLARE_METATYPE(tl::audio::DataType);
Q_DECLARE_METATYPE(tl::audio::DeviceFormat);

Q_DECLARE_METATYPE(tl::avio::FileExtensionType);
Q_DECLARE_METATYPE(tl::avio::VideoType);

Q_DECLARE_METATYPE(tl::core::LogType);

Q_DECLARE_METATYPE(tl::file::Mode);

Q_DECLARE_METATYPE(tl::imaging::Size);
Q_DECLARE_METATYPE(tl::imaging::FontFamily);

Q_DECLARE_METATYPE(tl::memory::Endian);

Q_DECLARE_METATYPE(tl::os::EnvListSeparator);

Q_DECLARE_METATYPE(tl::render::AlphaBlend);
Q_DECLARE_METATYPE(tl::render::Channels);
Q_DECLARE_METATYPE(tl::render::CompareMode);
Q_DECLARE_METATYPE(tl::render::YUVRange);

Q_DECLARE_METATYPE(tl::timeline::AudioBufferFrameCount);
Q_DECLARE_METATYPE(tl::timeline::FileSequenceAudio);
Q_DECLARE_METATYPE(tl::timeline::Loop);
Q_DECLARE_METATYPE(tl::timeline::Playback);
Q_DECLARE_METATYPE(tl::timeline::TimeAction);
Q_DECLARE_METATYPE(tl::timeline::TimerMode);
Q_DECLARE_METATYPE(tl::timeline::Transition);
