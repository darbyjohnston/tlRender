// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#if defined(TLRENDER_API_GL_4_1)
#include <tlGlad_GL_4_1/gl.h>
#elif defined(TLRENDER_API_GL_4_1_Debug)
#include <tlGlad_GL_4_1_Debug/gl.h>
#elif defined(TLRENDER_API_GLES_2)
#include <tlGlad_GLES_2/gles2.h>
#endif
