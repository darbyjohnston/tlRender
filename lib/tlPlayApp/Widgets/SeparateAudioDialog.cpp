// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Widgets/SeparateAudioPrivate.h>

namespace tl
{
    namespace play
    {
        struct SeparateAudioDialog::Private
        {
            std::shared_ptr<SeparateAudioWidget> widget;
        };

        void SeparateAudioDialog::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IDialog::_init(
                context,
                "tl::play_app::SeparateAudioDialog",
                parent);
            DTK_P();

            p.widget = SeparateAudioWidget::create(
                context,
                shared_from_this());

            p.widget->setCancelCallback(
                [this]
                {
                    close();
                });
        }

        SeparateAudioDialog::SeparateAudioDialog() :
            _p(new Private)
        {}

        SeparateAudioDialog::~SeparateAudioDialog()
        {}

        std::shared_ptr<SeparateAudioDialog> SeparateAudioDialog::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<SeparateAudioDialog>(new SeparateAudioDialog);
            out->_init(context, parent);
            return out;
        }

        void SeparateAudioDialog::setCallback(const std::function<void(
            const file::Path&,
            const file::Path&)>& value)
        {
            _p->widget->setCallback(value);
        }
    }
}
