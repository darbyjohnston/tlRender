// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

//! Convenience macro for making a class non-copyable.
#define TLR_NON_COPYABLE(CLASS) \
    CLASS(const CLASS&) = delete; \
    CLASS& operator = (const CLASS&) = delete

//! Convenience macro for private implementations.
//! 
//! Required includes:
//! * memory
#define TLR_PRIVATE() \
    struct Private; \
    std::unique_ptr<Private> _p

//! Define a variable, "p", that references the private implementation.
#define TLR_PRIVATE_P() \
    auto& p = *_p

//! Convenience macro for enum utilities.
//! 
//! Required includes:
//! * string
//! * vector
#define TLR_ENUM(ENUM) \
    std::vector<ENUM> get##ENUM##Enums(); \
    std::vector<std::string> get##ENUM##Labels(); \
    std::string getLabel(ENUM)

//! Convenience macro for serializing enums.
//! 
//! Required includes:
//! * iostream
#define TLR_ENUM_SERIALIZE(ENUM) \
    std::ostream& operator << (std::ostream&, ENUM); \
    std::istream& operator >> (std::istream&, ENUM&)

//! Implementation macro for enum utilities.
#define TLR_ENUM_IMPL(ENUM, ...) \
    std::vector<ENUM> get##ENUM##Enums() \
    { \
        std::vector<ENUM> out; \
        for (std::size_t i = 0; i < static_cast<std::size_t>(ENUM::Count); ++i) \
        { \
            out.push_back(static_cast<ENUM>(i)); \
        } \
        return out; \
    } \
    \
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
//! 
//! Required includes:
//! * tlrCore/Error.h
//! * tlrCore/String.h
//! * algorithm
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
            throw core::ParseError(); \
        } \
        out = static_cast<PREFIX::ENUM>(i - labels.begin()); \
        return is; \
    }
