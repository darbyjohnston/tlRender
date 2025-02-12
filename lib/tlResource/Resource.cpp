// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlResource/Resource.h>

#include <Icons/Color.h>
#include <Icons/CompareA.h>
#include <Icons/CompareB.h>
#include <Icons/CompareDifference.h>
#include <Icons/CompareHorizontal.h>
#include <Icons/CompareOverlay.h>
#include <Icons/Compare.h>
#include <Icons/CompareTile.h>
#include <Icons/CompareVertical.h>
#include <Icons/CompareWipe.h>
#include <Icons/Devices.h>
#include <Icons/FileOpenSeparateAudio.h>
#include <Icons/Files.h>
#include <Icons/Hidden.h>
#include <Icons/Info.h>
#include <Icons/Messages.h>
#include <Icons/tlRender.h>
#include <Icons/View.h>
#include <Icons/Visible.h>
#include <Icons/WindowFullScreen.h>
#include <Icons/WindowSecondary.h>

#include <cstring>
#include <map>

using namespace dtk;

namespace tl
{
    namespace
    {
        const std::map<std::string, const std::vector<uint8_t>* > iconResources =
        {
            { "Color", &Color_svg },
            { "CompareA", &CompareA_svg },
            { "CompareB", &CompareB_svg },
            { "CompareDifference", &CompareDifference_svg },
            { "CompareHorizontal", &CompareHorizontal_svg },
            { "CompareOverlay", &CompareOverlay_svg },
            { "Compare", &Compare_svg },
            { "CompareTile", &CompareTile_svg },
            { "CompareVertical", &CompareVertical_svg },
            { "CompareWipe", &CompareWipe_svg },
            { "Devices", &Devices_svg },
            { "FileOpenSeparateAudio", &FileOpenSeparateAudio_svg },
            { "Files", &Files_svg },
            { "Hidden", &Hidden_svg },
            { "Info", &Info_svg },
            { "Messages", &Messages_svg },
            { "tlRender", &tlRender_svg },
            { "View", &View_svg },
            { "Visible", &Visible_svg },
            { "WindowFullScreen", &WindowFullScreen_svg },
            { "WindowSecondary", &WindowSecondary_svg }
        };
    }

    std::vector<std::string> getIconResources()
    {
        std::vector<std::string> out;
        for (const auto& resource : iconResources)
        {
            out.push_back(resource.first);
        }
        return out;
    }

    std::vector<uint8_t> getIconResource(const std::string& name)
    {
        std::vector<uint8_t> out;
        const auto i = iconResources.find(name);
        if (i != iconResources.end())
        {
            out.resize(i->second->size());
            memcpy(out.data(), i->second->data(), i->second->size());
        }
        return out;
    }
}
