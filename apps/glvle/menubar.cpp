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
    bool new_box = false;
    bool open_box = false;
    std::string selected;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N"))
                new_box = true;
            if (ImGui::MenuItem("Open", "Ctrl+O"))
                open_box = true;

            ImGui::Separator();
            if (ImGui::MenuItem("Save project", "Ctrl+S", false, false)) {
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Ctrl+Q", false, false)) {
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (new_box)
        ImGui::OpenPopup("New package");

    if (open_box)
        ImGui::OpenPopup("Open package");

    if (select_new_directory_dialog("New package",
                                    "Select a new directory",
                                    "/home/gquesnel/devel/bits",
                                    selected)) {
        printf("select_directory_dialog Yes '%s'!", selected.c_str());
    }

    if (select_directory_dialog("Open package",
                                "Select a new directory",
                                "/home/gquesnel/devel/bits",
                                selected)) {
        printf("select_directory_dialog Yes '%s'!", selected.c_str());
    }
}

} // namespace glvle
} // namespace vle
