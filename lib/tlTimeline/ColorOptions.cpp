// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/ColorOptions.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#if defined(TLRENDER_OCIO)
#include <OpenColorIO/OpenColorTransforms.h>
#endif // TLRENDER_OCIO

#include <algorithm>
#include <array>
#include <sstream>

#if defined(TLRENDER_OCIO)
namespace OCIO = OCIO_NAMESPACE;
#endif // TLRENDER_OCIO

namespace tl
{
    namespace timeline
    {
        DTK_ENUM_IMPL(
            LUTOrder,
            "PostColorConfig",
            "PreColorConfig");

        std::vector<std::string> getLUTFormatNames()
        {
            std::vector<std::string> out;
#if defined(TLRENDER_OCIO)
            for (int i = 0; i < OCIO::FileTransform::GetNumFormats(); ++i)
            {
                out.push_back(OCIO::FileTransform::GetFormatNameByIndex(i));
            }
#endif // TLRENDER_OCIO
            return out;
        }

        std::vector<std::string> getLUTFormatExtensions()
        {
            std::vector<std::string> out;
#if defined(TLRENDER_OCIO)
            for (int i = 0; i < OCIO::FileTransform::GetNumFormats(); ++i)
            {
                std::string extension = OCIO::FileTransform::GetFormatExtensionByIndex(i);
                if (!extension.empty() && extension[0] != '.')
                {
                    extension.insert(extension.begin(), '.');
                }
                out.push_back(extension);
            }
#endif // TLRENDER_OCIO
            return out;
        }

        void to_json(nlohmann::json& json, const OCIOOptions& value)
        {
            json["enabled"] = value.enabled;
            json["input"] = value.input;
            json["display"] = value.display;
            json["view"] = value.view;
            json["look"] = value.look;
        }

        void to_json(nlohmann::json& json, const LUTOptions& value)
        {
            json["enabled"] = value.enabled;
            json["fileName"] = value.fileName;
            json["order"] = to_string(value.order);
        }

        void from_json(const nlohmann::json& json, OCIOOptions& value)
        {
            json["enabled"].get_to(value.enabled);
            json["input"].get_to(value.input);
            json["display"].get_to(value.display);
            json["view"].get_to(value.view);
            json["look"].get_to(value.look);
        }

        void from_json(const nlohmann::json& json, LUTOptions& value)
        {
            json["enabled"].get_to(value.enabled);
            json["fileName"].get_to(value.fileName);
            from_string(json["order"].get<std::string>(), value.order);
        }
    }
}
