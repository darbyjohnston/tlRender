// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <opentimelineio/version.h>

#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>

#include <iostream>

//! Convenience macro for making a class non-copyable.
#define TLR_NON_COPYABLE(CLASS) \
    CLASS(const CLASS&) = delete; \
    CLASS& operator = (const CLASS&) = delete

//! Convenience macro for getting a list of enums.
#define TLR_ENUM_VECTOR(ENUM) \
    std::vector<ENUM> get##ENUM##Enums();

//! Convenience macro for converting enums to strings.
#define TLR_ENUM_LABEL(ENUM) \
    std::vector<std::string> get##ENUM##Labels(); \
    std::string getLabel(ENUM)

//! Convenience macro for serializing enums.
#define TLR_ENUM_SERIALIZE(ENUM) \
    std::ostream& operator << (std::ostream&, ENUM); \
    std::istream& operator >> (std::istream&, ENUM&)

//! Implementation macro for getting a list of enums.
#define TLR_ENUM_VECTOR_IMPL(ENUM) \
    std::vector<ENUM> get##ENUM##Enums() \
    { \
        std::vector<ENUM> out; \
        for (std::size_t i = 0; i < static_cast<std::size_t>(ENUM::Count); ++i) \
        { \
            out.push_back(static_cast<ENUM>(i)); \
        } \
        return out; \
    }

//! Implementation macro for converting enums to strings.
#define TLR_ENUM_LABEL_IMPL(ENUM, ...) \
    std::vector<std::string> get##ENUM##Labels() \
    { \
        return { __VA_ARGS__ }; \
    } \
    \
    std::string getLabel(ENUM value) \
    { \
        const std::array<std::string, static_cast<std::size_t>(ENUM::Count)> data = { __VA_ARGS__ }; \
        return data[static_cast<std::size_t>(value)]; \
    }

//! Implementation macro for serializing enums.
#define TLR_ENUM_SERIALIZE_IMPL(PREFIX, ENUM, ...) \
    std::ostream& operator << (std::ostream& os, PREFIX::ENUM in) \
    { \
        os << PREFIX::get##ENUM##Labels()[static_cast<std::size_t>(in)]; \
        return os; \
    } \
    \
    std::istream& operator >> (std::istream& is, PREFIX::ENUM& out) \
    { \
        std::string s; \
        is >> s; \
        const auto labels = PREFIX::get##ENUM##Labels(); \
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
        out = static_cast<PREFIX::ENUM>(i - labels.begin()); \
        return is; \
    }

namespace tlr
{
    namespace otime = opentime::OPENTIME_VERSION;
    namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;

    std::ostream& operator << (std::ostream&, const otime::RationalTime&);
    std::ostream& operator << (std::ostream&, const otime::TimeRange&);
}
