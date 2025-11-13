// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimeline/ColorOptions.h>

#include <ftk/Core/Error.h>
#include <ftk/Core/String.h>

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
        FTK_ENUM_IMPL(
            OCIOConfig,
            "Built In",
            "Environment Variable",
            "File");

        bool OCIOOptions::operator == (const OCIOOptions& other) const
        {
            return
                enabled == other.enabled &&
                config == other.config &&
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

        FTK_ENUM_IMPL(
            LUTOrder,
            "Post Color Config",
            "Pre Color Config");

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
            json["Enabled"] = value.enabled;
            json["Config"] = to_string(value.config);
            json["FileName"] = value.fileName;
            json["Input"] = value.input;
            json["Display"] = value.display;
            json["View"] = value.view;
            json["Look"] = value.look;
        }

        void to_json(nlohmann::json& json, const LUTOptions& value)
        {
            json["Enabled"] = value.enabled;
            json["FileName"] = value.fileName;
            json["Order"] = to_string(value.order);
        }

        void from_json(const nlohmann::json& json, OCIOOptions& value)
        {
            json.at("Enabled").get_to(value.enabled);
            from_string(json.at("Config").get<std::string>(), value.config);
            json.at("FileName").get_to(value.fileName);
            json.at("Input").get_to(value.input);
            json.at("Display").get_to(value.display);
            json.at("View").get_to(value.view);
            json.at("Look").get_to(value.look);
        }

        void from_json(const nlohmann::json& json, LUTOptions& value)
        {
            json.at("Enabled").get_to(value.enabled);
            json.at("FileName").get_to(value.fileName);
            from_string(json.at("Order").get<std::string>(), value.order);
        }
    }
}
