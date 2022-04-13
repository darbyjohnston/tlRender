// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlBMD/DeviceInfo.h>

#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>

#include "platform.h"

namespace tl
{
    namespace bmd
    {
        bool DeviceInfo::operator == (const DeviceInfo& other) const
        {
            return model == other.model;
        }

        struct DeviceInfoSystem::Private
        {
            std::shared_ptr<observer::List<DeviceInfo> > info;
        };

        void DeviceInfoSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::bmd::DeviceInfoSystem", context);

            TLRENDER_P();

            p.info = observer::List<DeviceInfo>::create();

            std::vector<DeviceInfo> info;
            IDeckLinkIterator* deckLinkIterator = nullptr;
            if (GetDeckLinkIterator(&deckLinkIterator) == S_OK)
            {
                IDeckLink* deckLink = nullptr;
                while (deckLinkIterator->Next(&deckLink) == S_OK)
                {
                    DeviceInfo deviceInfo;

                    dlstring_t modelName;
                    deckLink->GetModelName(&modelName);
                    deviceInfo.model = DlToStdString(modelName);
                    DeleteString(modelName);

                    deckLink->Release();

                    info.push_back(deviceInfo);

                    context->log(
                        "tl::bmd::DeviceInfoSystem",
                        string::Format("Found device: {0}").arg(deviceInfo.model));
                }
            }
            if (deckLinkIterator)
            {
                deckLinkIterator->Release();
            }
            p.info->setIfChanged(info);
        }

        DeviceInfoSystem::DeviceInfoSystem() :
            _p(new Private)
        {}

        DeviceInfoSystem::~DeviceInfoSystem()
        {}

        std::shared_ptr<DeviceInfoSystem> DeviceInfoSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<DeviceInfoSystem>(new DeviceInfoSystem);
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IList<DeviceInfo> > DeviceInfoSystem::observeDeviceInfo() const
        {
            return _p->info;
        }
    }
}
