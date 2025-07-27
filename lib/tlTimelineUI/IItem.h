// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ColorOptions.h>
#include <tlTimeline/TimeUnits.h>
#include <tlTimeline/Timeline.h>

#include <feather-tk/ui/IWidget.h>

#include <opentimelineio/item.h>

namespace tl
{
    namespace timelineui
    {
        class IItem;

        //! Item data.
        struct ItemData
        {
            double speed = 0.0;
            std::string directory;
            timeline::Options options;
            std::shared_ptr<timeline::ITimeUnitsModel> timeUnitsModel;
            std::map<std::string, std::shared_ptr<io::Info> > info;
            std::map<std::string, std::shared_ptr<feather_tk::Image> > thumbnails;
            std::map<std::string, std::shared_ptr<feather_tk::TriMesh2F> > waveforms;
        };

        //! In/out points display options.
        enum class InOutDisplay
        {
            InsideRange,
            OutsideRange,

            Count,
            First = InsideRange
        };
        FEATHER_TK_ENUM(InOutDisplay);
        
        //! Cache display options.
        enum class CacheDisplay
        {
            VideoAndAudio,
            VideoOnly,

            Count,
            First = VideoAndAudio
        };
        FEATHER_TK_ENUM(CacheDisplay);

        //! Waveform primitive type.
        enum class WaveformPrim
        {
            Mesh,
            Image,

            Count,
            First = Mesh
        };
        FEATHER_TK_ENUM(WaveformPrim);

        //! Item options.
        struct ItemOptions
        {
            bool inputEnabled = true;
            bool editAssociatedClips = true;

            bool operator == (const ItemOptions&) const;
            bool operator != (const ItemOptions&) const;
        };

        //! Display options.
        struct DisplayOptions
        {
            InOutDisplay inOutDisplay = InOutDisplay::InsideRange;
            CacheDisplay cacheDisplay = CacheDisplay::VideoAndAudio;

            bool minimize = true;

            bool thumbnails = true;
            int thumbnailHeight = 100;
            int waveformWidth = 200;
            int waveformHeight = 50;
            WaveformPrim waveformPrim = WaveformPrim::Mesh;

            std::string regularFont = "NotoSans-Regular";
            std::string monoFont = "NotoSansMono-Regular";
            int fontSize = 12;
            float clipRectScale = 2.F;

            timeline::OCIOOptions ocio;
            timeline::LUTOptions lut;

            bool operator == (const DisplayOptions&) const;
            bool operator != (const DisplayOptions&) const;
        };

        //! Marker.
        struct Marker
        {
            std::string name;
            feather_tk::Color4F color;
            OTIO_NS::TimeRange range;
        };

        //! Get the markers from an item.
        std::vector<Marker> getMarkers(const OTIO_NS::Item*);

        //! Convert a named marker color.
        feather_tk::Color4F getMarkerColor(const std::string&);

        //! Drag and drop data.
        class DragAndDropData : public feather_tk::DragAndDropData
        {
        public:
            DragAndDropData(const std::shared_ptr<IItem>&);

            virtual ~DragAndDropData();

            const std::shared_ptr<IItem>& getItem() const;

        private:
            std::shared_ptr<IItem> _item;
        };

        //! Base class for items.
        class IItem : public feather_tk::IWidget
        {
        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::string& objectName,
                const OTIO_NS::TimeRange& timeRange,
                const OTIO_NS::TimeRange& availableRange,
                const OTIO_NS::TimeRange& trimmedRange,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<feather_tk::IWidget>& parent = nullptr);

            IItem();

        public:
            virtual ~IItem() = 0;
            
            //! Get the item time range.
            const OTIO_NS::TimeRange& getTimeRange() const;

            //! Set the item scale.
            virtual void setScale(double);

            //! Set the item options.
            virtual void setOptions(const ItemOptions&);

            //! Set the display options.
            virtual void setDisplayOptions(const DisplayOptions&);

            //! Get the selection color role.
            feather_tk::ColorRole getSelectRole() const;

            //! Set the selection color role.
            void setSelectRole(feather_tk::ColorRole);

            //! Convert a position to a time.
            OTIO_NS::RationalTime posToTime(float) const;

            //! Convert a time to a position.
            int timeToPos(const OTIO_NS::RationalTime&) const;

        protected:
            static feather_tk::Box2I _getClipRect(
                const feather_tk::Box2I&,
                double scale);

            std::string _getDurationLabel(const OTIO_NS::RationalTime&);

            virtual void _timeUnitsUpdate();

            OTIO_NS::TimeRange _timeRange = time::invalidTimeRange;
            OTIO_NS::TimeRange _availableRange = time::invalidTimeRange;
            OTIO_NS::TimeRange _trimmedRange = time::invalidTimeRange;
            double _scale = 500.0;
            ItemOptions _options;
            DisplayOptions _displayOptions;
            std::shared_ptr<ItemData> _data;

        private:
            FEATHER_TK_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const ItemOptions&);
        void to_json(nlohmann::json&, const DisplayOptions&);

        void from_json(const nlohmann::json&, ItemOptions&);
        void from_json(const nlohmann::json&, DisplayOptions&);

        ///@}
    }
}
