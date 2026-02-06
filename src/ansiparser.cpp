#include "ansiparser.hpp"
#include "terminal.hpp"
#include "utf-8.hpp"

AnsiParser::Iterator &AnsiParser::Iterator::operator++() {
    if (m_posistion >= m_data.size()) {
        m_posistion = std::string_view::npos;
        return *this;
    }

    size_t start = m_posistion;
    bool is_ansi_escape = (m_data[m_posistion] == term::ansi_escape) && (m_posistion + 1 < m_data.size()) &&
                          (m_data[m_posistion + 1] == '[');

    if (!is_ansi_escape) {
        unsigned char c = static_cast<unsigned char>(m_data[m_posistion]);
        size_t len = utf8::get_sequence_length(c);

        if (m_posistion + len > m_data.size()) {
            len = m_data.size() - m_posistion;
        }

        m_current = {m_data.substr(m_posistion, len), false};
        m_posistion += len;
        return *this;
    }

    m_posistion += 2; // skip ^[[
    while (m_posistion < m_data.size()) {
        char c = m_data[m_posistion++];
        if (c >= term::ansi_escape_ends_lower && c <= term::ansi_escape_ends_upper)
            break;
    }
    m_current = {m_data.substr(start, m_posistion - start), true};

    return *this;
}

AnsiToken AnsiParser::Iterator::operator*() const { return m_current; }
bool AnsiParser::Iterator::operator!=(const AnsiParser::Iterator &other) const {
    return m_posistion != other.m_posistion;
}

AnsiParser::Iterator AnsiParser::Iterator::operator++(int) {
    Iterator tmp = *this;
    ++(*this);
    return tmp;
}


AnsiParser::Iterator AnsiParser::begin() {
    Iterator it{m_data, 0};
    return ++it;
}

AnsiParser::Iterator AnsiParser::end() { return {m_data, std::string_view::npos}; }
