// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/StringFormat.h>

#include <tlCore/Error.h>

#include <iomanip>
#include <map>
#include <sstream>

namespace tl
{
    namespace string
    {
        Format::Format(const std::string& value) :
            _text(value)
        {}

        Format& Format::arg(const std::string& value)
        {
            std::map<int, std::pair<size_t, size_t> > args;

            enum class Parse
            {
                None,
                StartBracket,
                Number,
                EndBracket
            };
            Parse parse = Parse::None;

            size_t pos = 0;
            size_t size = 0;

            for (size_t i = 0; i < _text.size(); ++i)
            {
                Parse newParse = Parse::None;
                if ('{' == _text[i])
                {
                    newParse = Parse::StartBracket;
                }
                else if ('}' == _text[i])
                {
                    newParse = Parse::EndBracket;
                }
                else if (_text[i] >= '0' && _text[i] <= '9')
                {
                    newParse = Parse::Number;
                }
                
                switch (newParse)
                {
                case Parse::StartBracket:
                    if (Parse::None == parse ||
                        Parse::EndBracket == parse)
                    {
                        parse = newParse;
                        pos = i;
                        size = 1;
                    }
                    else
                    {
                        parse = Parse::None;
                    }
                    break;
                case Parse::Number:
                    if (Parse::StartBracket == parse ||
                        Parse::Number == parse)
                    {
                        parse = newParse;
                        ++size;
                    }
                    else
                    {
                        parse = Parse::None;
                    }
                    break;
                case Parse::EndBracket:
                    if (Parse::Number == parse)
                    {
                        parse = newParse;
                        ++size;
                    }
                    else
                    {
                        parse = Parse::None;
                    }
                    break;
                default:
                    parse = Parse::None;
                    break;
                }
                
                if (Parse::EndBracket == parse)
                {
                    const int arg = std::stoi(_text.substr(pos + 1, size - 2));
                    const auto i = args.find(arg);
                    if (i == args.end())
                    {
                        args[arg] = std::make_pair(pos, size);
                    }
                    else
                    {
                        _error = "Duplicate argument";
                        break;
                    }
                }
            }
            if (_error.empty())
            {
                if (!args.empty())
                {
                    _text.replace(args.begin()->second.first, args.begin()->second.second, value);
                }
                else
                {
                    _error = "Argument not found";
                }
            }
            return *this;
        }

        Format& Format::arg(int value, int width, char pad)
        {
            std::stringstream ss;
            ss << std::setfill(pad) << std::setw(width) << value;
            return arg(ss.str());
        }

        Format& Format::arg(int8_t value, int width, char pad)
        {
            std::stringstream ss;
            ss << std::setfill(pad) << std::setw(width) << static_cast<int>(value);
            return arg(ss.str());
        }

        Format& Format::arg(uint8_t value, int width, char pad)
        {
            std::stringstream ss;
            ss << std::setfill(pad) << std::setw(width) << static_cast<int>(value);
            return arg(ss.str());
        }

        Format& Format::arg(int16_t value, int width, char pad)
        {
            std::stringstream ss;
            ss << std::setfill(pad) << std::setw(width) << static_cast<int>(value);
            return arg(ss.str());
        }

        Format& Format::arg(uint16_t value, int width, char pad)
        {
            std::stringstream ss;
            ss << std::setfill(pad) << std::setw(width) << static_cast<int>(value);
            return arg(ss.str());
        }

        Format& Format::arg(float value, int precision, int width, char pad)
        {
            std::stringstream ss;
            if (precision >= 0)
            {
                ss.precision(precision);
                ss << std::fixed;
            }
            ss << std::setfill(pad) << std::setw(width) << value;
            return arg(ss.str());
        }

        Format& Format::arg(double value, int precision, int width, char pad)
        {
            std::stringstream ss;
            if (precision >= 0)
            {
                ss.precision(precision);
                ss << std::fixed;
            }
            ss << std::setfill(pad) << std::setw(width) << value;
            return arg(ss.str());
        }

        bool Format::hasError() const
        {
            return _error.size() > 0;
        }

        const std::string& Format::getError() const
        {
            return _error;
        }

        Format::operator std::string() const
        {
            return _text;
        }
    }
}
