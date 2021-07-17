// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace file
    {
        inline const std::string& Path::getDirectory() const
        {
            return _directory;
        }

        inline const std::string& Path::getBaseName() const
        {
            return _baseName;
        }

        inline const std::string& Path::getNumber() const
        {
            return _number;
        }

        inline int Path::getPadding() const
        {
            return _padding;
        }

        inline const std::string& Path::getExtension() const
        {
            return _extension;
        }
    }
}
