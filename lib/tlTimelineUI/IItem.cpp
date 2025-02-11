// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IItem.h>

#include <dtk/core/Error.h>
#include <dtk/core/Format.h>
#include <dtk/core/String.h>

#include <opentimelineio/marker.h>

#include <sstream>

namespace tl
{
    namespace timelineui
    {
        DTK_ENUM_IMPL(
            InOutDisplay,
            "InsideRange",
            "OutsideRange");

        DTK_ENUM_IMPL(
            CacheDisplay,
            "VideoAndAudio",
            "VideoOnly");

        DTK_ENUM_IMPL(
            WaveformPrim,
            "Mesh",
            "Image");

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
            dtk::ColorRole selectRole = dtk::ColorRole::None;
            std::shared_ptr<dtk::ValueObserver<bool> > timeUnitsObserver;
        };

        void IItem::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& objectName,
            const OTIO_NS::TimeRange& timeRange,
            const OTIO_NS::TimeRange& trimmedRange,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& data,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, objectName, parent);
            DTK_P();

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
            _setSizeUpdate();
            _setDrawUpdate();
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
            _setSizeUpdate();
            _setDrawUpdate();
        }

        dtk::ColorRole IItem::getSelectRole() const
        {
            return _p->selectRole;
        }

        void IItem::setSelectRole(dtk::ColorRole value)
        {
            DTK_P();
            if (value == p.selectRole)
                return;
            p.selectRole = value;
            _setDrawUpdate();
        }

        OTIO_NS::RationalTime IItem::posToTime(float value) const
        {
            OTIO_NS::RationalTime out = time::invalidTime;
            const dtk::Box2I& g = getGeometry();
            if (g.w() > 0)
            {
                const double normalized = (value - g.min.x) /
                    static_cast<double>(_timeRange.duration().rescaled_to(1.0).value() * _scale);
                out = OTIO_NS::RationalTime(
                    _timeRange.start_time() +
                    OTIO_NS::RationalTime(
                        _timeRange.duration().value() * normalized,
                        _timeRange.duration().rate())).
                    round();
                out = dtk::clamp(
                    out,
                    _timeRange.start_time(),
                    _timeRange.end_time_inclusive());
            }
            return out;
        }

        int IItem::timeToPos(const OTIO_NS::RationalTime& value) const
        {
            const dtk::Box2I& g = getGeometry();
            const OTIO_NS::RationalTime t = value - _timeRange.start_time();
            return g.min.x + t.rescaled_to(1.0).value() * _scale;
        }

        dtk::Box2I IItem::_getClipRect(
            const dtk::Box2I& value,
            double scale)
        {
            dtk::Box2I out;
            const dtk::V2I c = dtk::center(value);
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

        void to_json(nlohmann::json& json, const ItemOptions& value)
        {
            json["inputEnabled"] = value.inputEnabled;
            json["editAssociatedClips"] = value.editAssociatedClips;
        }

        void to_json(nlohmann::json& json, const DisplayOptions& value)
        {
            json["inOutDisplay"] = to_string(value.inOutDisplay);
            json["cacheDisplay"] = to_string(value.cacheDisplay);
            nlohmann::json json2;
            for (size_t i = 0; i < value.tracks.size(); ++i)
            {
                json2.push_back(value.tracks[i]);
            }
            json["tracks"] = json2;
            json["trackInfo"] = value.trackInfo;
            json["clipInfo"] = value.clipInfo;
            json["thumbnails"] = value.thumbnails;
            json["thumbnailHeight"] = value.thumbnailHeight;
            json["waveformWidth"] = value.waveformWidth;
            json["waveformHeight"] = value.waveformHeight;
            json["waveformPrim"] = to_string(value.waveformPrim);
            json["thumbnailFade"] = value.thumbnailFade;
            json["transitions"] = value.transitions;
            json["markers"] = value.markers;
            json["regularFont"] = value.regularFont;
            json["monoFont"] = value.monoFont;
            json["fontSize"] = value.fontSize;
            json["clipRectScale"] = value.clipRectScale;
            json["ocio"] = value.ocio;
            json["lut"] = value.lut;
        }

        void from_json(const nlohmann::json& json, ItemOptions& value)
        {
            json["inputEnabled"].get_to(value.inputEnabled);
            json["editAssociatedClips"].get_to(value.editAssociatedClips);
        }

        void from_json(const nlohmann::json& json, DisplayOptions& value)
        {
            from_string(json["inOutDisplay"].get<std::string>(), value.inOutDisplay);
            from_string(json["cacheDisplay"].get<std::string>(), value.cacheDisplay);
            auto& json2 = json["tracks"];
            for (auto i = json2.begin(); i != json2.end(); ++i)
            {
                value.tracks.push_back(i->get<int>());
            }
            json["trackInfo"].get_to(value.trackInfo);
            json["clipInfo"].get_to(value.clipInfo);
            json["thumbnails"].get_to(value.thumbnails);
            json["thumbnailHeight"].get_to(value.thumbnailHeight);
            json["waveformWidth"].get_to(value.waveformWidth);
            json["waveformHeight"].get_to(value.waveformHeight);
            from_string(json["waveformPrim"].get<std::string>(), value.waveformPrim);
            json["thumbnailFade"].get_to(value.thumbnailFade);
            json["transitions"].get_to(value.transitions);
            json["markers"].get_to(value.markers);
            json["regularFont"].get_to(value.regularFont);
            json["monoFont"].get_to(value.monoFont);
            json["fontSize"].get_to(value.fontSize);
            json["clipRectScale"].get_to(value.clipRectScale);
            json["ocio"].get_to(value.ocio);
            json["lut"].get_to(value.lut);
        }
    }
}
