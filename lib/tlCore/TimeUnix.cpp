// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlCore/Time.h>

#include <time.h>

namespace tl
{
	namespace core
	{
		namespace time
		{
			void sleep(const std::chrono::microseconds& value)
			{
				const auto microseconds = value.count();
				const auto seconds = microseconds / 1000000;
				struct timespec t;
				t.tv_sec = seconds;
				t.tv_nsec = (microseconds - (seconds * 1000000)) * 1000;
				struct timespec out;
				nanosleep(&t, &out);
			}
		}
	}
}
