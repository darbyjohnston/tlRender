// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGL/State.h>

#include <tlGlad/gl.h>

namespace tl
{
    namespace gl
    {
        struct SetAndRestore::Private
        {
            unsigned int id = 0;
            GLboolean prev = GL_FALSE;
        };

        SetAndRestore::SetAndRestore(unsigned int id, bool value) :
            _p(new Private)
        {
            _p->id = id;

            glGetBooleanv(id, &_p->prev);

            if (value)
            {
                glEnable(id);
            }
            else
            {
                glDisable(id);
            }
        }

        SetAndRestore::~SetAndRestore()
        {
            if (_p->prev)
            {
                glEnable(_p->id);
            }
            else
            {
                glDisable(_p->id);
            }
        }
    }
}
