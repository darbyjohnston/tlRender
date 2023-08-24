// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/MessageDialog.h>

#include <tlUI/Divider.h>
#include <tlUI/Label.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/Spacer.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            class MessageWidget : public IWidget
            {
                TLRENDER_NON_COPYABLE(MessageWidget);

            protected:
                void _init(
                    const std::string&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                MessageWidget();

            public:
                virtual ~MessageWidget();

                static std::shared_ptr<MessageWidget> create(
                    const std::string&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setCallback(const std::function<void(bool)>&);

                void setGeometry(const math::Box2i&) override;
                void sizeHintEvent(const SizeHintEvent&) override;

            private:
                std::shared_ptr<Label> _titleLabel;
                std::shared_ptr<Label> _label;
                std::shared_ptr<PushButton> _okButton;
                std::shared_ptr<PushButton> _cancelButton;
                std::shared_ptr<VerticalLayout> _layout;
                std::function<void(bool)> _callback;
            };

            void MessageWidget::_init(
                const std::string& text,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init("tl::ui::MessageWidget", context, parent);

                _titleLabel = Label::create("File Browser", context);
                _titleLabel->setMarginRole(SizeRole::MarginSmall);
                _titleLabel->setBackgroundRole(ColorRole::Button);

                _label = Label::create(text, context);
                _label->setMarginRole(SizeRole::MarginLarge);

                _okButton = PushButton::create("OK", context);
                _cancelButton = PushButton::create("Cancel", context);

                _layout = VerticalLayout::create(context, shared_from_this());
                _layout->setSpacingRole(SizeRole::None);
                _titleLabel->setParent(_layout);
                Divider::create(Orientation::Vertical, context, _layout);
                auto vLayout = VerticalLayout::create(context, _layout);
                vLayout->setMarginRole(SizeRole::MarginSmall);
                vLayout->setSpacingRole(SizeRole::None);
                _label->setParent(vLayout);
                auto hLayout = HorizontalLayout::create(context, vLayout);
                hLayout->setSpacingRole(SizeRole::None);
                _cancelButton->setParent(hLayout);
                auto spacer = Spacer::create(Orientation::Horizontal, context, hLayout);
                spacer->setHStretch(Stretch::Expanding);
                _okButton->setParent(hLayout);

                _okButton->setClickedCallback(
                    [this]
                    {
                        if (_callback)
                        {
                            _callback(true);
                        }
                    });

                _cancelButton->setClickedCallback(
                    [this]
                    {
                        if (_callback)
                        {
                            _callback(false);
                        }
                    });
            }

            MessageWidget::MessageWidget()
            {}

            MessageWidget::~MessageWidget()
            {}

            std::shared_ptr<MessageWidget> MessageWidget::create(
                const std::string& text,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<MessageWidget>(new MessageWidget);
                out->_init(text, context, parent);
                return out;
            }

            void MessageWidget::setCallback(const std::function<void(bool)>& value)
            {
                _callback = value;
            }

            void MessageWidget::setGeometry(const math::Box2i& value)
            {
                IWidget::setGeometry(value);
                _layout->setGeometry(value);
            }

            void MessageWidget::sizeHintEvent(const SizeHintEvent& event)
            {
                IWidget::sizeHintEvent(event);
                _sizeHint = _layout->getSizeHint();
            }
        }

        struct MessageDialog::Private
        {
            std::shared_ptr<MessageWidget> widget;

            std::function<void(bool)> callback;
        };

        void MessageDialog::_init(
            const std::string& text,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IDialog::_init("tl::ui::MessageDialog", context, parent);
            TLRENDER_P();

            p.widget = MessageWidget::create(text, context, shared_from_this());

            p.widget->setCallback(
                [this](bool value)
                {
                    if (_p->callback)
                    {
                        _p->callback(value);
                    }
                });
        }

        MessageDialog::MessageDialog() :
            _p(new Private)
        {}

        MessageDialog::~MessageDialog()
        {}

        std::shared_ptr<MessageDialog> MessageDialog::create(
            const std::string& text,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MessageDialog>(new MessageDialog);
            out->_init(text, context, parent);
            return out;
        }

        void MessageDialog::setCallback(const std::function<void(bool)>& value)
        {
            _p->callback = value;
        }

        struct MessageDialogSystem::Private
        {
            std::shared_ptr<MessageDialog> dialog;
        };

        void MessageDialogSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::ui::MessageDialogSystem", context);
        }

        MessageDialogSystem::MessageDialogSystem() :
            _p(new Private)
        {}

        MessageDialogSystem::~MessageDialogSystem()
        {}

        std::shared_ptr<MessageDialogSystem> MessageDialogSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<MessageDialogSystem>(new MessageDialogSystem);
            out->_init(context);
            return out;
        }

        void MessageDialogSystem::open(
            const std::string& text,
            const std::shared_ptr<EventLoop>& eventLoop,
            const std::function<void(bool)>& callback)
        {
            TLRENDER_P();
            if (auto context = _context.lock())
            {
                p.dialog = MessageDialog::create(text, context);
                p.dialog->open(eventLoop);
                p.dialog->setCallback(
                    [this, callback](bool value)
                    {
                        callback(value);
                        _p->dialog->close();
                    });
                p.dialog->setCloseCallback(
                    [this]
                    {
                        _p->dialog.reset();
                    });
            }
        }
    }
}
