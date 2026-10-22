// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/ItemOptions.h>

#include <tlRender/UI/ItemOptions.h>

#include <ftk/CorePy/Bindings.h>

#include <ftk/Core/Context.h>

#include <tlRender/CorePy/Function.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/list.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/optional.h>
#include <tlRender/TimelinePy/OTIOCasters.h>

#include <nanobind/stl/array.h>
#include <nanobind/stl/set.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/filesystem.h>

namespace nb = nanobind;

namespace tl
{
    namespace python
    {
        void itemOptions(nb::module_& m)
        {
            using namespace ui;
            
            FTK_ENUM_PY(m, InOutDisplay);
            FTK_ENUM_BIND(m, InOutDisplay);

            FTK_ENUM_PY(m, CacheDisplay);
            FTK_ENUM_BIND(m, CacheDisplay);

            FTK_ENUM_PY(m, WaveformPrim);
            FTK_ENUM_BIND(m, WaveformPrim);

            nb::class_<ItemData>(
                m, "ItemData",
                "What the items of a timeline widget share: the speed, the "
                "directory relative media paths are read against, the "
                "options and the time units model. TimelineWidget makes "
                "its own; a TimelineRuler is given one.")
                .def(nb::init())
                .def_rw("speed", &ItemData::speed)
                .def_rw("dir", &ItemData::dir)
                .def_rw("options", &ItemData::options)
                .def_rw("timeUnitsModel", &ItemData::timeUnitsModel)
                .def_rw(
                    "toMediaTime",
                    &ItemData::toMediaTime,
                    "How to turn a time on the timeline into the time shown "
                    "for it; see TimeMap. Only positions go through it, a "
                    "duration being a length either way. Unset, the two "
                    "are the same.");

            nb::class_<ItemOptions>(
                m, "ItemOptions",
                "Options for timeline items. inputEnabled off makes them "
                "ignore the mouse and keyboard, for a timeline that is only "
                "looked at.")
                .def(nb::init())
                .def_rw("inputEnabled", &ItemOptions::inputEnabled)
                .def(nanobind::self == nanobind::self)
                .def(nanobind::self != nanobind::self);

            nb::class_<Marker>(
                m, "Marker",
                "A named, colored range drawn on the timeline.")
                .def(nb::init())
                .def_rw("name", &Marker::name)
                .def_rw("color", &Marker::color)
                .def_rw("range", &Marker::range)
                .def(nanobind::self == nanobind::self)
                .def(nanobind::self != nanobind::self);

            nb::class_<DisplayOptions>(
                m, "DisplayOptions",
                "How a timeline widget draws: in and out points, the cache, "
                "thumbnails and waveforms and their sizes, clip colors, and "
                "the OCIO and LUT options thumbnails are made with.")
                .def(nb::init())
                .def_rw("inOutDisplay", &DisplayOptions::inOutDisplay)
                .def_rw("cacheDisplay", &DisplayOptions::cacheDisplay)
                .def_rw("minimize", &DisplayOptions::minimize)
                .def_rw("clipColors", &DisplayOptions::clipColors)
                .def_rw("thumbnails", &DisplayOptions::thumbnails)
                .def_rw("thumbnailHeight", &DisplayOptions::thumbnailHeight)
                .def_rw("waveforms", &DisplayOptions::waveforms)
                .def_rw("waveformWidth", &DisplayOptions::waveformWidth)
                .def_rw("waveformHeight", &DisplayOptions::waveformHeight)
                .def_rw("waveformPrim", &DisplayOptions::waveformPrim)
                .def_rw("clipRectScale", &DisplayOptions::clipRectScale)
                .def_rw("ocio", &DisplayOptions::ocio)
                .def_rw("lut", &DisplayOptions::lut)
                .def(nanobind::self == nanobind::self)
                .def(nanobind::self != nanobind::self);

            ftk::python::observable<DisplayOptions>(m, "DisplayOptions");
            
        }
    }
}

