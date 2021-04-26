// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace core
    {
        //! This function provides an assert (for convenience use the TLR_ASSERT macro).
        void _assert(const char* file, int line);
    }
}

//! This macro provides an assert.
#if defined(TLR_ASSERT)
#undef TLR_ASSERT
#define TLR_ASSERT(value) \
    if (!(value)) \
        tlr::core::_assert(__FILE__, __LINE__)
#else
#define TLR_ASSERT(value)
#endif
