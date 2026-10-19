// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef VLC_ML_HISTORY_FILTER_HPP
#define VLC_ML_HISTORY_FILTER_HPP

#include <regex>
#include <string>

namespace vlc::medialibrary {

class HistoryFilter
{
public:
    explicit HistoryFilter(const std::string& pattern) : m_enabled(!pattern.empty())
    {
        if (!m_enabled) return;
        try {
            m_expression = std::regex(pattern, std::regex::ECMAScript | std::regex::icase);
        } catch (const std::regex_error&) {
            m_valid = false;
        }
    }

    bool valid() const { return m_valid; }
    bool excludes(const std::string& uri, const std::string& path) const
    {
        // Invalid hand-edited configuration must not save private history.
        return !m_valid || (m_enabled && (std::regex_search(uri, m_expression)
                                       || std::regex_search(path, m_expression)));
    }

private:
    bool m_enabled;
    bool m_valid = true;
    std::regex m_expression;
};

}
#endif
