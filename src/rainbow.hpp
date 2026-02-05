#pragma once

#include <stdint.h>
#include <string>

#include "args.hpp"

class Rainbow {
public:
    explicit Rainbow(const cli::Options &options);

    void process(std::istream &in);

private:
    void print_line(const std::string &line) const;

    [[nodiscard]] bool is_tty() const;

private:
    float m_spread;
    float m_speed;
    float m_freq;
    int m_duration;
    int m_color_offset = 0;
    int m_line_count = 0;
    bool m_invert;
    bool m_animate;
    bool m_truecolor_mode;
    bool m_force_term;
};
