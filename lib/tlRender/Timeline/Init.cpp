// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/Init.h>

#include <tlRender/Timeline/AudioSystem.h>
#include <tlRender/Timeline/Player.h>
#include <tlRender/Timeline/System.h>

#include <tlRender/IO/System.h>

#include <tlRender/Core/Version.h>

#include <ftk/GL/Init.h>
#include <ftk/Core/Context.h>
#include <ftk/Core/DiagSystem.h>
#include <ftk/Core/Format.h>

#include <opentimelineio/typeRegistry.h>

namespace tl
{
    void init(const std::shared_ptr<ftk::Context>& context)
    {
        auto logSystem = context->getLogSystem();
        logSystem->print(
            "tl::init",
            ftk::Format("tlRender version: {0}").arg(TLRENDER_VERSION_FULL));

        auto diagSystem = context->getSystem<ftk::DiagSystem>();
        diagSystem->addSampler(
            "Memory/Audio: {0}MB",
            [] { return tl::Audio::getTotalByteCount() / ftk::megabyte; });
        diagSystem->addSampler(
            "Timeline objects/Players: {0}",
            [] { return tl::Player::getObjectCount(); });
        diagSystem->addSampler(
            "Timeline objects/Timelines: {0}",
            [] { return tl::Timeline::getObjectCount(); });
        diagSystem->addSampler(
            "Timeline objects/I/O: {0}",
            [] { return tl::IIO::getObjectCount(); });
        diagSystem->addSampler(
            "Timeline objects/Audio: {0}",
            [] { return tl::Audio::getObjectCount(); });

        ftk::gl::init(context);

        AudioSystem::create(context);
        ReadSystem::create(context);
        WriteSystem::create(context);

        System::create(context);
    }
}
