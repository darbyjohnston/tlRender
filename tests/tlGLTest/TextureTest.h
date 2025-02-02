// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace gl_tests
    {
        class TextureTest : public tests::ITest
        {
        protected:
            TextureTest(const std::shared_ptr<dtk::Context>&);

        public:
            static std::shared_ptr<TextureTest> create(const std::shared_ptr<dtk::Context>&);

            void run() override;

        private:
            void _texture();
            void _textureAtlas();
        };
    }
}
