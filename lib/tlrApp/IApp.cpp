// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/IApp.h>

namespace tlr
{
    namespace app
    {
        void IApp::_init(int argc, char* argv[])
        {
            for (int i = 1; i < argc; ++i)
            {
                _args.push_back(argv[i]);
            }
        }
        
        IApp::IApp()
        {}

        IApp::~IApp()
        {}
    }
}
