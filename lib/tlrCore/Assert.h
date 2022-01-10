// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

//! tlRender.
namespace tlr
{
    //! Core functionality.
    namespace core
    {
        //! Assert (for convenience use the TLR_ASSERT macro).
        void _assert(const char* file, int line);
    }
}

//! Assert macro.
#if defined(TLR_ASSERT)
#undef TLR_ASSERT
#define TLR_ASSERT(value) \
    if (!(value)) \
        tlr::core::_assert(__FILE__, __LINE__)
#else
#define TLR_ASSERT(value)
#endif
