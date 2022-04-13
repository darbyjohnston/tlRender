// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlBMD/PlaybackDevice.h>

#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>

#include "platform.h"

#include <iostream>

namespace tl
{
    namespace bmd
    {
        struct PlaybackDevice::Private
        {
            int deviceIndex = -1;
        };

        void PlaybackDevice::_init(int deviceIndex, const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            int count = 0;
            IDeckLinkIterator* deckLinkIterator = nullptr;
            if (GetDeckLinkIterator(&deckLinkIterator) == S_OK)
            {
                IDeckLink* deckLink = nullptr;
                while (deckLinkIterator->Next(&deckLink) == S_OK)
                {
                    if (count == deviceIndex)
                    {
                        p.deviceIndex = deviceIndex;

                        dlstring_t modelName;
                        deckLink->GetModelName(&modelName);
                        context->log(
                            "tl::bmd::PlaybackDevice",
                            string::Format("Using device {0}: {1}").
                            arg(deviceIndex).
                            arg(DlToStdString(modelName)));
                        DeleteString(modelName);
                        
                        deckLink->Release();
                        
                        break;
                    }

                    ++count;
                }
            }
            if (deckLinkIterator)
            {
                deckLinkIterator->Release();
            }
        }

        PlaybackDevice::PlaybackDevice() :
            _p(new Private)
        {}

        PlaybackDevice::~PlaybackDevice()
        {}

        std::shared_ptr<PlaybackDevice> PlaybackDevice::create(
            int deviceIndex,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<PlaybackDevice>(new PlaybackDevice);
            out->_init(deviceIndex, context);
            return out;
        }

        void PlaybackDevice::display(const std::shared_ptr<imaging::Image>& image)
        {
            TLRENDER_P();

            std::cout << "display " << p.deviceIndex << std::endl;
        }
    }
}
