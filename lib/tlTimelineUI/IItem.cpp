// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IItem.h>

#include <feather-tk/core/Error.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

#include <opentimelineio/marker.h>

#include <sstream>

namespace tl
{
    namespace timelineui
    {
        FEATHER_TK_ENUM_IMPL(
            InOutDisplay,
            "InsideRange",
            "OutsideRange");

        FEATHER_TK_ENUM_IMPL(
            CacheDisplay,
            "VideoAndAudio",
            "VideoOnly");

        FEATHER_TK_ENUM_IMPL(
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

        feather_tk::Color4F getMarkerColor(const std::string& value)
        {
            const std::map<std::string, feather_tk::Color4F> colors =
            {
                //! \bug The OTIO marker variables are causing undefined
                //! symbol errors on Linux and macOS.
                /*{OTIO_NS::Marker::Color::pink, feather_tk::Color4F(1.F, .752F, .796F)},
                { OTIO_NS::Marker::Color::red, feather_tk::Color4F(1.F, 0.F, 0.F) },
                { OTIO_NS::Marker::Color::orange, feather_tk::Color4F(1.F, .75F, 0.F) },
                { OTIO_NS::Marker::Color::yellow, feather_tk::Color4F(1.F, 1.F, 0.F) },
                { OTIO_NS::Marker::Color::green, feather_tk::Color4F(0.F, 1.F, 0.F) },
                { OTIO_NS::Marker::Color::cyan, feather_tk::Color4F(0.F, 1.F, 1.F) },
                { OTIO_NS::Marker::Color::blue, feather_tk::Color4F(0.F, 0.F, 1.F) },
                { OTIO_NS::Marker::Color::purple, feather_tk::Color4F(0.5F, 0.F, .5F) },
                { OTIO_NS::Marker::Color::magenta, feather_tk::Color4F(1.F, 0.F, 1.F) },
                { OTIO_NS::Marker::Color::black, feather_tk::Color4F(0.F, 0.F, 0.F) },
                { OTIO_NS::Marker::Color::white, feather_tk::Color4F(1.F, 1.F, 1.F) }*/
                { "PINK", feather_tk::Color4F(1.F, .752F, .796F)},
                { "RED", feather_tk::Color4F(1.F, 0.F, 0.F)},
                { "ORANGE", feather_tk::Color4F(1.F, .75F, 0.F) },
                { "YELLOW", feather_tk::Color4F(1.F, 1.F, 0.F)},
                { "GREEN", feather_tk::Color4F(0.F, 1.F, 0.F) },
                { "CYAN", feather_tk::Color4F(0.F, 1.F, 1.F)},
                { "BLUE", feather_tk::Color4F(0.F, 0.F, 1.F)},
                { "PURPLE", feather_tk::Color4F(0.5F, 0.F, .5F)},
                { "MAGENTA", feather_tk::Color4F(1.F, 0.F, 1.F)},
                { "BLACK", feather_tk::Color4F(0.F, 0.F, 0.F)},
                { "WHITE", feather_tk::Color4F(1.F, 1.F, 1.F)}
            };
            const auto i = colors.find(value);
            return i != colors.end() ? i->second : feather_tk::Color4F();
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
            feather_tk::ColorRole selectRole = feather_tk::ColorRole::None;
            std::shared_ptr<feather_tk::ValueObserver<bool> > timeUnitsObserver;
        };

        void IItem::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::string& objectName,
            const OTIO_NS::TimeRange& timeRange,
            const OTIO_NS::TimeRange& availableRange,
            const OTIO_NS::TimeRange& trimmedRange,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& data,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, objectName, parent);
            FEATHER_TK_P();

            _timeRange = timeRange;
            _availableRange = availableRange;
            _trimmedRange = trimmedRange;
            _scale = scale;
            _options = options;
            _displayOptions = displayOptions;
            _data = data;

            p.timeUnitsObserver = feather_tk::ValueObserver<bool>::create(
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

        feather_tk::ColorRole IItem::getSelectRole() const
        {
            return _p->selectRole;
        }

        void IItem::setSelectRole(feather_tk::ColorRole value)
        {
            FEATHER_TK_P();
            if (value == p.selectRole)
                return;
            p.selectRole = value;
            _setDrawUpdate();
        }

        OTIO_NS::RationalTime IItem::posToTime(float value) const
        {
            OTIO_NS::RationalTime out = time::invalidTime;
            const feather_tk::Box2I& g = getGeometry();
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
                out = feather_tk::clamp(
                    out,
                    _timeRange.start_time(),
                    _timeRange.end_time_inclusive());
            }
            return out;
        }

