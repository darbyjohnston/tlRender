// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/Path.h>

#include <tlCore/Error.h>
#include <tlCore/Math.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <array>
#include <iomanip>
#include <sstream>

namespace tl
{
    namespace file
    {
        Path::Path()
        {}

        Path::Path(
            const std::string& value,
            const PathOptions& options)
        {
            if (!value.empty())
            {
                // Find the extension.
                const size_t size = value.size();
                size_t i = size - 1;
                for (; i > 0 && value[i] != '.' && !isPathSeparator(value[i]); --i)
                    ;
                if (i > 0 &&
                    '.' == value[i] &&
                    '.' != value[i - 1] &&
                    !isPathSeparator(value[i - 1]))
                {
                    _extension = value.substr(i, size - i);
                }
                else
                {
                    i = size;
                }

                // Find the number.
                size_t j = i;
                for (; i > 0 && value[i - 1] >= '0' && value[i - 1] <= '9'; --i)
                    ;
                if (value[i] >= '0' && value[i] <= '9' &&
                    (j - i) <= options.maxNumberDigits)
                {
                    _number = value.substr(i, j - i);
                    _numberValue = std::atoi(_number.c_str());
                    _numberDigits = math::digits(_numberValue);
                    _sequence = math::IntRange(_numberValue, _numberValue);
                    if (_number.size() > 1 && '0' == _number[0])
                    {
                        _padding = _number.size();
                    }
                }
                else
                {
                    i = j;
                }

                // Find the directory.
                j = i;
                for (; i > 0 && !isPathSeparator(value[i]); --i)
                    ;
                size_t k = 0;
                if (isPathSeparator(value[i]))
                {
                    _directory = value.substr(0, i + 1);
                    k = i + 1;
                }

                // Find the base name.
                if (k < j)
                {
                    _baseName = value.substr(k, j - k);
                }

                // Special case for Windows drive letters.
                if (_directory.empty() &&
                    2 == _baseName.size() &&
                    _baseName[0] >= 'A' &&
                    _baseName[0] <= 'Z' &&
                    ':' == _baseName[1])
                {
                    _directory.swap(_baseName);
                }
            }
        }

        Path::Path(
            const std::string& directory,
            const std::string& baseName,
            const std::string& number,
            uint8_t padding,
            const std::string& extension) :
            _directory(directory),
            _baseName(baseName),
            _number(number),
            _padding(padding),
            _extension(extension)
        {}

        Path::Path(
            const std::string& directory,
            const std::string& value,
            const PathOptions& options) :
            Path(appendSeparator(directory) + value, options)
        {}

        std::string Path::get(int number, bool directory) const
        {
            std::stringstream ss;
            if (directory)
            {
                ss << _directory;
            }
            ss << _baseName;
            if (number != -1)
            {
                ss << std::setfill('0') << std::setw(_padding) << number;
            }
            else
            {
                ss << _number;
            }
            ss << _extension;
            return ss.str();
        }

        void Path::setSequence(const math::IntRange& value)
        {
            _sequence = value;
        }

        bool Path::sequence(const Path& value) const
        {
            return
                !_number.empty() &&
                !value._number.empty() &&
                _directory == value._directory &&
                _baseName == value._baseName &&
                (_padding == value._padding || _padding == value._numberDigits) &&
                _extension == value._extension;
        }

        std::string Path::getSequenceString() const
        {
            std::string out;
            if (isSequence())
            {
                out = string::Format("{0}-{1}").
                    arg(_sequence.getMin(), _padding, '0').
                    arg(_sequence.getMax(), _padding, '0');
            }
            return out;
        }

        bool Path::isEmpty() const
        {
            return
                _directory.empty() &&
                _baseName.empty() &&
                _number.empty() &&
                _extension.empty();
        }

        bool Path::isAbsolute() const
        {
            const std::size_t size = _directory.size();
            if (size > 0 && isPathSeparator(_directory[0]))
            {
                return true;
            }
            if (size > 1 &&
                _directory[0] >= 'A' && _directory[0] <= 'Z' && ':' == _directory[1])
            {
                return true;
            }
            return false;
        }

        bool Path::operator == (const Path& other) const
        {
            return
                _directory == other._directory &&
                _baseName == other._baseName &&
                _number == other._number &&
                _sequence == other._sequence &&
                _padding == other._padding &&
                _extension == other._extension;
        }

        bool Path::operator != (const Path& other) const
        {
            return !(*this == other);
        }

        std::string appendSeparator(const std::string& value)
        {
            std::string out = value;
            const size_t size = out.size();
            char separator = pathSeparator;
            for (size_t i = 0; i < size; ++i)
            {
                if (out[i] == pathSeparators[0])
                {
                    separator = pathSeparators[0];
                    break;
                }
                else if(out[i] == pathSeparators[1])
                {
                    separator = pathSeparators[1];
                    break;
                }
            }
            if (size > 0 && !isPathSeparator(out[size - 1]))
            {
                out += separator;
            }
            return out;
        }

        std::string getParent(const std::string& value)
        {
            char startSeparator = 0;
            if (!value.empty() && isPathSeparator(value[0]))
            {
                startSeparator = value[0];
            }
            auto v = string::split(value, pathSeparators);
            if (startSeparator || v.size() > 1)
            {
                v.pop_back();
            }
            std::string out;
            if (startSeparator)
            {
                out += startSeparator;
            }
            out += string::join(v, pathSeparator);
            return out;
        }

        TLRENDER_ENUM_IMPL(
            UserPath,
            "Home",
            "Desktop",
            "Documents",
            "Downloads");
        TLRENDER_ENUM_SERIALIZE_IMPL(UserPath);
    }
}
