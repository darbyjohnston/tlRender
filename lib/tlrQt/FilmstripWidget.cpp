// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/FilmstripWidget.h>

#include <tlrQt/TimelineThumbnailProvider.h>

#include <QHBoxLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QStyle>

namespace tlr
{
    namespace qt
    {
        struct FilmstripWidget::Private
        {
            std::shared_ptr<timeline::Timeline> timeline;
            TimelineThumbnailProvider* thumbnailProvider = nullptr;
            std::map<otime::RationalTime, QImage> thumbnails;
        };

        FilmstripWidget::FilmstripWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
            setMinimumHeight(50);
        }

        void FilmstripWidget::setTimeline(const std::shared_ptr<timeline::Timeline>& timeline)
        {
            TLR_PRIVATE_P();
            p.timeline = timeline;
            if (p.thumbnailProvider)
            {
                delete p.thumbnailProvider;
                p.thumbnailProvider = nullptr;
            }
            if (p.timeline)
            {
                p.thumbnailProvider = new TimelineThumbnailProvider(p.timeline, this);
                connect(
                    p.thumbnailProvider,
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
            TLR_PRIVATE_P();
            QPainter painter(this);
            painter.fillRect(rect(), QColor(0, 0, 0));
            for (const auto& i : p.thumbnails)
            {
                const int x = _timeToPos(i.first);
                painter.drawImage(QPoint(x, 0), i.second);
            }
        }

        void FilmstripWidget::_thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >& thumbnails)
        {
            TLR_PRIVATE_P();
            for (const auto& i : thumbnails)
            {
                p.thumbnails[i.first] = i.second;
            }
            update();
        }

        otime::RationalTime FilmstripWidget::_posToTime(int value) const
        {
            TLR_PRIVATE_P();
            otime::RationalTime out = time::invalidTime;
            if (p.timeline)
            {
                const auto& globalStartTime = p.timeline->getGlobalStartTime();
                const auto& duration = p.timeline->getDuration();
                out = otime::RationalTime(
                    floor(value / static_cast<double>(width()) * (duration.value() - 1) + globalStartTime.value()),
                    duration.rate());
            }
            return out;
        }

        int FilmstripWidget::_timeToPos(const otime::RationalTime& value) const
        {
            TLR_PRIVATE_P();
            int out = 0;
            if (p.timeline)
            {
                const auto& globalStartTime = p.timeline->getGlobalStartTime();
                const auto& duration = p.timeline->getDuration();
                out = (value.value() - globalStartTime.value()) / (duration.value() - 1) * width();
            }
            return out;
        }

        void FilmstripWidget::_thumbnailsUpdate()
        {
            TLR_PRIVATE_P();
            p.thumbnails.clear();
            if (p.timeline && p.thumbnailProvider)
            {
                p.thumbnailProvider->cancelRequests();

                const auto& duration = p.timeline->getDuration();
                const auto& imageInfo = p.timeline->getImageInfo();
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
                    p.thumbnailProvider->request(requests, QSize(thumbnailWidth, thumbnailHeight));
                }
            }
            update();
        }
    }
}
