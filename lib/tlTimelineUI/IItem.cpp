// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IItem.h>

#include <dtk/core/Format.h>

#include <opentimelineio/marker.h>

namespace tl
{
    namespace timelineui
    {
        bool ItemOptions::operator == (const ItemOptions& other) const
        {
            return
                inputEnabled == other.inputEnabled &&
                editAssociatedClips == other.editAssociatedClips;
        }

        bool ItemOptions::operator != (const ItemOptions& other) const
        {
            return !(*this == other);
        }

        bool DisplayOptions::operator == (const DisplayOptions& other) const
        {
            return
                inOutDisplay == other.inOutDisplay &&
                cacheDisplay == other.cacheDisplay &&
                tracks == other.tracks &&
                trackInfo == other.trackInfo &&
                clipInfo == other.clipInfo &&
                thumbnails == other.thumbnails &&
                thumbnailHeight == other.thumbnailHeight &&
                waveformWidth == other.waveformWidth &&
                waveformHeight == other.waveformHeight &&
                waveformPrim == other.waveformPrim &&
                thumbnailFade == other.thumbnailFade &&
                transitions == other.transitions &&
                markers == other.markers &&
                regularFont == other.regularFont &&
                monoFont == other.monoFont &&
                fontSize == other.fontSize &&
                clipRectScale == other.clipRectScale &&
                ocio == other.ocio &&
                lut == other.lut;
        }

        bool DisplayOptions::operator != (const DisplayOptions& other) const
        {
            return !(*this == other);
        }

        std::vector<Marker> getMarkers(const OTIO_NS::Item* item)
        {
            std::vector<Marker> out;
            for (const auto& marker : item->markers())
            {
                out.push_back({
                    marker->name(),
                    getMarkerColor(marker->color()),
                    marker->marked_range() });
            }
            return out;
        }

        dtk::Color4F getMarkerColor(const std::string& value)
        {
            const std::map<std::string, dtk::Color4F> colors =
            {
                //! \bug The OTIO marker variables are causing undefined
                //! symbol errors on Linux and macOS.
                /*{OTIO_NS::Marker::Color::pink, dtk::Color4F(1.F, .752F, .796F)},
                { OTIO_NS::Marker::Color::red, dtk::Color4F(1.F, 0.F, 0.F) },
                { OTIO_NS::Marker::Color::orange, dtk::Color4F(1.F, .75F, 0.F) },
                { OTIO_NS::Marker::Color::yellow, dtk::Color4F(1.F, 1.F, 0.F) },
                { OTIO_NS::Marker::Color::green, dtk::Color4F(0.F, 1.F, 0.F) },
                { OTIO_NS::Marker::Color::cyan, dtk::Color4F(0.F, 1.F, 1.F) },
                { OTIO_NS::Marker::Color::blue, dtk::Color4F(0.F, 0.F, 1.F) },
                { OTIO_NS::Marker::Color::purple, dtk::Color4F(0.5F, 0.F, .5F) },
                { OTIO_NS::Marker::Color::magenta, dtk::Color4F(1.F, 0.F, 1.F) },
                { OTIO_NS::Marker::Color::black, dtk::Color4F(0.F, 0.F, 0.F) },
                { OTIO_NS::Marker::Color::white, dtk::Color4F(1.F, 1.F, 1.F) }*/
                { "PINK", dtk::Color4F(1.F, .752F, .796F)},
                { "RED", dtk::Color4F(1.F, 0.F, 0.F)},
                { "ORANGE", dtk::Color4F(1.F, .75F, 0.F) },
                { "YELLOW", dtk::Color4F(1.F, 1.F, 0.F)},
                { "GREEN", dtk::Color4F(0.F, 1.F, 0.F) },
                { "CYAN", dtk::Color4F(0.F, 1.F, 1.F)},
                { "BLUE", dtk::Color4F(0.F, 0.F, 1.F)},
                { "PURPLE", dtk::Color4F(0.5F, 0.F, .5F)},
                { "MAGENTA", dtk::Color4F(1.F, 0.F, 1.F)},
                { "BLACK", dtk::Color4F(0.F, 0.F, 0.F)},
                { "WHITE", dtk::Color4F(1.F, 1.F, 1.F)}
            };
            const auto i = colors.find(value);
            return i != colors.end() ? i->second : dtk::Color4F();
        }

