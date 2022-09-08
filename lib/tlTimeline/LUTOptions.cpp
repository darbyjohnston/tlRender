// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/LUTOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <OpenColorIO/OpenColorTransforms.h>

#include <algorithm>
#include <array>

namespace OCIO = OCIO_NAMESPACE;

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            LUTOrder,
            "PostColorConfig",
            "PreColorConfig");
        TLRENDER_ENUM_SERIALIZE_IMPL(LUTOrder);

        std::vector<std::string> getLUTFormatNames()
        {
            std::vector<std::string> out;
            for (int i = 0; i < OCIO::FileTransform::GetNumFormats(); ++i)
            {
                out.push_back(OCIO::FileTransform::GetFormatNameByIndex(i));
            }
            return out;
        }

        std::vector<std::string> getLUTFormatExtensions()
        {
            std::vector<std::string> out;
            for (int i = 0; i < OCIO::FileTransform::GetNumFormats(); ++i)
            {
                std::string extension = OCIO::FileTransform::GetFormatExtensionByIndex(i);
                if (!extension.empty() && extension[0] != '.')
                {
                    extension.insert(extension.begin(), '.');
                }
                out.push_back(extension);
            }
            return out;
        }
    }
}
