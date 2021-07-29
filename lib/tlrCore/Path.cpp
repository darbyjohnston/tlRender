// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Path.h>

#include <tlrCore/String.h>

#include <fseq.h>

#include <iomanip>
#include <sstream>

namespace tlr
{
    namespace file
    {
        Path::Path()
        {}

        Path::Path(const std::string& value)
        {
            std::string tmp(value);
            for (auto i = tmp.begin(); i != tmp.end(); ++i)
            {
                if ('\\' == *i)
                {
                    *i = '/';
                }
            }

            FSeqFileName f;
            fseqFileNameInit(&f);
            fseqFileNameSplit(tmp.c_str(), &f, string::cBufferSize);
            _directory = f.path;
            _baseName = f.base;
            if (_directory.empty() &&
                2 == _baseName.size() &&
                _baseName[0] >= 'A' &&
                _baseName[0] <= 'Z' &&
                ':' == _baseName[1])
            {
                _directory.swap(_baseName);
            }
            _number = f.number;
            _padding = !_number.empty() ? ('0' == _number[0] ? _number.size() : 0) : 0;
            _extension = f.extension;
            fseqFileNameDel(&f);
        }

        namespace
        {
            std::string directoryFix(const std::string& value)
            {
                std::string out = value;
                const size_t size = out.size();
                if (size > 0 && !('/' == out[size - 1] || '\\' == out[size - 1]))
                {
                    out += '/';
                }
                return out;
            }
        }

        Path::Path(const std::string& directory, const std::string& value) :
            Path(directoryFix(directory) + value)
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

        bool Path::isEmpty() const
        {
            return _directory.empty() &&
                _baseName.empty() &&
                _number.empty() &&
                _extension.empty();
        }

        bool Path::isAbsolute() const
        {
            const std::size_t size = _directory.size();
            if (size > 0 && '/' == _directory[0])
            {
                return true;
            }
            else if (size > 0 && '\\' == _directory[0])
            {
                return true;
            }
            if (size > 1 && _directory[0] >= 'A' && _directory[0] <= 'Z' && ':' == _directory[1])
            {
                return true;
            }
            return false;
        }

        bool Path::operator == (const Path& other) const
        {
            return _directory == other._directory &&
                _baseName == other._baseName &&
                _number == other._number &&
                _padding == other._padding &&
                _extension == other._extension;
        }

        bool Path::operator != (const Path& other) const
        {
            return !(*this == other);
        }
    }
}
