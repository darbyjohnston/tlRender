// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

#include <tlIO/IO.h>

namespace tl
{
    namespace io_tests
    {
        class CineonTest : public tests::ITest
        {
        protected:
            CineonTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<CineonTest> create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _enums();
            void _io();
            void _write(
                const std::shared_ptr<io::IPlugin>&,
                const std::shared_ptr<imaging::Image>&,
                const file::Path&,
                const imaging::Info&,
                const imaging::Tags&);
            void _read(
                const std::shared_ptr<io::IPlugin>&,
                const std::shared_ptr<imaging::Image>&,
                const file::Path&,
                bool memoryIO,
                const imaging::Tags&);
            void _readError(
                const std::shared_ptr<io::IPlugin>&,
                const std::shared_ptr<imaging::Image>&,
                const file::Path&,
                bool memoryIO);
        };
    }
}
