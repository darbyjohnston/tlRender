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
            const std::string& fileName,
            const PathOptions& options)
        {
            if (!fileName.empty())
            {
                // Find the extension.
                const size_t size = fileName.size();
                size_t i = size - 1;
                for (; i > 0 && fileName[i] != '.' && !isPathSeparator(fileName[i]); --i)
                    ;
                if (i > 0 &&
                    '.' == fileName[i] &&
                    '.' != fileName[i - 1] &&
                    !isPathSeparator(fileName[i - 1]))
                {
                    _extension = fileName.substr(i, size - i);
                }
                else
                {
                    i = size;
                }

                // Find the number.
                size_t j = i;
                for (; i > 0 && fileName[i - 1] >= '0' && fileName[i - 1] <= '9'; --i)
                    ;
                if (fileName[i] >= '0' && fileName[i] <= '9' &&
                    (j - i) <= options.maxNumberDigits)
                {
                    _number = fileName.substr(i, j - i);
                }
                else
                {
                    i = j;
                }

                // Find the directory.
                j = i;
                for (; i > 0 && !isPathSeparator(fileName[i]); --i)
                    ;
                size_t k = 0;
                if (isPathSeparator(fileName[i]))
                {
                    // Find the protocol.
                    size_t l = i;
                    for (; l > 0 && fileName[l] != ':'; --l)
                        ;
                    if (':' == fileName[l] &&
                        4 == l &&
                        'f' == fileName[0] &&
                        'i' == fileName[1] &&
                        'l' == fileName[2] &&
                        'e' == fileName[3] &&
                        l < size - 4 &&
                        '/' == fileName[l + 1] &&
                        '/' == fileName[l + 2] &&
                        '/' == fileName[l + 3])
                    {
                        _protocol = fileName.substr(0, l + 3);
                        l += 3;
                    }
                    else if (':' == fileName[l] &&
                        4 == l &&
                        'f' == fileName[0] &&
                        'i' == fileName[1] &&
                        'l' == fileName[2] &&
                        'e' == fileName[3] &&
                        l < size - 3 &&
                        '/' == fileName[l + 1] &&
                        '/' == fileName[l + 2])
                    {
                        _protocol = fileName.substr(0, l + 3);
                        l += 3;
                    }
                    else if (':' == fileName[l] &&
                        4 == l &&
                        'f' == fileName[0] &&
                        'i' == fileName[1] &&
                        'l' == fileName[2] &&
                        'e' == fileName[3] &&
                        l < size - 2 &&
                        '/' == fileName[l + 1])
                    {
                        _protocol = fileName.substr(0, l + 1);
                        l += 1;
                    }
                    else if (
                        ':' == fileName[l] &&
                        4 == l &&
                        'f' == fileName[0] &&
                        'i' == fileName[1] &&
                        'l' == fileName[2] &&
                        'e' == fileName[3])
                    {
                        _protocol = fileName.substr(0, l + 1);
                    }
                    else if (':' == fileName[l] &&
                        l > 1 &&
                        l < size - 3 &&
                        '/' == fileName[l + 1] &&
                        '/' == fileName[l + 2])
                    {
                        _protocol = fileName.substr(0, l + 3);
                        l += 3;
                    }
                    else
                    {
                        l = 0;
                    }

                    _directory = fileName.substr(l, (i - l) + 1);
                    k = i + 1;
                }

                // Find the base name.
                if (k < j)
                {
                    _baseName = fileName.substr(k, j - k);
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

                _protocolUpdate();
                _numberUpdate();
            }
        }

        Path::Path(
            const std::string& directory,
            const std::string& fileName,
            const PathOptions& options) :
            Path(appendSeparator(directory) + fileName, options)
        {
            _protocolUpdate();
            _numberUpdate();
        }

        Path::Path(
            const std::string& directory,
            const std::string& baseName,
            const std::string& number,
            size_t padding,
            const std::string& extension,
            const std::string& protocol) :
            _protocol(protocol),
            _directory(directory),
            _baseName(baseName),
            _number(number),
            _padding(padding),
            _extension(extension)
        {
            _protocolUpdate();
            _numberUpdate();
        }

        std::string Path::get(int number, PathType type) const
        {
            std::stringstream ss;
            switch (type)
            {
            case PathType::Full:
                ss << _protocol;
            case PathType::Path:
                ss << _directory;
                break;
            default: break;
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

        void Path::setProtocol(const std::string& value)
        {
            if (value == _protocol)
                return;
            _protocol = value;
            _protocolUpdate();
        }

        void Path::setDirectory(const std::string& value)
        {
            _directory = value;
        }
        
        void Path::setBaseName(const std::string& value)
        {
            _baseName = value;
        }
        
        void Path::setNumber(const std::string& value)
        {
            if (value == _number)
                return;
            _number = value;
            _numberUpdate();
        }
        
        void Path::setSequence(const math::IntRange& value)
        {
            _sequence = value;
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

        void Path::setExtension(const std::string& value)
        {
            _extension = value;
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

        void Path::_protocolUpdate()
        {
            if (!_protocol.empty())
            {
                const auto i = _protocol.find_first_of(':');
                if (i != std::string::npos)
                {
                    _protocolName = _protocol.substr(0, i + 1);
                }
            }
            else
            {
                _protocolName = std::string();
            }
        }
        
        void Path::_numberUpdate()
        {
            _numberValue = std::atoi(_number.c_str());
            _numberDigits = math::digits(_numberValue);
            _sequence = math::IntRange(_numberValue, _numberValue);
            if (_number.size() > 1 && '0' == _number[0])
            {
                _padding = _number.size();
            }
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
