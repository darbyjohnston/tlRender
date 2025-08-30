// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>
#include <tlTimeline/TimeUnits.h>

#include <tlCore/AudioSystem.h>

#include <feather-tk/core/RenderOptions.h>

#include <QMetaType>

Q_DECLARE_METATYPE(ftk::AlphaBlend);
Q_DECLARE_METATYPE(ftk::ChannelDisplay);
Q_DECLARE_METATYPE(ftk::ImageType);
Q_DECLARE_METATYPE(ftk::ImageFilter);
Q_DECLARE_METATYPE(ftk::InputVideoLevels);
Q_DECLARE_METATYPE(ftk::Size2I);

Q_DECLARE_METATYPE(tl::audio::DataType);

Q_DECLARE_METATYPE(tl::io::FileType);

Q_DECLARE_METATYPE(tl::timeline::Compare);
Q_DECLARE_METATYPE(tl::timeline::CompareTime);
Q_DECLARE_METATYPE(tl::timeline::ImageSequenceAudio);
Q_DECLARE_METATYPE(tl::timeline::Loop);
Q_DECLARE_METATYPE(tl::timeline::Playback);
Q_DECLARE_METATYPE(tl::timeline::TimeAction);
Q_DECLARE_METATYPE(tl::timeline::TimeUnits);
Q_DECLARE_METATYPE(tl::timeline::Transition);
