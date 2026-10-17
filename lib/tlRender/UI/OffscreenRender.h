// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/UI/Export.h>

#include <ftk/Core/Image.h>
#include <ftk/Core/Util.h>

#include <memory>

namespace ftk
{
    class Context;
}

namespace tl
{
    class IRender;

    namespace ui
    {
        //! Somewhere to render frames off the screen, and read them back,
        //! with whichever renderer the application draws with: OpenGL, or
        //! the GPU renderer where it was asked for and started (on a
        //! tablet, the only one there is). An export or a bake asks for
        //! one of these rather than choosing, so it has no renderer code
        //! of its own.
        //!
        //! A frame is begin(), drawing with getRender(), end(), and read(),
        //! in one go: with OpenGL the buffer is bound from begin() until
        //! the read, and what was bound before is put back after it. The
        //! OpenGL renderer needs a current context, as any OpenGL drawing
        //! does.
        class TL_UI_API_TYPE OffscreenRender : public std::enable_shared_from_this<OffscreenRender>
        {
            FTK_NON_COPYABLE(OffscreenRender);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const ftk::Size2I&,
                ftk::ImageType colorBuffer);

            OffscreenRender();

        public:
            TL_UI_API ~OffscreenRender();

            //! Create a new offscreen render of the given size. The color
            //! buffer is what the pictures are held in while they are
            //! drawn; see Viewport::getColorBuffer().
            TL_UI_API static std::shared_ptr<OffscreenRender> create(
                const std::shared_ptr<ftk::Context>&,
                const ftk::Size2I&,
                ftk::ImageType colorBuffer);

            //! Get whether read() can give an image of this type with the
            //! renderer in use.
            TL_UI_API static bool canRead(ftk::ImageType);

            //! Get the size.
            TL_UI_API const ftk::Size2I& getSize() const;

            //! Get the renderer to draw with, between begin() and end().
            TL_UI_API const std::shared_ptr<IRender>& getRender() const;

            //! Begin a frame.
            TL_UI_API void begin();

            //! End a frame.
            TL_UI_API void end();

            //! Read the frame back, as the given image.
            TL_UI_API std::shared_ptr<ftk::Image> read(const ftk::ImageInfo&);

        private:
            FTK_PRIVATE();
        };
    }
}
