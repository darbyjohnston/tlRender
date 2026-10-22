// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/GLPy/Bindings.h>

#include <tlRender/GL/OffscreenRender.h>

#include <ftk/Core/Context.h>

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

namespace nb = nanobind;

namespace tl
{
    namespace python
    {
        void offscreenRender(nb::module_& m)
        {
            nb::class_<gl::OffscreenRender>(
                m, "OffscreenRender",
                "Renders video frames to an image without a window: what a "
                "viewport would draw, a tile or wipe compare included, as an "
                "ftk.Image to write with WriteSystem or read with "
                "numpy.asarray(). The video alone is drawn, with no "
                "background or overlays; where no frame covers the image it "
                "is transparent. The OpenGL context is a hidden window's. On "
                "Linux with no display -- a container, a render farm, "
                "continuous integration -- set SDL_VIDEO_DRIVER=offscreen in "
                "the environment, which makes it an EGL context with no "
                "window system; that needs Mesa's EGL and DRI drivers, and "
                "uses the GPU where there is one and llvmpipe where there is "
                "not. Keep one and render the same frames again with other "
                "options -- a wipe position, a size, a look -- and the frames "
                "stay on the GPU instead of being copied in each time; see "
                "cacheByteCount and clearCache().")
                .def(
                    nb::new_(&gl::OffscreenRender::create),
                    nb::arg("context"))
                .def(
                    "setOCIOOptions",
                    &gl::OffscreenRender::setOCIOOptions,
                    nb::arg("options"))
                .def(
                    "setLUTOptions",
                    &gl::OffscreenRender::setLUTOptions,
                    nb::arg("options"))
                .def_prop_rw(
                    "cacheByteCount",
                    &gl::OffscreenRender::getCacheByteCount,
                    &gl::OffscreenRender::setCacheByteCount,
                    "The most bytes of frames kept as textures, so that "
                    "rendering the same frames again -- with other compare "
                    "options, another size or look -- does not copy them in "
                    "again. Frames are told apart by the image, not its "
                    "pixels. Zero keeps none, for frames each rendered once.")
                .def(
                    "clearCache",
                    &gl::OffscreenRender::clearCache,
                    "Clear the frames kept as textures: after changing an "
                    "image's pixels in place, which otherwise renders as it "
                    "was.")
                .def(
                    "render",
                    &gl::OffscreenRender::render,
                    nb::arg("videoFrames"),
                    nb::arg("compareOptions") = CompareOptions(),
                    nb::arg("size") = ftk::Size2I(),
                    nb::arg("imageOptions") = std::vector<ftk::ImageOptions>(),
                    nb::arg("displayOptions") = std::vector<DisplayOptions>(),
                    nb::arg("type") = ftk::ImageType::RGBA_U8,
                    "Render video frames -- the first and then one for each "
                    "compared timeline, as Player.currentVideo has them -- "
                    "laid out by compareOptions. The image is the size the "
                    "comparison needs at the frames' own resolution, or the "
                    "size given with the comparison fitted into it. type is "
                    "what is read back: RGBA_U8 for a file or a look, "
                    "RGBA_F32 to measure values past one where OpenGL can "
                    "read it back -- OpenGL ES reads back bytes only, and a "
                    "type that cannot be read back raises.");
        }
    }
}
