// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlCore/FileInfo.h>

namespace tl
{
    namespace file
    {
        bool listFilter(const std::string&, const ListOptions&);
        
        void listSequence(
            const std::string& path,
            const std::string& fileName,
            std::vector<FileInfo>&,
            const ListOptions&);
        
        void _list(
            const std::string&,
            std::vector<FileInfo>&,
            const ListOptions& = ListOptions());
    }
}
