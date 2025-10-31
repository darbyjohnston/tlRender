// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlQt/Init.h>

namespace tl
{
    //! Qt QWidget support
    namespace qtwidget
    {
        //! Initialize the library. This needs to be called before the Qt
        //! application is created.
        void init(
            const std::shared_ptr<ftk::Context>&,
            qt::DefaultSurfaceFormat);

        //! Initialize the fonts. This needs to be called after the Qt
        //! application is created.
        void initFonts(const std::shared_ptr<ftk::Context>& context);
    }
}
