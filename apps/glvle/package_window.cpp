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

#include <vle/utils/Filesystem.hpp>

#include <cassert>
#include <cstdio>

#include "imgui.h"

namespace vle {
namespace glvle {

void
package_window(Glvle& gv)
{
    assert(gv.show_package_window);
    assert(gv.pkg);

    if (!ImGui::Begin("Package window", nullptr)) {
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), gv.package.c_str());
    ImGui::Text("Binary: %s", gv.pkg->getDir().c_str());
    ImGui::Text("Source: %s", gv.pkg->getDir(vle::utils::PKG_SOURCE).c_str());

    float width = ImGui::GetContentRegionAvailWidth() / 5.f;

    if (ImGui::Button("configure", ImVec2(width, 0))) {
    }
    ImGui::SameLine();
    if (ImGui::Button("build", ImVec2(width, 0))) {
    }
    ImGui::SameLine();
    if (ImGui::Button("install", ImVec2(width, 0))) {
    }
    ImGui::SameLine();
    if (ImGui::Button("clean", ImVec2(width, 0))) {
    }
    ImGui::SameLine();
    if (ImGui::Button("test", ImVec2(width, 0))) {
    }

    ImGui::End();
}

} // namespace glvle
} // namespace vle
