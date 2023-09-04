// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace file
    {
        inline bool isPathSeparator(char value)
        {
            return
                value == pathSeparators[0] ||
                value == pathSeparators[1];
        }

        constexpr bool PathOptions::operator == (const PathOptions& other) const
        {
            return maxNumberDigits == other.maxNumberDigits;
        }

        constexpr bool PathOptions::operator != (const PathOptions& other) const
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

        inline const math::IntRange& Path::getSequence() const
        {
            return _sequence;
        }

        inline bool Path::isSequence() const
        {
            return _sequence.getMin() != _sequence.getMax();
        }

        inline bool Path::sequence(const Path& value) const
        {
            return
                _directory == value._directory &&
                _baseName == value._baseName &&
                _padding == value._padding &&
                _extension == value._extension;
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
