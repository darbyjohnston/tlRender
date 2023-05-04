// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IButton.h>

namespace tl
{
    namespace ui
    {
        class FloatModel;
        class IntModel;

        //! Button for incrementing a value.
        class IncButton : public IButton
        {
            TLRENDER_NON_COPYABLE(IncButton);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IncButton();

        public:
            ~IncButton() override;

            //! Create a new increment button.
            static std::shared_ptr<IncButton> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Buttons for incrementing and decrementing a value.
        class IncButtons : public IWidget
        {
            TLRENDER_NON_COPYABLE(IncButtons);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IncButtons();

        public:
            ~IncButtons() override;

            //! Create new increment buttons.
            static std::shared_ptr<IncButtons> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the increment callback.
            void setIncCallback(const std::function<void(void)>&);

            //! Set the decrement callback.
            void setDecCallback(const std::function<void(void)>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        protected:
            std::shared_ptr<IncButton> _incButton;
            std::shared_ptr<IncButton> _decButton;
        };

        //! Buttons for incrementing and decrementing an integer value.
        class IntIncButtons : public IncButtons
        {
            TLRENDER_NON_COPYABLE(IntIncButtons);

        protected:
            void _init(
                const std::shared_ptr<IntModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IntIncButtons();

        public:
            ~IntIncButtons() override;

            //! Create new increment buttons.
            static std::shared_ptr<IntIncButtons> create(
                const std::shared_ptr<IntModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the integer model.
            const std::shared_ptr<IntModel>& getModel() const;

        private:
            void _modelUpdate();

            TLRENDER_PRIVATE();
        };

        //! Buttons for incrementing and decrementing a floating point value.
        class FloatIncButtons : public IncButtons
        {
            TLRENDER_NON_COPYABLE(FloatIncButtons);

        protected:
            void _init(
                const std::shared_ptr<FloatModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            FloatIncButtons();

        public:
            ~FloatIncButtons() override;

            //! Create new increment buttons.
            static std::shared_ptr<FloatIncButtons> create(
                const std::shared_ptr<FloatModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the floating point model.
            const std::shared_ptr<FloatModel>& getModel() const;

        private:
            void _modelUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
