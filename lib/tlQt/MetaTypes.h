// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>
#include <tlTimeline/TimeUnits.h>

#include <tlCore/AudioSystem.h>

#include <QMetaType>

Q_DECLARE_METATYPE(tl::audio::DataType);

Q_DECLARE_METATYPE(tl::io::FileType);

Q_DECLARE_METATYPE(tl::timeline::CompareMode);
Q_DECLARE_METATYPE(tl::timeline::CompareTimeMode);
Q_DECLARE_METATYPE(tl::timeline::FileSequenceAudio);
Q_DECLARE_METATYPE(tl::timeline::Loop);
Q_DECLARE_METATYPE(tl::timeline::Playback);
Q_DECLARE_METATYPE(tl::timeline::TimeAction);
Q_DECLARE_METATYPE(tl::timeline::TimeUnits);
Q_DECLARE_METATYPE(tl::timeline::Transition);
