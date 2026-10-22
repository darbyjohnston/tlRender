// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/GL/Export.h>
#include <tlRender/Timeline/ColorOptions.h>
#include <tlRender/Timeline/CompareOptions.h>
#include <tlRender/Timeline/DisplayOptions.h>
#include <tlRender/Timeline/Video.h>

#include <ftk/Core/Image.h>
#include <ftk/Core/RenderOptions.h>

#include <memory>
#include <vector>

namespace ftk
{
    class Context;
}

namespace tl
{
    namespace gl
    {
        //! Renders video frames to an image, without a window to show them
        //! in: a script's way to get what a viewport would draw, a tile or
        //! wipe compare included, into a file or an array.
        //!
        //! The context is a hidden window's. On Linux with no display, the
        //! SDL_VIDEO_DRIVER=offscreen environment variable makes that an EGL
        //! context with no window system at all, which is what a container,
        //! a render farm node or a continuous integration job has.
        //!
        //! The video alone is drawn: no background, grid or other overlay.
        //! Where no frame covers the image, as between tiles, it is
        //! transparent.
        class TL_GL_API_TYPE OffscreenRender : public std::enable_shared_from_this<OffscreenRender>
        {
            FTK_NON_COPYABLE(OffscreenRender);

        protected:
            void _init(const std::shared_ptr<ftk::Context>&);

            OffscreenRender();

        public:
            TL_GL_API ~OffscreenRender();

            //! Create a new renderer. Throws when there is no OpenGL context
            //! to be had.
            TL_GL_API static std::shared_ptr<OffscreenRender> create(
                const std::shared_ptr<ftk::Context>&);

            //! Set the OpenColorIO options.
            TL_GL_API void setOCIOOptions(const OCIOOptions&);

            //! Set the LUT options.
            TL_GL_API void setLUTOptions(const LUTOptions&);

            //! \name Texture Cache
            //! Frames rendered again are kept as textures, so that rendering
            //! the same frames once more -- with other comparison options,
            //! a different size, another look -- does not copy them in
            //! again. A frame is kept from the second render in a row that
            //! has it; frames each rendered once, as a sequence is, are not
            //! kept. Frames are told apart by the image, not by its pixels:
            //! an image whose pixels are changed in place renders as it was
            //! until the cache is cleared.
            ///@{

            //! Get the most bytes of images that are kept.
            TL_GL_API size_t getCacheByteCount() const;

            //! Set the most bytes of images that are kept. Zero keeps none.
            TL_GL_API void setCacheByteCount(size_t);

            //! Clear the cache.
            TL_GL_API void clearCache();

            ///@}

            //! Render video frames, the first and then one for each timeline
            //! compared with it, as a Player's current video has them.
            //!
            //! The image is the size the comparison needs for the frames at
            //! their own resolution -- side by side, two of them across --
            //! or, given a size, that size with the comparison fitted into
            //! it. The image type is what is read back: RGBA_U8 for a file
            //! or a look, RGBA_F32 to measure values past one where OpenGL
            //! can read it back (ftk::gl::OffscreenBuffer::canRead() says;
            //! OpenGL ES reads back bytes only). A type that cannot be read
            //! back throws.
            TL_GL_API std::shared_ptr<ftk::Image> render(
                const std::vector<VideoFrame>&,
                const CompareOptions& = CompareOptions(),
                const ftk::Size2I& size = ftk::Size2I(),
                const std::vector<ftk::ImageOptions>& = {},
                const std::vector<DisplayOptions>& = {},
                ftk::ImageType = ftk::ImageType::RGBA_U8);

        private:
            FTK_PRIVATE();
        };
    }
}
