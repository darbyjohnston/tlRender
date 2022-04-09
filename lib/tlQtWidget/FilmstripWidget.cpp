// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQtWidget/FilmstripWidget.h>

#include <tlQt/TimelineThumbnailProvider.h>

#include <tlCore/Math.h>

#include <QHBoxLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QStyle>

namespace tl
{
    namespace qtwidget
    {
        struct FilmstripWidget::Private
        {
            qt::TimelineThumbnailProvider* thumbnailProvider = nullptr;
            std::shared_ptr<timeline::Timeline> timeline;
            int rowCount = 1;
            QSize thumbnailSize;
            qint64 thumbnailRequestId = 0;
            struct Thumbnail
            {
                QImage image;
                std::chrono::steady_clock::time_point time;
            };
            std::map<otime::RationalTime, Thumbnail> thumbnails;
        };

        FilmstripWidget::FilmstripWidget(qt::TimelineThumbnailProvider* thumbnailProvider, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
            setMinimumHeight(50);

            p.thumbnailProvider = thumbnailProvider;
            if (p.thumbnailProvider)
            {
                connect(
                    p.thumbnailProvider,
                    SIGNAL(thumbails(qint64, const QList<QPair<otime::RationalTime, QImage> >&)),
                    SLOT(_thumbnailsCallback(qint64, const QList<QPair<otime::RationalTime, QImage> >&)));
            }
        }

        FilmstripWidget::~FilmstripWidget()
        {}

        void FilmstripWidget::setTimeline(const std::shared_ptr<timeline::Timeline>& timeline)
        {
            TLRENDER_P();
            if (timeline == p.timeline)
                return;
            p.timeline = timeline;
            _thumbnailsUpdate();
        }

        int FilmstripWidget::rowCount() const
        {
            return _p->rowCount;
        }

        void FilmstripWidget::setRowCount(int value)
        {
            TLRENDER_P();
            if (value == p.rowCount)
                return;
            p.rowCount = value;
            updateGeometry();
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
            TLRENDER_P();
            QPainter painter(this);
            const auto& rect = this->rect();
            painter.fillRect(rect, QColor(0, 0, 0));
            const int width = rect.width();
            int x = 0;
            int y = 0;
            const auto now = std::chrono::steady_clock::now();
            for (const auto& i : p.thumbnails)
            {
                const auto diff = std::chrono::duration<float>(now - i.second.time);
                const float opacity = math::lerp(diff.count(), 0.F, 1.F);
                if (opacity < 1.F)
                {
                    update();
                }
                painter.setOpacity(opacity);
                painter.drawImage(QPoint(x, y), i.second.image);
                x += p.thumbnailSize.width();
                if (x > width)
                {
                    x = 0;
                    y += p.thumbnailSize.height();
                }
            }
        }

        void FilmstripWidget::_thumbnailsCallback(qint64 id, const QList<QPair<otime::RationalTime, QImage> >& thumbnails)
        {
            TLRENDER_P();
            if (p.thumbnailRequestId == id)
            {
                for (const auto& i : thumbnails)
                {
                    Private::Thumbnail thumbnail;
                    thumbnail.image = i.second;
                    thumbnail.time = std::chrono::steady_clock::now();
                    p.thumbnails[i.first] = thumbnail;
                }
                p.thumbnailRequestId = 0;
                update();
            }
        }

        void FilmstripWidget::_thumbnailsUpdate()
        {
            TLRENDER_P();
            p.thumbnails.clear();
            if (p.timeline && p.thumbnailProvider)
            {
                p.thumbnailProvider->cancelRequests(p.thumbnailRequestId);
                p.thumbnailRequestId = 0;

                const auto& size = this->size();
                const int width = size.width();
                const int height = size.height();
                const auto& info = p.timeline->getIOInfo();
                const int thumbnailHeight = height / p.rowCount;
                const int thumbnailWidth = !info.video.empty() ?
                    static_cast<int>(thumbnailHeight * info.video[0].size.getAspect()) :
                    0;
                p.thumbnailSize = QSize(thumbnailWidth, thumbnailHeight);
                if (thumbnailWidth > 0)
                {
                    QList<otime::RationalTime> requests;
                    const auto& globalStartTime = p.timeline->getGlobalStartTime();
                    const auto& duration = p.timeline->getDuration();
                    const int count = static_cast<int>(ceilf(width / static_cast<float>(thumbnailWidth))) * p.rowCount;
                    for (int i = 0; i < count; ++i)
                    {
                        requests.push_back(otime::RationalTime(
                            floor(i / static_cast<double>(count) * (duration.value() - 1) + globalStartTime.value()),
                            duration.rate()));
                    }
                    p.thumbnailRequestId = p.thumbnailProvider->request(
                        QString::fromUtf8(p.timeline->getPath().get().c_str()),
                        requests,
                        QSize(thumbnailWidth, thumbnailHeight));
                }
            }
            update();
        }
    }
}
