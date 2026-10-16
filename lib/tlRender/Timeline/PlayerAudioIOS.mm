// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlRender/Timeline/PlayerPrivate.h>

#import <AVFoundation/AVFoundation.h>

namespace tl
{
    double getAudioOutputLatency()
    {
        AVAudioSession* session = [AVAudioSession sharedInstance];
        return session.outputLatency + session.IOBufferDuration;
    }
}
