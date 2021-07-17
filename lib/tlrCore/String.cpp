// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/String.h>

#include <algorithm>
#include <codecvt>
#include <locale>

namespace tlr
{
    namespace string
    {
        std::vector<std::string> split(const std::string& s, char delimeter, bool keepEmpty)
        {
            std::vector<std::string> out;
            bool word = false;
            std::size_t wordStart = 0;
            std::size_t i = 0;
            for (; i < s.size(); ++i)
            {
                if (s[i] != delimeter)
                {
                    if (!word)
                    {
                        word = true;
                        wordStart = i;
                    }
                }
                else
                {
                    if (word)
                    {
                        word = false;
                        out.push_back(s.substr(wordStart, i - wordStart));
                    }
                    if (keepEmpty && i > 0 && s[i - 1] == delimeter)
                    {
                        out.push_back(std::string());
                    }
                }
            }
            if (word)
            {
                out.push_back(s.substr(wordStart, i - wordStart));
            }
            return out;
        }

        std::vector<std::string> split(const std::string& s, const std::vector<char>& delimeters, bool keepEmpty)
        {
            std::vector<std::string> out;
            bool word = false;
            std::size_t wordStart = 0;
            std::size_t i = 0;
            for (; i < s.size(); ++i)
            {
                if (std::find(delimeters.begin(), delimeters.end(), s[i]) == delimeters.end())
                {
                    if (!word)
                    {
                        word = true;
                        wordStart = i;
                    }
                }
                else
                {
                    if (word)
                    {
                        word = false;
                        out.push_back(s.substr(wordStart, i - wordStart));
                    }
                    if (keepEmpty && i > 0 && std::find(delimeters.begin(), delimeters.end(), s[i - 1]) != delimeters.end())
                    {
                        out.push_back(std::string());
                    }
                }
            }
            if (word)
            {
                out.push_back(s.substr(wordStart, i - wordStart));
            }
            return out;
        }

        std::string join(const std::vector<std::string>& values, const std::string& delimeter)
        {
            std::string out;
            const std::size_t size = values.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                out += values[i];
                if (i < size - 1)
                {
                    out += delimeter;
                }
            }
            return out;
        }

        std::string toUpper(const std::string& value)
        {
            std::string out;
            for (auto i : value)
            {
                out.push_back(std::toupper(i));
            }
            return out;
        }

        std::string toLower(const std::string& value)
        {
            std::string out;
            for (auto i : value)
            {
                out.push_back(std::tolower(i));
            }
            return out;
        }

        bool compareNoCase(const std::string& a, const std::string& b)
        {
            return toLower(a) == toLower(b);
        }

        std::string removeTrailingNewlines(const std::string& value)
        {
            std::string out = value;
            removeTrailingNewlines(out);
            return out;
        }

        void removeTrailingNewlines(std::string& value)
        {
            size_t size = value.size();
            while (size && ('\n' == value[size - 1] || '\r' == value[size - 1]))
            {
                value.pop_back();
                size = value.size();
            }
        }

        std::wstring toWide(const std::string& value)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            return converter.from_bytes(value);
        }

        std::string fromWide(const std::wstring& value)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            return converter.to_bytes(value);
        }

        std::string escape(const std::string& value)
        {
            std::string out;
            for (const auto i : value)
            {
                if ('\\' == i)
                {
                    out.push_back('\\');
                }
                out.push_back(i);
            }
            return out;
        }

        std::string unescape(const std::string& value)
        {
            std::string out;
            const size_t size = value.size();
            for (size_t i = 0; i < size; ++i)
            {
                out.push_back(value[i]);
                if (i < size - 1 && '\\' == value[i] && '\\' == value[i + 1])
                {
                    ++i;
                }
            }
            return out;
        }
    }
}
