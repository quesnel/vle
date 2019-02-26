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
show_app_menubar()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            static bool show_open_project = false;
            if (ImGui::MenuItem("Open project", "Ctrl+O")) {
                show_open_project = true;
                ImGui::OpenPopup("Open package");
                printf("show_open_project %d\n", show_open_project);
            }

            printf("show_open_project %d\n", show_open_project);

            if (show_open_project) {
                printf("show_open_project %d\n", show_open_project);
                std::string selected;
                if (select_directory_dialog(
                      "Open package",
                      "Select the VPZ file of the package",
                      "/home/gquesnel/devel/bits",
                      selected)) {
                    printf("%s\n", selected.c_str());
                    show_open_project = false;
                }
            }

            // if (show_open_project) {
            //     std::string selected;
            //     if (select_directory_dialog("Open package",
            //                                 "Select the package",
            //                                 "/home/gquesnel",
            //                                 selected)) {
            //         printf("%s\n", selected.c_str());
            //         show_open_project = false;
            //     }
            // }
            // ImGui::Separator();
            // if (ImGui::MenuItem("Save project", "Ctrl+S")) {
            // }
            // if (ImGui::MenuItem("Quit", "Alt+F4")) {
            // }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
}
}
