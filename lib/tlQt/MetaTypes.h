// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>
#include <tlTimeline/TimelinePlayer.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/ColorConfig.h>
#include <tlCore/FileIO.h>
#include <tlCore/FontSystem.h>
#include <tlCore/Image.h>
#include <tlCore/OS.h>

#include <QMetaType>

Q_DECLARE_METATYPE(tl::audio::DataType);
Q_DECLARE_METATYPE(tl::audio::DeviceFormat);

Q_DECLARE_METATYPE(tl::log::Type);

Q_DECLARE_METATYPE(tl::file::Mode);

Q_DECLARE_METATYPE(tl::imaging::Size);
Q_DECLARE_METATYPE(tl::imaging::FontFamily);

Q_DECLARE_METATYPE(tl::memory::Endian);

Q_DECLARE_METATYPE(tl::os::EnvListSeparator);

Q_DECLARE_METATYPE(tl::io::FileType);

Q_DECLARE_METATYPE(tl::timeline::AlphaBlend);
Q_DECLARE_METATYPE(tl::timeline::AudioBufferFrameCount);
Q_DECLARE_METATYPE(tl::timeline::Channels);
Q_DECLARE_METATYPE(tl::timeline::CompareMode);
Q_DECLARE_METATYPE(tl::timeline::FileSequenceAudio);
Q_DECLARE_METATYPE(tl::timeline::Loop);
Q_DECLARE_METATYPE(tl::timeline::Playback);
Q_DECLARE_METATYPE(tl::timeline::TimeAction);
Q_DECLARE_METATYPE(tl::timeline::TimerMode);
Q_DECLARE_METATYPE(tl::timeline::Transition);
Q_DECLARE_METATYPE(tl::timeline::YUVRange);