        DragAndDropData::DragAndDropData(
            const std::shared_ptr<IItem>& item) :
            _item(item)
        {}

        DragAndDropData::~DragAndDropData()
        {}

        const std::shared_ptr<IItem>& DragAndDropData::getItem() const
        {
            return _item;
        }

        struct IItem::Private
        {
            ui::ColorRole selectRole = ui::ColorRole::None;
            std::shared_ptr<dtk::ValueObserver<bool> > timeUnitsObserver;
        };

        void IItem::_init(
            const std::string& objectName,
            const OTIO_NS::TimeRange& timeRange,
            const OTIO_NS::TimeRange& trimmedRange,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& data,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(objectName, context, parent);
            TLRENDER_P();

            _timeRange = timeRange;
            _trimmedRange = trimmedRange;
            _scale = scale;
            _options = options;
            _displayOptions = displayOptions;
            _data = data;

            p.timeUnitsObserver = dtk::ValueObserver<bool>::create(
                data->timeUnitsModel->observeTimeUnitsChanged(),
                [this](bool)
                {
                    _timeUnitsUpdate();
                });
        }

        IItem::IItem() :
            _p(new Private)
        {}

        IItem::~IItem()
        {}

        const OTIO_NS::TimeRange& IItem::getTimeRange() const
        {
            return _timeRange;
        }

        void IItem::setScale(double value)
        {
            if (value == _scale)
                return;
            _scale = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void IItem::setOptions(const ItemOptions& value)
        {
            _options = value;
        }

        void IItem::setDisplayOptions(const DisplayOptions& value)
        {
            if (value == _displayOptions)
                return;
            _displayOptions = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        ui::ColorRole IItem::getSelectRole() const
        {
            return _p->selectRole;
        }

        void IItem::setSelectRole(ui::ColorRole value)
        {
            TLRENDER_P();
            if (value == p.selectRole)
                return;
            p.selectRole = value;
            _updates |= ui::Update::Draw;
        }

        OTIO_NS::RationalTime IItem::posToTime(float value) const
        {
            OTIO_NS::RationalTime out = time::invalidTime;
            if (_geometry.w() > 0)
            {
                const double normalized = (value - _geometry.min.x) /
                    static_cast<double>(_timeRange.duration().rescaled_to(1.0).value() * _scale);
                out = OTIO_NS::RationalTime(
                    _timeRange.start_time() +
                    OTIO_NS::RationalTime(
                        _timeRange.duration().value() * normalized,
                        _timeRange.duration().rate())).
                    round();
                out = math::clamp(
                    out,
                    _timeRange.start_time(),
                    _timeRange.end_time_inclusive());
            }
            return out;
        }

        int IItem::timeToPos(const OTIO_NS::RationalTime& value) const
        {
            const OTIO_NS::RationalTime t = value - _timeRange.start_time();
            return _geometry.min.x + t.rescaled_to(1.0).value() * _scale;
        }

        math::Box2i IItem::_getClipRect(
            const math::Box2i& value,
            double scale)
        {
            math::Box2i out;
            const math::Vector2i c = value.getCenter();
            out.min.x = (value.min.x - c.x) * scale + c.x;
            out.min.y = (value.min.y - c.y) * scale + c.y;
            out.max.x = (value.max.x - c.x) * scale + c.x;
            out.max.y = (value.max.y - c.y) * scale + c.y;
            return out;
        }

        std::string IItem::_getDurationLabel(const OTIO_NS::RationalTime& value)
        {
            const OTIO_NS::RationalTime rescaled = value.rescaled_to(_data->speed);
            return dtk::Format("{0}").
                arg(_data->timeUnitsModel->getLabel(rescaled));
        }

        void IItem::_timeUnitsUpdate()
        {}
    }
}
