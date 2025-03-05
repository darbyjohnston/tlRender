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
        bool OCIOOptions::operator == (const OCIOOptions& other) const
        {
            return
                enabled == other.enabled &&
                fileName == other.fileName &&
                input == other.input &&
                display == other.display &&
                view == other.view &&
                look == other.look;
        }

        bool OCIOOptions::operator != (const OCIOOptions& other) const
        {
            return !(*this == other);
        }

        DTK_ENUM_IMPL(
            LUTOrder,
            "PostColorConfig",
            "PreColorConfig");

        bool LUTOptions::operator == (const LUTOptions& other) const
        {
            return
                enabled == other.enabled &&
                fileName == other.fileName &&
                order == other.order;
        }

        bool LUTOptions::operator != (const LUTOptions& other) const
        {
            return !(*this == other);
        }

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
            json.at("enabled").get_to(value.enabled);
            json.at("input").get_to(value.input);
            json.at("display").get_to(value.display);
            json.at("view").get_to(value.view);
            json.at("look").get_to(value.look);
        }

        void from_json(const nlohmann::json& json, LUTOptions& value)
        {
            json.at("enabled").get_to(value.enabled);
            json.at("fileName").get_to(value.fileName);
            from_string(json.at("order").get<std::string>(), value.order);
        }
    }
}
