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
text_window(const std::string& file, Gltxt& txt)
{
    bool ret = true;

    ImGui::SetNextWindowSize(ImVec2(600, 600), true);
    if (!ImGui::Begin(file.c_str(), &ret)) {
        ImGui::End();
        return ret;
    }

    if (txt.st == Gltxt::status::uninitialized) {
        vle::utils::Path f(file);

        if (!f.exists()) {
            txt.st = Gltxt::status::access_file_error;
        } else {
            if (f.file_size() <= 1024 * 1024) {
                std::ifstream t(file);
                if (!t.is_open()) {
                    txt.st = Gltxt::status::open_file_error;
                } else {
                    std::stringstream buffer;
                    buffer << t.rdbuf();
                    txt.content = buffer.str();

                    txt.st = Gltxt::status::success;
                }
            } else {
                txt.st = Gltxt::status::big_file_error;
            }
        }
    }

    if (txt.st == Gltxt::status::success) {
        ImGuiInputTextFlags flags =
            ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_ReadOnly;

        ImGui::InputTextMultiline("##source",
                                  &txt.content[0],
                                  txt.content.size(),
                                  ImGui::GetContentRegionAvail(),
                                  flags);
    } else {
        if (txt.st == Gltxt::status::access_file_error)
            ImGui::Text("Fail to access file");
        else if (txt.st == Gltxt::status::open_file_error)
            ImGui::Text("Fail to open file");
        else if (txt.st == Gltxt::status::big_file_error)
            ImGui::Text("File too big for glvle");
        else
            ImGui::Text("Internal error");
    }

    ImGui::End();
    return ret;
}

} // namespace glvle
} // namespace vle
