// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/SequenceIO.h>

#include <dtk/core/Format.h>

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
            out["SequenceIO/DefaultSpeed"] = dtk::Format("{0}").arg(value.defaultSpeed);
            out["SequenceIO/ThreadCount"] = dtk::Format("{0}").arg(value.threadCount);
            return out;
        }

        void to_json(nlohmann::json& json, const SequenceOptions& value)
        {
            json["defaultSpeed"] = value.defaultSpeed;
            json["threadCount"] = value.threadCount;
        }

        void from_json(const nlohmann::json& json, SequenceOptions& value)
        {
            json["defaultSpeed"].get_to(value.defaultSpeed);
            json["threadCount"].get_to(value.threadCount);
        }
    }
}
