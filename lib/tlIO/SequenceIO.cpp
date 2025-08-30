// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/SequenceIO.h>

#include <feather-tk/core/Format.h>

namespace tl
{
    namespace io
    {
        bool SequenceOptions::operator == (const SequenceOptions& other) const
        {
            return
                defaultSpeed == other.defaultSpeed &&
                threadCount == other.threadCount;
        }

        bool SequenceOptions::operator != (const SequenceOptions& other) const
        {
            return !(*this == other);
        }

        Options getOptions(const SequenceOptions& value)
        {
            Options out;
            out["SequenceIO/DefaultSpeed"] = ftk::Format("{0}").arg(value.defaultSpeed);
            out["SequenceIO/ThreadCount"] = ftk::Format("{0}").arg(value.threadCount);
            return out;
        }

        void to_json(nlohmann::json& json, const SequenceOptions& value)
        {
            json["DefaultSpeed"] = value.defaultSpeed;
            json["ThreadCount"] = value.threadCount;
        }

        void from_json(const nlohmann::json& json, SequenceOptions& value)
        {
            json.at("DefaultSpeed").get_to(value.defaultSpeed);
            json.at("ThreadCount").get_to(value.threadCount);
        }
    }
}
