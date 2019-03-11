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

#include <stack>

#include <cassert>
#include <cstdio>

namespace vle {
namespace glvle {

template<typename T>
inline void
sort_vector(T& vec) noexcept
{
    std::sort(vec.begin(), vec.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.path.string() < rhs.path.string();
    });
}

void
Glpackage::directory::refresh()
{
    for (vle::utils::DirectoryIterator it{ path }, et{}; it != et; ++it) {
        if (it->is_file()) {
            file_child.emplace_back(*it);
        } else if (it->is_directory()) {
            dir_child.emplace_back(*it);
            dir_child.back().refresh();
        }
    }

    sort_vector(file_child);
    sort_vector(dir_child);
}

void
Glpackage::open(vle::utils::ContextPtr ctx,
                const std::string& package_,
                bool create)
{
    package.reset();
    st = status::uninitialized;

    try {
        auto pkg = std::make_shared<vle::utils::Package>(ctx, package_);
        if (create)
            pkg->create();

        package = pkg;
        st = status::reading;

        auto path = vle::utils::Path(package->getDir(vle::utils::PKG_SOURCE));
        if (path.exists()) {
            current = path;
            current.refresh();
            st = status::success;
        } else
            st = status::open_package_error;
    } catch (const std::exception& /*e*/) {
        st = status::open_package_error;
    }
}

bool
Glpackage::show(Glvle& gv)
{
    struct element
    {
        element(const directory& dir_)
          : dir(dir_)
          , read(false)
          , open(false)
        {}

        directory dir;
        bool read;
        bool open;
    };

    ImGui::SetNextWindowSize(ImVec2(350, 500), true);
    if (!ImGui::Begin("Package window", nullptr)) {
        ImGui::End();
        return false;
    }

    ImGui::TextColored(
      ImVec4(1.f, 1.f, 0.f, 1.f), "%s", current.path.string().c_str());

    std::stack<element> stack;
    stack.emplace(current);
    stack.top().open = true;

    while (!stack.empty()) {
        if (stack.top().read) {
            element top = stack.top();
            stack.pop();

            if (top.open) {
                for (const auto& f : top.dir.file_child) {
                    constexpr ImGuiTreeNodeFlags node_flags =
                      ImGuiTreeNodeFlags_OpenOnArrow |
                      ImGuiTreeNodeFlags_OpenOnDoubleClick |
                      ImGuiTreeNodeFlags_Leaf |
                      ImGuiTreeNodeFlags_NoTreePushOnOpen;

                    ImGui::TreeNodeEx(f.path.filename().c_str(), node_flags);
                    if (ImGui::IsItemClicked()) {
                        if (f.path.extension() == ".vpz") {
                            auto& vpz = gv.vpz_files[f.path.string()];
                            vpz.id = std::string("vpz-") +
                                     std::to_string(id_generator++);
                            vpz.open(f.path.string());
                        } else {
                            gv.txt_files.emplace(
                              f.path.string(),
                              vle::glvle::Gltxt(f.path.string()));
                        }
                    }
                }

                ImGui::TreePop();
            }
        } else {
            stack.top().read = true;

            if (ImGui::TreeNodeEx(stack.top().dir.path.filename().c_str())) {
                stack.top().open = true;

                auto top = stack.top().dir;
                std::for_each(
                  top.dir_child.rbegin(),
                  top.dir_child.rend(),
                  [&stack](const auto& elem) { stack.emplace(elem); });
            } else {
                stack.top().open = false;
            }
        }
    }

    const float width = ImGui::GetContentRegionAvailWidth() / 4.f -
                        2.f * ImGui::GetStyle().FramePadding.x;

    ImGui::Separator();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    ImGui::Text("Binary: %s", package->getDir().c_str());
    ImGui::Text("Source: %s", package->getDir(vle::utils::PKG_SOURCE).c_str());
    ImGui::PopStyleVar();

    if (ImGui::Button("configure", ImVec2(width, 0))) {
        gv.log_w.log(
          6, "configure package %s\n", current.path.string().c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("build", ImVec2(width, 0))) {
        gv.log_w.log(6, "build package %s\n", current.path.string().c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("clean", ImVec2(width, 0))) {
        gv.log_w.log(6, "cleam package %s\n", current.path.string().c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("test", ImVec2(width, 0))) {
        gv.log_w.log(6, "test package %s\n", current.path.string().c_str());
    }

    ImGui::End();

    return true;
}

void
Glpackage::clear() noexcept
{
    package.reset();
    current.clear();
}

// void
// package_window(Glvle& gv)
// {
//     assert(gv.show_package_window);
//     assert(gv.pkg);

//     if (local_package != gv.package || hierarchy.path.empty())
//         need_refresh = true;

//     ImGui::SetNextWindowSize(ImVec2(350, 500), true);
//     if (!ImGui::Begin("Package window", nullptr)) {
//         ImGui::End();
//         return;
//     }

//     ImGui::TextColored(
//       ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", gv.package.c_str());

//     ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_DefaultOpen;
//     if (ImGui::TreeNodeEx(gv.package.c_str(), node_flags)) {
//         for (const auto& n : hierarchy.children)
//             boost::apply_visitor(node_visitor(gv), n);
//         ImGui::TreePop();
//     }

//     if (need_refresh) {
//         local_package = gv.package;
//         hierarchy = refresh(gv);
//         need_refresh = false;
//     }

//     float width = ImGui::GetContentRegionAvailWidth() / 4.f -
//                   2.f * ImGui::GetStyle().FramePadding.x;

//     ImGui::Separator();

//     ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
//     ImGui::Text("Binary: %s", gv.pkg->getDir().c_str());
//     ImGui::Text("Source: %s",
//     gv.pkg->getDir(vle::utils::PKG_SOURCE).c_str()); ImGui::PopStyleVar();

//     if (ImGui::Button("configure", ImVec2(width, 0))) {
//     }
//     ImGui::SameLine();
//     if (ImGui::Button("build", ImVec2(width, 0))) {
//     }
//     ImGui::SameLine();
//     if (ImGui::Button("clean", ImVec2(width, 0))) {
//     }
//     ImGui::SameLine();
//     if (ImGui::Button("test", ImVec2(width, 0))) {
//     }

//     ImGui::End();
// }

} // namespace glvle
} // namespace vle
