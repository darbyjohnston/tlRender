// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/GLPy/Bindings.h>

namespace nb = nanobind;

namespace tl
{
    namespace python
    {
        void glBind(nb::module_& m)
        {
            // In the top level module rather than one of its own: what it
            // draws is the timeline's, and a script asks for it beside the
            // Player whose frames it draws.
            offscreenRender(m);
        }
    }
}
