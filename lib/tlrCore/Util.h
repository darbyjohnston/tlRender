// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <opentimelineio/version.h>

#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>

#include <iostream>

//! Convenience macro for making a class non-copyable.
#define TLR_NON_COPYABLE(NAME) \
    NAME(const NAME&) = delete; \
    NAME& operator = (const NAME&) = delete

//! Convenience macro for converting enums to strings.
#define TLR_ENUM_LABEL(NAME) \
    std::vector<std::string> get##NAME##Labels(); \
    std::string getLabel(NAME)

//! Convenience macro for serializing enums.
#define TLR_ENUM_SERIALIZE(NAME) \
    std::ostream& operator << (std::ostream&, NAME); \
    std::istream& operator >> (std::istream&, NAME&)

//! Implementation macro for converting enums to strings.
#define TLR_ENUM_LABEL_IMPL(NAME, ...) \
    std::vector<std::string> get##NAME##Labels() \
    { \
        return { __VA_ARGS__ }; \
    } \
    \
    std::string getLabel(NAME value) \
    { \
        const std::array<std::string, static_cast<size_t>(NAME::Count)> data = { __VA_ARGS__ }; \
        return data[static_cast<size_t>(value)]; \
    }

//! Implementation macro for serializing enums.
#define TLR_ENUM_SERIALIZE_IMPL(PREFIX, NAME, ...) \
    std::ostream& operator << (std::ostream& os, PREFIX::NAME in) \
    { \
        os << PREFIX::get##NAME##Labels()[static_cast<size_t>(in)]; \
        return os; \
    } \
    \
    std::istream& operator >> (std::istream& is, PREFIX::NAME& out) \
    { \
        std::string s; \
        is >> s; \
        const auto labels = PREFIX::get##NAME##Labels(); \
        const auto i = std::find_if( \
            labels.begin(), \
            labels.end(), \
            [s](const std::string& value) \
            { \
                return string::compareNoCase(s, value); \
            }); \
        if (i == labels.end()) \
        { \
            throw ParseError(); \
        } \
        out = static_cast<PREFIX::NAME>(i - labels.begin()); \
        return is; \
    }

namespace tlr
{
    namespace otime = opentime::OPENTIME_VERSION;
    namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;

    std::ostream& operator << (std::ostream&, const otime::RationalTime&);
    std::ostream& operator << (std::ostream&, const otime::TimeRange&);
}
