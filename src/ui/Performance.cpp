#include "ui/Performance.hpp"

#include "core/AsyncTaskQueue.hpp"
#include "core/Project.hpp"
#include <imgui.h>

void Performance::render(double delta_time)
{
    auto project = Project::get_current();

    m_current_time += delta_time;

    if (m_current_time - m_last_updated >= m_update_interval) {
        m_last_updated = m_current_time;
        m_last_total_background_tasks = m_total_background_tasks;
        m_total_background_tasks = AsyncTaskQueue::background.num_total_queued_tasks();
    }

    for (std::size_t i = 0; i < m_frametimes.size(); ++i) {
        if (i + 1 == m_frametimes.size()) {
            m_frametimes[i] = delta_time;
            break;
        }
        m_frametimes[i] = m_frametimes[i + 1];
    }

    if (ImGui::Begin("Performance")) {
        ImGui::Text("fps: %f", 1.0f / delta_time);
        ImGui::Text("frametime: %f s", delta_time);
        ImGui::PlotLines("frametime", m_frametimes.data(), m_frametimes.size(), 0, nullptr, 0.0f);
        ImGui::Text("current background tasks: %zu", AsyncTaskQueue::background.num_queued_tasks());
        ImGui::Text("total background tasks over %f s: %zu", m_update_interval, m_total_background_tasks - m_last_total_background_tasks);
        ImGui::Text("total background tasks: %zu", AsyncTaskQueue::background.num_total_queued_tasks());
        ImGui::Text("total models: %zu", project->m_models.size());
        ImGui::Text("total textures: %zu", project->m_textures.size());
    }
    ImGui::End();
}
