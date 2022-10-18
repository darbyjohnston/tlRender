// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

#include <tlIO/PNG.h>

namespace tl
{
    namespace io_tests
    {
        class PNGTest : public tests::ITest
        {
        protected:
            PNGTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<PNGTest> create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _write(
                const std::shared_ptr<png::Plugin>&,
                const std::shared_ptr<imaging::Image>&,
                const file::Path&,
                const imaging::Info&);
            void _read(
                const std::shared_ptr<png::Plugin>&,
                const std::shared_ptr<imaging::Image>&,
                const file::Path&,
                bool memoryIO);
            void _readError(
                const std::shared_ptr<png::Plugin>&,
                const std::shared_ptr<imaging::Image>&,
                const file::Path&,
                bool memoryIO);
        };
    }
}
