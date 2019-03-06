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

#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>

#include "glvle.hpp"

#include "imgui.h"

namespace vle {
namespace glvle {

Gltxt::Gltxt(const std::string& file_)
    : file(file_)
    , st(Gltxt::status::reading)
{

    try {
        future_content = std::async([](const std::string& file)
                {
                    vle::utils::Path f(file);

                    if (!f.exists())
                        return std::make_tuple(std::string(), status::access_file_error);

                    if (f.file_size() > 1024 * 1024)
                        return std::make_tuple(std::string(), status::big_file_error);
                    std::ifstream t(file);
                    if (!t.is_open())
                        return std::make_tuple(std::string(), status::open_file_error);

                    std::stringstream buffer;
                    buffer << t.rdbuf();
                    return std::make_tuple(buffer.str(), status::success);
                }, file);
    } catch (const std::exception& /*e*/) {
        st = Gltxt::status::internal_error;
    }
}

bool
Gltxt::show()
{
    bool ret = true;

    ImGui::SetNextWindowSize(ImVec2(600, 600), true);
    if (!ImGui::Begin(file.c_str(), &ret)) {
        ImGui::End();
        return ret;
    }

    constexpr ImGuiInputTextFlags flags =
                ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_ReadOnly;

    switch (st) {
        case status::success:

            ImGui::InputTextMultiline("##source",
                    &content[0],
                    content.size(),
                    ImGui::GetContentRegionAvail(),
                    flags);
            break;
        case status::uninitialized:
            break;
        case status::reading:
            if (future_content.valid()) {
                auto s = future_content.wait_for(std::chrono::seconds(0));
                if (s == std::future_status::ready)
                    std::tie(content, st) = future_content.get();
            }
            break;
        case status::internal_error:
            ImGui::Text("Internal error");
            break;
        case status::access_file_error:
            ImGui::Text("Fail to access file");
            break;
        case status::open_file_error:
            ImGui::Text("Fail to open file");
            break;
        case status::big_file_error:
            ImGui::Text("File too big for glvle");
            break;
    }

    ImGui::End();
    return ret;
}

} // namespace glvle
} // namespace vle
