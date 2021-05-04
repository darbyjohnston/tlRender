// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Time.h>

#if defined(_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

namespace tlr
{
    namespace time
    {
        void sleep(const std::chrono::microseconds& value)
        {
#if defined(_WINDOWS)
            if (HANDLE h = CreateWaitableTimer(NULL, TRUE, NULL))
            {
                LARGE_INTEGER l;
                l.QuadPart = -std::chrono::duration_cast<std::chrono::nanoseconds>(value).count() / 100;
                if (SetWaitableTimer(h, &l, 0, NULL, NULL, FALSE))
                {
                    WaitForSingleObject(h, INFINITE);
                }
                CloseHandle(h);
            }
#else
            const auto microseconds = value.count();
            const auto seconds = microseconds / 1000000;
            struct timespec t;
            t.tv_sec = seconds;
            t.tv_nsec = (microseconds - (seconds * 1000000)) * 1000;
            struct timespec out;
            nanosleep(&t, &out);
#endif
        }
    }
}
