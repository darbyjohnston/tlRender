// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>
#include <tlCore/Range.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace ui
    {
        //! Floating point value model.
        class FloatModel : public std::enable_shared_from_this<FloatModel>
        {
            TLRENDER_NON_COPYABLE(FloatModel);

            void _init(const std::shared_ptr<system::Context>&);

        protected:
            FloatModel();

        public:
            ~FloatModel();

            //! Create a new floating point value model.
            static std::shared_ptr<FloatModel> create(
                const std::shared_ptr<system::Context>&);

            //! \name Value
            ///@{

            float getValue() const;

            void setValue(float);

            std::shared_ptr<observer::IValue<float> > observeValue() const;

            ///@}

            //! \name Range
            ///@{

            const math::FloatRange& getRange() const;

            void setRange(const math::FloatRange&);

            std::shared_ptr<observer::IValue<math::FloatRange> > observeRange() const;

            ///@}

            //! \name Increment
            ///@{

            float getStep() const;

            void setStep(float);

            void addStep();
            void subtractStep();

            float getLargeStep() const;

            void setLargeStep(float);

            void addLargeStep();
            void subtractLargeStep();

            ///@}

        private:
            TLRENDER_PRIVATE();
        };
    }
}
