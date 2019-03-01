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

#include <boost/variant.hpp>

#include <cassert>
#include <cstdio>

#include "imgui.h"

namespace vle {
namespace glvle {

struct file;
struct directory;

using node = boost::variant<file, directory>;

struct file
{
    vle::utils::Path path;

    file() = default;

    file(vle::utils::Path path_)
      : path(std::move(path_))
    {}
};

struct directory
{
    vle::utils::Path path;
    std::vector<node> children;

    directory() = default;

    directory(vle::utils::Path path_)
      : path(std::move(path_))
    {}
};

struct node_visitor : public boost::static_visitor<void>
{
    Glvle& gv;
    // const vle::utils::Path& current;

    // node_visitor(Glvle& gv_, const vle::utils::Path& current_)
    //   : gv(gv_)
    //   , current(current_)
    // {}
    node_visitor(Glvle& gv_)
      : gv(gv_)
    {}

    void operator()(const file& f) const
    {
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                        ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                        ImGuiTreeNodeFlags_Leaf |
                                        ImGuiTreeNodeFlags_NoTreePushOnOpen;

        ImGui::TreeNodeEx(f.path.filename().c_str(), node_flags);
        if (ImGui::IsItemClicked()) {
            if (f.path.extension() == ".vpz")
                gv.vpz_files[f.path.string()] = vle::glvle::Glvpz();
            else
                gv.txt_files[f.path.string()] = std::string();
        }
    }

    void operator()(const directory& dir) const
    {
        if (ImGui::TreeNode(dir.path.filename().c_str())) {
            for (const auto& n : dir.children)
                boost::apply_visitor(node_visitor(gv), n);
            ImGui::TreePop();
        }
    }
};

static bool need_refresh = false;
static directory hierarchy;
static std::string local_package;

void
refresh(directory& c)
{
    for (vle::utils::DirectoryIterator it{ c.path }, et{}; it != et; ++it) {
        if (it->is_file()) {
            c.children.emplace_back(file(*it));
        } else if (it->is_directory()) {
            c.children.emplace_back(directory(*it));
            refresh(boost::get<directory>(c.children.back()));
        }
    }

    std::sort(
      c.children.begin(),
      c.children.end(),
      [](const auto& lhs, const auto& rhs) {
          const auto* file_lhs = boost::get<file>(&lhs);
          const auto* file_rhs = boost::get<file>(&rhs);
          const auto* dir_lhs = boost::get<directory>(&lhs);
          const auto* dir_rhs = boost::get<directory>(&rhs);
          if (dir_lhs) {
              if (dir_rhs) {
                  return dir_lhs->path.filename() < dir_rhs->path.filename();
              } else {
                  return true;
              }
          } else {
              if (dir_rhs) {
                  return false;
              } else {
                  return file_lhs->path.filename() < file_rhs->path.filename();
              }
          }
      });
}

directory
refresh(Glvle& gv)
{
    directory current(gv.pkg->getDir(vle::utils::PKG_SOURCE));

    refresh(current);

    return current;
}

void
package_window(Glvle& gv)
{
    assert(gv.show_package_window);
    assert(gv.pkg);

    if (local_package != gv.package || hierarchy.path.empty())
        need_refresh = true;

    ImGui::SetNextWindowSize(ImVec2(350, 500), true);
    if (!ImGui::Begin("Package window", nullptr)) {
        ImGui::End();
        return;
    }

    ImGui::TextColored(
      ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", gv.package.c_str());

    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::TreeNodeEx(gv.package.c_str(), node_flags)) {
        for (const auto& n : hierarchy.children)
            boost::apply_visitor(node_visitor(gv), n);
        ImGui::TreePop();
    }

    if (need_refresh) {
        local_package = gv.package;
        hierarchy = refresh(gv);
        need_refresh = false;
    }

    float width = ImGui::GetContentRegionAvailWidth() / 4.f -
                  2.f * ImGui::GetStyle().FramePadding.x;

    ImGui::Separator();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::Text("Binary: %s", gv.pkg->getDir().c_str());
    ImGui::Text("Source: %s", gv.pkg->getDir(vle::utils::PKG_SOURCE).c_str());
    ImGui::PopStyleVar();

    if (ImGui::Button("configure", ImVec2(width, 0))) {
    }
    ImGui::SameLine();
    if (ImGui::Button("build", ImVec2(width, 0))) {
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
