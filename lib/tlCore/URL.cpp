// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/URL.h>

#include <iomanip>
#include <regex>
#include <sstream>

namespace tl
{
    std::string scheme(std::string const& url)
    {
        const std::regex rx("^([A-Za-z0-9+-\\.]+://)");
        const auto rxi = std::sregex_iterator(url.begin(), url.end(), rx);
        return rxi != std::sregex_iterator() ? rxi->str() : std::string();
    }

    namespace url
    {
        std::string encode(std::string const& url)
        {
            // Don't encode these characters.
            const std::vector<char> chars =
            {
                '-', '.', '_', '~', ':', '/', '?',  '#',
                '[', ']', '@', '!', '$', '&', '\'', '(',
                ')', '*', '+', ',', ';', '=', '\\'
            };

            // Copy characters to the result, encoding if necessary.
            std::stringstream ss;
            ss.fill('0');
            ss << std::hex;
            for (auto i = url.begin(), end = url.end(); i != end; ++i)
            {
                const auto j = std::find(chars.begin(), chars.end(), *i);
                if (std::isalnum(*i) || j != chars.end())
                {
                    ss << *i;
                }
                else
                {
                    ss << '%' << std::setw(2) << int(*i);
                }
            }
            return ss.str();
        }

        std::string decode(std::string const& url)
        {
            std::string out;

            // Find all percent encodings.
            size_t url_pos = 0;
            const std::regex rx("(%[0-9A-Fa-f][0-9A-Fa-f])");
            for (auto i = std::sregex_iterator(url.begin(), url.end(), rx);
                i != std::sregex_iterator();
                ++i)
            {
                // Copy parts without any encodings.
                if (url_pos != static_cast<size_t>(i->position()))
                {
                    out.append(url.substr(url_pos, i->position() - url_pos));
                    url_pos = i->position() + i->str().size();
                }

                // Convert the encoding and append it.
                std::stringstream ss;
                ss << std::hex << i->str().substr(1);
                unsigned int j = 0;
                ss >> j;
                out.push_back(char(j));
            }

            // Copy the remainder without any encodings.
            if (!url.empty() && url_pos != url.size() - 1)
            {
                out.append(url.substr(url_pos, url.size() - url_pos));
            }

            return out;
        }
    }
}
