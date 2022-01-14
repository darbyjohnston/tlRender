// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/TimelinePlayer.h>

#include <QAbstractListModel>

namespace tlr
{
    struct TimelineListItem
    {
        TimelineListItem();
        TimelineListItem(const std::shared_ptr<timeline::TimelinePlayer>&);

        file::Path path;
        file::Path audioPath;

        otime::RationalTime duration = time::invalidTime;
        otime::RationalTime globalStartTime = time::invalidTime;
        avio::Info avIOInfo;

        double speed = 0.0;
        timeline::Playback playback = timeline::Playback::Stop;
        timeline::Loop loop = timeline::Loop::Loop;
        otime::RationalTime currentTime = time::invalidTime;
        otime::TimeRange inOutRange = time::invalidTimeRange;

        uint16_t videoLayer = 0;

        float volume = 0.F;
        bool mute = false;
        double audioOffset = 0.0;
    };

    //! Timeline list model.
    class TimelineListModel : public QAbstractListModel
    {
        Q_OBJECT

    public:
        TimelineListModel(
            const std::shared_ptr<core::Context>&,
            QObject* parent = nullptr);

        void add(const TimelineListItem&);
        void remove(int);

        TimelineListItem get(int) const;
        void set(int, const TimelineListItem&);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

    private:
        std::vector<TimelineListItem> _items;
    };
}
