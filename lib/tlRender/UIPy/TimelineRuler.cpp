// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/TimelineRuler.h>

#include <tlRender/UI/TimelineRuler.h>

#include <ftk/Core/Context.h>

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
        void timelineRuler(nb::module_& m)
        {
            using namespace ui;

            nb::class_<TimelineRuler, ftk::IMouseWidget>(
                m, "TimelineRuler",
                "The time ruler a TimelineWidget draws above its timelines, "
                "with the player's in and out points, frame markers and "
                "cache. TimelineWidget makes and places its own; this is "
                "for a layout of one's own, which then sets the scale and "
                "scroll position as the timelines move.")
                .def(
                    nb::new_(&TimelineRuler::create),
                    nb::arg("context"),
                    nb::arg("itemData"),
                    nb::arg("parent") = nullptr)
                .def("setItemData", &TimelineRuler::setItemData)
                .def("setPlayer", &TimelineRuler::setPlayer, nb::arg("player").none())
                .def("setScale", &TimelineRuler::setScale)
                .def("setScrollPos", &TimelineRuler::setScrollPos)
                .def("setOffset", &TimelineRuler::setOffset)
                .def("setFrameMarkers", &TimelineRuler::setFrameMarkers)
                .def("setDisplayOptions", &TimelineRuler::setDisplayOptions)
                .def("setOptions", &TimelineRuler::setOptions)
                .def("setStopOnScrub", &TimelineRuler::setStopOnScrub)
                .def_prop_ro(
                    "observeScrub",
                    &TimelineRuler::observeScrub)
                .def("timeToPos", &TimelineRuler::timeToPos);
        }
    }
}

