// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/FilmstripWidget.h>

#include <QHBoxLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QStyle>

namespace tlr
{
    namespace qt
    {
        FilmstripWidget::FilmstripWidget(QWidget* parent) :
            QWidget(parent)
        {
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
            setMinimumHeight(50);
        }

        void FilmstripWidget::setTimeline(const std::shared_ptr<timeline::Timeline>& timeline)
        {
            _timeline = timeline;
            if (_thumbnailProvider)
            {
                delete _thumbnailProvider;
                _thumbnailProvider = nullptr;
            }
            if (_timeline)
            {
                _thumbnailProvider = new TimelineThumbnailProvider(_timeline, this);
                connect(
                    _thumbnailProvider,
                    SIGNAL(thumbails(const QList<QPair<otime::RationalTime, QImage> >&)),
                    SLOT(_thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >&)));
            }
            _thumbnailsUpdate();
        }

        void FilmstripWidget::resizeEvent(QResizeEvent* event)
        {
            if (event->oldSize() != size())
            {
                _thumbnailsUpdate();
            }
        }

        void FilmstripWidget::paintEvent(QPaintEvent*)
        {
            QPainter painter(this);
            painter.fillRect(rect(), QColor(0, 0, 0));
            for (const auto& i : _thumbnails)
            {
                const int x = _timeToPos(i.first);
                painter.drawImage(QPoint(x, 0), i.second);
            }
        }

        void FilmstripWidget::_thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >& thumbnails)
        {
            for (const auto& i : thumbnails)
            {
                _thumbnails[i.first] = i.second;
            }
            update();
        }

        otime::RationalTime FilmstripWidget::_posToTime(int value) const
        {
            otime::RationalTime out = invalidTime;
            if (_timeline)
            {
                const auto& globalStartTime = _timeline->getGlobalStartTime();
                const auto& duration = _timeline->getDuration();
                out = otime::RationalTime(
                    floor(value / static_cast<double>(width()) * (duration.value() - 1) + globalStartTime.value()),
                    duration.rate());
            }
            return out;
        }

        int FilmstripWidget::_timeToPos(const otime::RationalTime& value) const
        {
            int out = 0;
            if (_timeline)
            {
                const auto& globalStartTime = _timeline->getGlobalStartTime();
                const auto& duration = _timeline->getDuration();
                out = (value.value() - globalStartTime.value()) / (duration.value() - 1) * width();
            }
            return out;
        }

        void FilmstripWidget::_thumbnailsUpdate()
        {
            _thumbnails.clear();
            if (_timeline && _thumbnailProvider)
            {
                _thumbnailProvider->cancelRequests();

                const auto& duration = _timeline->getDuration();
                const auto& imageInfo = _timeline->getImageInfo();
                const auto& size = this->size();
                const int width = size.width();
                const int height = size.height();
                const int thumbnailWidth = static_cast<int>(height * imageInfo.size.getAspect());
                const int thumbnailHeight = height;
                if (thumbnailWidth > 0)
                {
                    QList<otime::RationalTime> requests;
                    int x = 0;
                    while (x < width)
                    {
                        requests.push_back(_posToTime(x));
                        x += thumbnailWidth;
                    }
                    _thumbnailProvider->request(requests, QSize(thumbnailWidth, thumbnailHeight));
                }
            }
            update();
        }
    }
}
