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
text_window(const std::string& file, std::string& content)
{
    bool ret = true;

    ImGui::SetNextWindowSize(ImVec2(600, 600), true);
    if (!ImGui::Begin(file.c_str(), &ret)) {
        ImGui::End();
        return ret;
    }

    if (content.empty()) {
        vle::utils::Path f(file);
        if (f.file_size() <= 1024 * 1024) {
            std::ifstream t(file);
            std::stringstream buffer;
            buffer << t.rdbuf();
            content = buffer.str();
        }
    }

    ImGuiInputTextFlags flags =
      ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_ReadOnly;

    ImGui::InputTextMultiline("##source",
                              &content[0],
                              content.size(),
                              ImGui::GetContentRegionAvail(),
                              flags);

    ImGui::End();
    return ret;
}

} // namespace glvle
} // namespace vle
