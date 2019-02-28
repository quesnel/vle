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
show_app_menubar(Glvle& gv)
{
    bool new_box = false;
    bool open_box = false;
    std::string path_name;
    std::string dir_name;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N"))
                new_box = true;
            if (ImGui::MenuItem("Open", "Ctrl+O"))
                open_box = true;

            ImGui::Separator();
            if (ImGui::MenuItem("Close", nullptr, gv.have_package)) {
                gv.show_package_window = false;
                gv.package.clear();
                gv.working_dir.clear();
                gv.have_package = false;
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Ctrl+Q", false, false)) {
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Main menu bar", nullptr, gv.show_main_menubar);
            ImGui::MenuItem("Package window",
                            nullptr,
                            &gv.show_package_window,
                            gv.have_package);
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
                                    path_name,
                                    dir_name)) {
        if (!path_name.empty() && !dir_name.empty()) {
            try {
                vle::utils::Path::current_path(path_name);
                gv.pkg =
                  std::make_shared<vle::utils::Package>(gv.ctx, dir_name);
                gv.pkg->create();

                gv.show_package_window = true;
                gv.package = dir_name;
                gv.working_dir = path_name;
                gv.have_package = true;
            } catch (const std::exception& e) {
                printf("%s.\n", e.what());
            }
        }
    }

    if (select_directory_dialog("Open package",
                                "Select a new directory",
                                "/home/gquesnel/devel/bits",
                                path_name,
                                dir_name)) {
        if (!dir_name.empty()) {
            try {
                vle::utils::Path p(path_name);
                vle::utils::Path::current_path(p);
                gv.pkg =
                  std::make_shared<vle::utils::Package>(gv.ctx, dir_name);

                gv.show_package_window = true;
                gv.package = dir_name;
                gv.working_dir = path_name;
                gv.have_package = true;
            } catch (const std::exception& e) {
                printf("%s.\n", e.what());
            }
        }
    }
}

} // namespace glvle
} // namespace vle
