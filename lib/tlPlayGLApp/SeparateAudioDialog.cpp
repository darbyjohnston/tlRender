// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/SeparateAudioPrivate.h>

namespace tl
{
    namespace play_gl
    {
        struct SeparateAudioDialog::Private
        {
            std::shared_ptr<SeparateAudioWidget> widget;
        };

        void SeparateAudioDialog::_init(
            const std::string& path,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IDialog::_init(
                "tl::play_gl::SeparateAudioDialog",
                context,
                parent);
            TLRENDER_P();

            p.widget = SeparateAudioWidget::create(
                path,
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
            const std::string& path,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<SeparateAudioDialog>(new SeparateAudioDialog);
            out->_init(path, context, parent);
            return out;
        }

        void SeparateAudioDialog::setFileCallback(const std::function<void(
            const file::Path&,
            const file::Path&)>& value)
        {
            _p->widget->setFileCallback(value);
        }
    }
}
