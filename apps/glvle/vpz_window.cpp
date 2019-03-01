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

#include <fstream>
#include <sstream>

#include "glvle.hpp"

#include "imgui.h"

namespace vle {
namespace glvle {

bool
vpz_window(Glvle& /*gl*/, const std::string& file, Glvpz& vpz)
{
    bool ret = true;

    ImGui::SetNextWindowSize(ImVec2(600, 600), true);
    if (!ImGui::Begin(file.c_str(), &ret)) {
        ImGui::End();
        return ret;
    }

    if (vpz.st == vle::glvle::Glvpz::status::uninitialized) {
        try {
            vpz.vpz->parseFile(file);

        } catch (const std::exception& e) {
            vpz.st = vle::glvle::Glvpz::status::read_error;
        }
    }

    switch (vpz.st) {
    case vle::glvle::Glvpz::status::success:
        break;
    case vle::glvle::Glvpz::status::uninitialized:
        break;
    case vle::glvle::Glvpz::status::read_error: {
        bool dummy_open = true;
        if (ImGui::BeginPopupModal("Alert", &dummy_open)) {
            ImGui::Text("Error when reading vpz file");
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    } break;
    }

    ImGui::End();
    return ret;
}

} // namespace glvle
} // namespace vle
