/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems.
 * https://www.vle-project.org
 *
 * Copyright (c) 2003-2018 Gauthier Quesnel <gauthier.quesnel@inra.fr>
 * Copyright (c) 2003-2018 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2018 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and
 * contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "glvle.hpp"

namespace vle {
namespace glvle {

void
Gllog::clear() noexcept
{
    buffer.clear();
    line_offsets.clear();
    line_offsets.push_back(0);
}

void
Gllog::log(const char* fmt, ...)
{
    auto old_size = buffer.size();

    va_list args;
    va_start(args, fmt);
    buffer.appendfv(fmt, args);
    va_end(args);

    for (auto new_size = buffer.size(); old_size < new_size; ++old_size)
        if (buffer[old_size] == '\n')
            line_offsets.push_back(old_size + 1);

    if (auto_scroll)
        scroll_to_bottom = true;
}

void
Gllog::show(Glvle& gv)
{
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Log", &gv.show_log_window);

    if (ImGui::SmallButton("[Debug] Add 5 entries")) {
        static int counter = 0;
        for (int n = 0; n < 5; n++) {
            const char* categories[3] = { "info", "warn", "error" };
            const char* words[] = { "Bumfuzzled",   "Cattywampus",
                                    "Snickersnee",  "Abibliophobia",
                                    "Absquatulate", "Nincompoop",
                                    "Pauciloquent" };
            log("[%05d] [%s] Hello, current time is %.1f, here's a "
                "word: '%s'\n",
                ImGui::GetFrameCount(),
                categories[counter % IM_ARRAYSIZE(categories)],
                ImGui::GetTime(),
                words[counter % IM_ARRAYSIZE(words)]);
            counter++;
        }
    }
    ImGui::End();

    if (!ImGui::Begin("Log", &gv.show_log_window)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginPopup("Options")) {
        if (ImGui::Checkbox("Auto-scroll", &auto_scroll))
            if (auto_scroll)
                scroll_to_bottom = true;
        ImGui::EndPopup();
    }

    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");
    ImGui::SameLine();

    bool need_clear = ImGui::Button("Clear");
    ImGui::SameLine();

    bool need_copy = ImGui::Button("Copy");
    ImGui::SameLine();
    filter.Draw("Filter", -100.0f);

    ImGui::Separator();
    ImGui::BeginChild(
      "scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (need_clear)
        clear();

    if (need_copy)
        ImGui::LogToClipboard();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    const char* buf = buffer.begin();
    const char* buf_end = buffer.end();

    if (filter.IsActive()) {
        for (auto line_no = 0; line_no < line_offsets.Size; line_no++) {
            const char* line_start = buf + line_offsets[line_no];
            const char* line_end = (line_no + 1 < line_offsets.Size)
                                     ? (buf + line_offsets[line_no + 1] - 1)
                                     : buf_end;
            if (filter.PassFilter(line_start, line_end))
                ImGui::TextUnformatted(line_start, line_end);
        }
    } else {
        ImGuiListClipper clipper;
        clipper.Begin(line_offsets.Size);
        while (clipper.Step()) {
            for (int line_no = clipper.DisplayStart;
                 line_no < clipper.DisplayEnd;
                 line_no++) {
                const char* line_start = buf + line_offsets[line_no];
                const char* line_end =
                  (line_no + 1 < line_offsets.Size)
                    ? (buf + line_offsets[line_no + 1] - 1)
                    : buf_end;
                ImGui::TextUnformatted(line_start, line_end);
            }
        }
        clipper.End();
    }

    ImGui::PopStyleVar();

    if (scroll_to_bottom)
        ImGui::SetScrollHereY(1.0f);
    scroll_to_bottom = false;
    ImGui::EndChild();
    ImGui::End();
}

} // glvle
} // vle
