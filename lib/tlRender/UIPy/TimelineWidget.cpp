// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/TimelineWidget.h>

#include <tlRender/UI/TimelineWidget.h>

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
        void timelineWidget(nb::module_& m)
        {
            using namespace ui;

            nb::class_<TimelineWidget, ftk::IWidget>(
                m, "TimelineWidget",
                "Draws a Player's timeline -- tracks, clips, transitions, "
                "markers, thumbnails and audio waveforms -- under a ruler "
                "that scrubs it, with the timelines the player compares "
                "against drawn beside it. Give it a player; it follows the "
                "player's time. Thumbnails and waveforms are made on other "
                "threads by the ThumbnailSystem and fill in as they "
                "arrive.")
                .def(
                    nb::new_(nb::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const std::shared_ptr<ftk::IWidget>&>(&TimelineWidget::create)),
                    nb::arg("context"),
                    nb::arg("parent") = nullptr)
                .def(
                    nb::new_(nb::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const std::shared_ptr<ITimeUnitsModel>&,
                        const std::shared_ptr<ftk::IWidget>&>(&TimelineWidget::create)),
                    nb::arg("context"),
                    nb::arg("timeUnitsModel"),
                    nb::arg("parent") = nullptr)
                .def_prop_ro(
                    "timeUnitsModel",
                    &TimelineWidget::getTimeUnitsModel)
                .def_prop_rw(
                    "player",
                    &TimelineWidget::getPlayer,
                    &TimelineWidget::setPlayer,
                    // "No player" is a null player.
                    nb::for_setter(nb::arg("value").none()),
                    "The player whose timeline is drawn, or None.")
                .def_prop_rw(
                    "displayOptions",
                    &TimelineWidget::getDisplayOptions,
                    &TimelineWidget::setDisplayOptions,
                    nb::rv_policy::copy)
                .def_prop_ro(
                    "observeDisplayOptions",
                    &TimelineWidget::observeDisplayOptions)
                .def(
                    "setTimelines",
                    &TimelineWidget::setTimelines,
                    nb::arg("timelines").none(),
                    "Set the timelines drawn beside the player's own. None, "
                    "the default, draws the player's compare timelines; a "
                    "list draws those instead, for showing timelines the "
                    "picture is not comparing.")
                .def_prop_rw(
                    "frameMarkers",
                    &TimelineWidget::getFrameMarkers,
                    &TimelineWidget::setFrameMarkers,
                    nb::rv_policy::copy)
                .def_prop_rw(
                    "markers",
                    &TimelineWidget::getMarkers,
                    &TimelineWidget::setMarkers,
                    nb::rv_policy::copy)
                .def_prop_rw(
                    "frameView",
                    &TimelineWidget::hasFrameView,
                    &TimelineWidget::setFrameView,
                    "Whether the view is framed automatically: the whole "
                    "timeline fitted to the widget's width.")
                .def_prop_ro(
                    "observeFrameView",
                    &TimelineWidget::observeFrameView)
                .def_prop_rw(
                    "scrollBarsVisible",
                    &TimelineWidget::areScrollBarsVisible,
                    &TimelineWidget::setScrollBarsVisible)
                .def_prop_ro(
                    "observeScrollBarsVisible",
                    &TimelineWidget::observeScrollBarsVisible)
                .def_prop_rw(
                    "autoScroll",
                    &TimelineWidget::hasAutoScroll,
                    &TimelineWidget::setAutoScroll,
                    "Whether the view scrolls to keep the current time in "
                    "view during playback.")
                .def_prop_ro(
                    "observeAutoScroll",
                    &TimelineWidget::observeAutoScroll)
                .def_prop_rw(
                    "stopOnScrub",
                    &TimelineWidget::hasStopOnScrub,
                    &TimelineWidget::setStopOnScrub,
                    "Whether scrubbing stops playback.")
                .def_prop_ro(
                    "observeStopOnScrub",
                    &TimelineWidget::observeStopOnScrub);
        }
    }
}

