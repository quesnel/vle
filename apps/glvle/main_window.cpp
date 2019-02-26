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

#include <cstdio>

#include "imgui.h"

namespace vle {
namespace glvle {

void
show_main_window()
{
    ImGuiWindowFlags window_flags = 0;

    // window_flags |= ImGuiWindowFlags_NoTitleBar;
    // window_flags |= ImGuiWindowFlags_NoScrollbar;
    // window_flags |= ImGuiWindowFlags_MenuBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    // window_flags |= ImGuiWindowFlags_NoResize;
    // window_flags |= ImGuiWindowFlags_NoCollapse;
    // window_flags |= ImGuiWindowFlags_NoNav;
    // window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(150, 400), ImGuiCond_FirstUseEver);

    // Main body of the Demo window starts here.
    // Early out if the window is collapsed, as an optimization.
    if (!ImGui::Begin("Main", nullptr, window_flags)) {
        ImGui::End();
        return;
    }

    static char project_name[256] = "vle.adaptive-qss";
    static char vpz_name[256] = "file.vpz";
    static char date_value[256] = "2019-02-25";
    static char author_value[256] =
      "Gauthier Quesnel <gauthier.quesnel@inra.fr>";

    static bool show_open_package = false;
    ImGui::InputText("project", project_name, IM_ARRAYSIZE(project_name));
    if (ImGui::Button("...##pkg")) {
        show_open_package = true;
        ImGui::OpenPopup("Open package");
    }

    if (show_open_package) {
        std::string selected;
        if (select_directory_dialog("Open package",
                                    "Select the VPZ file of the package",
                                    "/home/gquesnel/devel/bits",
                                    selected)) {
            printf("%s\n", selected.c_str());
            show_open_package = false;
        }
    }

    ImGui::InputText("vpz", vpz_name, IM_ARRAYSIZE(vpz_name));
    ImGui::SameLine();
    static bool show_open_file = false;

    if (ImGui::Button("...##vpz")) {
        show_open_file = true;
        ImGui::OpenPopup("Open Vpz");
    }

    if (show_open_file) {
        std::string selected;
        if (select_file_dialog("Open Vpz",
                               "Select the VPZ file of the package",
                               ".vpz",
                               "/home/gquesnel/devel/bits",
                               selected)) {
            printf("%s\n", selected.c_str());
            show_open_file = false;
        }
    }

    static bool show_simulation_parameters = true;

    if (ImGui::CollapsingHeader("Configuration")) {
        ImGui::LabelText("label", "Value");

        ImGui::InputText("author", author_value, IM_ARRAYSIZE(author_value));
        ImGui::InputText("date", date_value, IM_ARRAYSIZE(date_value));

        static double begin_time = 0.0;
        ImGui::InputDouble("begin date", &begin_time, -100, 100, "%.8f");

        static double duration = 100.0;
        ImGui::InputDouble("duration", &duration, 0.01f, DBL_MAX, "%.8f");
    }

    ImGui::End();
}

} // namespace glvle
} // namespace vle
