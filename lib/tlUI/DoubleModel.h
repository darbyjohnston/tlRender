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
        //! Double precision floating point value model.
        class DoubleModel : public std::enable_shared_from_this<DoubleModel>
        {
            TLRENDER_NON_COPYABLE(DoubleModel);

            void _init(const std::shared_ptr<system::Context>&);

        protected:
            DoubleModel();

        public:
            ~DoubleModel();

            //! Create a new model.
            static std::shared_ptr<DoubleModel> create(
                const std::shared_ptr<system::Context>&);

            //! \name Value
            ///@{

            double getValue() const;

            void setValue(double);

            std::shared_ptr<observer::IValue<double> > observeValue() const;

            ///@}

            //! \name Range
            ///@{

            const math::DoubleRange& getRange() const;

            void setRange(const math::DoubleRange&);

            std::shared_ptr<observer::IValue<math::DoubleRange> > observeRange() const;

            ///@}

            //! \name Increment
            ///@{

            double getStep() const;

            void setStep(double);

            void incrementStep();
            void decrementStep();

            double getLargeStep() const;

            void setLargeStep(double);

            void incrementLargeStep();
            void decrementLargeStep();

            ///@}

            //! \name Default Value
            ///@{

            bool hasDefaultValue() const;

            std::shared_ptr<observer::IValue<bool> > observeHasDefaultValue() const;

            double getDefaultValue() const;

            void setDefaultValue(double);

            void setDefaultValue();

            void clearDefaultValue();

            ///@}

        private:
            TLRENDER_PRIVATE();
        };
    }
}
