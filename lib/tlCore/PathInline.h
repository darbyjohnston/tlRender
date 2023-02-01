// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace file
    {
        inline bool PathOptions::operator == (const PathOptions& other) const
        {
            return maxNumberDigits == other.maxNumberDigits;
        }

        inline bool PathOptions::operator != (const PathOptions& other) const
        {
            return !(*this == other);
        }

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

        inline uint8_t Path::getPadding() const
        {
            return _padding;
        }

        inline const std::string& Path::getExtension() const
        {
            return _extension;
        }
    }
}
