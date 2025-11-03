// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "StatusBar.h"

#include "App.h"
#include "FilesModel.h"

#include <ftk/UI/Divider.h>
#include <ftk/Core/Format.h>

namespace tl
{
    namespace play
    {
        void StatusBar::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "StatusBar", parent);

            _layout = ftk::HorizontalLayout::create(context, shared_from_this());
            _layout->setSpacingRole(ftk::SizeRole::None);

            _labels["Log"] = ftk::Label::create(context, _layout);
            _labels["Log"]->setHStretch(ftk::Stretch::Expanding);
            _labels["Log"]->setMarginRole(ftk::SizeRole::MarginInside);

            ftk::Divider::create(context, ftk::Orientation::Horizontal, _layout);
            _labels["Info"] = ftk::Label::create(context, _layout);
            _labels["Info"]->setMarginRole(ftk::SizeRole::MarginInside);
            
            _logTimer = ftk::Timer::create(context);

            _logObserver = ftk::ListObserver<ftk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<ftk::LogItem>& value)
                {
                    _logUpdate(value);
                });

            _playerObserver = ftk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _infoUpdate(value);
                });
        }

        StatusBar::~StatusBar()
        {}

        std::shared_ptr<StatusBar> StatusBar::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StatusBar>(new StatusBar);
            out->_init(context, app, parent);
            return out;
        }

        void StatusBar::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }

        void StatusBar::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_layout->getSizeHint());
        }

        void StatusBar::_logUpdate(const std::vector<ftk::LogItem>& value)
        {
            for (const auto& i : value)
            {
                switch (i.type)
                {
                case ftk::LogType::Error:
                {
                    const std::string text = ftk::toString(i);
                    _labels["Log"]->setText(text);
                    _labels["Log"]->setTooltip(text);
                    _logTimer->start(
                        std::chrono::seconds(5),
                        [this]
                        {
                            _labels["Log"]->setText(std::string());
                            _labels["Log"]->setTooltip(std::string());
                        });
                    break;
                }
                default: break;
                }
            }
        }

        void StatusBar::_infoUpdate(const std::shared_ptr<timeline::Player>& player)
        {
            std::string text;
            if (player)
            {
                std::vector<std::string> pieces;
                const auto& path = player->getPath();
                pieces.push_back(path.get(-1, file::PathType::FileName));

                const auto& ioInfo = player->getIOInfo();
                if (!ioInfo.video.empty())
                {
                    const auto& videoInfo = ioInfo.video.front();
                    pieces.push_back(ftk::Format("video: {1}x{2}:{3} {4}").
                        arg(videoInfo.size.w).
                        arg(videoInfo.size.h).
                        arg(videoInfo.getAspect(), 2).
                        arg(videoInfo.type));
                }

                if (ioInfo.audio.isValid())
                {
                    pieces.push_back(ftk::Format("audio: {1} {2} {3}").
                        arg(ioInfo.audio.channelCount).
                        arg(ioInfo.audio.dataType).
                        arg(ioInfo.audio.sampleRate));
                }

                text = ftk::join(pieces, ", ");
            }
            _labels["Info"]->setText(text);
            _labels["Info"]->setTooltip(text);
        }
    }
}