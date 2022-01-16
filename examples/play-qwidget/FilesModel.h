// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/TimelinePlayer.h>

#include <QAbstractListModel>

namespace tlr
{
    //! Files model item.
    struct FilesModelItem
    {
        FilesModelItem();
        FilesModelItem(const std::string& fileName);
        FilesModelItem(const std::string& fileName, const std::string& audioFileName);
        FilesModelItem(const std::shared_ptr<timeline::TimelinePlayer>&);

        file::Path path;
        file::Path audioPath;

        bool init = false;

        otime::RationalTime duration = time::invalidTime;
        otime::RationalTime globalStartTime = time::invalidTime;
        avio::Info avInfo;

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

    //! Files model.
    class FilesModel : public QAbstractListModel
    {
        Q_OBJECT

    public:
        FilesModel(
            const std::shared_ptr<core::Context>&,
            QObject* parent = nullptr);

        void add(const FilesModelItem&);
        void remove();
        void clear();

        const FilesModelItem* current() const;
        void setCurrent(const QModelIndex&);

        void update(const FilesModelItem&);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

    public Q_SLOTS:
        void first();
        void last();
        void next();
        void prev();

    Q_SIGNALS:
        void currentChanged(const FilesModelItem*);

    private:
        std::vector<FilesModelItem> _items;
        int _current = -1;
    };
}