        int IItem::timeToPos(const OTIO_NS::RationalTime& value) const
        {
            const feather_tk::Box2I& g = getGeometry();
            const OTIO_NS::RationalTime t = value - _timeRange.start_time();
            return g.min.x + t.rescaled_to(1.0).value() * _scale;
        }

        feather_tk::Box2I IItem::_getClipRect(
            const feather_tk::Box2I& value,
            double scale)
        {
            feather_tk::Box2I out;
            const feather_tk::V2I c = feather_tk::center(value);
            out.min.x = (value.min.x - c.x) * scale + c.x;
            out.min.y = (value.min.y - c.y) * scale + c.y;
            out.max.x = (value.max.x - c.x) * scale + c.x;
            out.max.y = (value.max.y - c.y) * scale + c.y;
            return out;
        }

        std::string IItem::_getDurationLabel(const OTIO_NS::RationalTime& value)
        {
            const OTIO_NS::RationalTime rescaled = value.rescaled_to(_data->speed);
            return feather_tk::Format("{0}").
                arg(_data->timeUnitsModel->getLabel(rescaled));
        }

        void IItem::_timeUnitsUpdate()
        {}

        void to_json(nlohmann::json& json, const ItemOptions& value)
        {
            json["InputEnabled"] = value.inputEnabled;
            json["EditAssociatedClips"] = value.editAssociatedClips;
        }

        void to_json(nlohmann::json& json, const DisplayOptions& value)
        {
            json["InOutDisplay"] = to_string(value.inOutDisplay);
            json["CacheDisplay"] = to_string(value.cacheDisplay);
            nlohmann::json json2;
            for (size_t i = 0; i < value.tracks.size(); ++i)
            {
                json2.push_back(value.tracks[i]);
            }
            json["Tracks"] = json2;
            json["TrackInfo"] = value.trackInfo;
            json["ClipInfo"] = value.clipInfo;
            json["Thumbnails"] = value.thumbnails;
            json["ThumbnailHeight"] = value.thumbnailHeight;
            json["WaveformWidth"] = value.waveformWidth;
            json["WaveformHeight"] = value.waveformHeight;
            json["WaveformPrim"] = to_string(value.waveformPrim);
            json["RegularFont"] = value.regularFont;
            json["MonoFont"] = value.monoFont;
            json["FontSize"] = value.fontSize;
            json["ClipRectScale"] = value.clipRectScale;
            json["OCIO"] = value.ocio;
            json["LUT"] = value.lut;
        }

        void from_json(const nlohmann::json& json, ItemOptions& value)
        {
            json.at("InputEnabled").get_to(value.inputEnabled);
            json.at("EditAssociatedClips").get_to(value.editAssociatedClips);
        }

        void from_json(const nlohmann::json& json, DisplayOptions& value)
        {
            from_string(json["InOutDisplay"].get<std::string>(), value.inOutDisplay);
            from_string(json["CacheDisplay"].get<std::string>(), value.cacheDisplay);
            auto& json2 = json["Tracks"];
            for (auto i = json2.begin(); i != json2.end(); ++i)
            {
                value.tracks.push_back(i->get<int>());
            }
            json["TrackInfo"].get_to(value.trackInfo);
            json["ClipInfo"].get_to(value.clipInfo);
            json["Thumbnails"].get_to(value.thumbnails);
            json["ThumbnailHeight"].get_to(value.thumbnailHeight);
            json["WaveformWidth"].get_to(value.waveformWidth);
            json["WaveformHeight"].get_to(value.waveformHeight);
            from_string(json["WaveformPrim"].get<std::string>(), value.waveformPrim);
            json["RegularFont"].get_to(value.regularFont);
            json["MonoFont"].get_to(value.monoFont);
            json["FontSize"].get_to(value.fontSize);
            json["ClipRectScale"].get_to(value.clipRectScale);
            json["OCIO"].get_to(value.ocio);
            json["LUT"].get_to(value.lut);
        }
    }
}
