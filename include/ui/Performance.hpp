#pragma once

#include <array>
#include <cstddef>

struct Performance {
    void render(double delta_time);

private:
    double const m_update_interval = 5.0;
    double m_current_time{0};
    double m_last_updated{-m_update_interval};
    std::size_t m_last_total_background_tasks{0};
    std::size_t m_total_background_tasks{0};
    std::array<float, 100> m_frametimes;
};
