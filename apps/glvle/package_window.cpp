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

#include <sstream>
#include <stack>
#include <thread>

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

    if (pst == package_status::none) {
        if (ImGui::Button("configure", ImVec2(width, 0))) {
            pst = package_status::configure;
            gv.log_w.log(
              6, "configure package %s\n", current.path.string().c_str());

            try {
                future_package_return =
                  std::async(std::launch::async,
                             [](vle::utils::Package& p,
                                std::ostream& out,
                                std::ostream& err) -> bool {
                                 p.configure();
                                 p.wait(out, err);
                                 return p.isSuccess();
                             },
                             std::ref(*package),
                             std::ref(log_package_out),
                             std::ref(log_package_err));

            } catch (const std::exception& /*e*/) {
                pst = package_status::none;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("build", ImVec2(width, 0))) {
            pst = package_status::build;

            gv.log_w.log(
              6, "build package %s\n", current.path.string().c_str());
            try {
                future_package_return =
                  std::async(std::launch::async,
                             [](vle::utils::Package& p,
                                std::ostream& out,
                                std::ostream& err) -> bool {
                                 p.build();
                                 p.wait(out, err);
                                 if (p.isSuccess()) {
                                     p.install();
                                     p.wait(out, err);
                                     return p.isSuccess();
                                 }

                                 return false;
                             },
                             std::ref(*package),
                             std::ref(log_package_out),
                             std::ref(log_package_err));

            } catch (const std::exception& /*e*/) {
                pst = package_status::none;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("clean", ImVec2(width, 0))) {
            pst = package_status::clean;
            gv.log_w.log(
              6, "clean package %s\n", current.path.string().c_str());
            try {
                future_package_return =
                  std::async(std::launch::async,
                             [](vle::utils::Package& p) -> bool {
                                 p.clean();
                                 p.rclean();
                                 return true;
                             },
                             std::ref(*package));

            } catch (const std::exception& /*e*/) {
                pst = package_status::none;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("test", ImVec2(width, 0))) {
            pst = package_status::test;
            gv.log_w.log(
              6, "test package %s\n", current.path.string().c_str());
            try {
                future_package_return =
                  std::async(std::launch::async,
                             [](vle::utils::Package& p,
                                std::ostream& out,
                                std::ostream& err) -> bool {
                                 p.test();
                                 p.wait(out, err);
                                 return p.isSuccess();
                             },
                             std::ref(*package),
                             std::ref(log_package_out),
                             std::ref(log_package_err));

            } catch (const std::exception& /*e*/) {
                pst = package_status::none;
            }
        }
    } else {
        static const char* str_status[] = { "Nothing to do"
                                            "Doing configure...",
                                            "Doing build...",
                                            "Doing clean...",
                                            "Doing test..." };

        if (future_package_return.valid()) {
            {
                std::lock_guard<std::mutex> guard(log_package_return);
                gv.log_w.log(6, "%s", log_package_out.str().c_str());
                log_package_out.str(std::string());
                gv.log_w.log(5, "%s", log_package_err.str().c_str());
                log_package_err.str(std::string());
            }

            auto s = future_package_return.wait_for(std::chrono::seconds(0));
            if (s == std::future_status::ready)
                pst = package_status::none;
            else
                ImGui::Text("%s", str_status[static_cast<int>(pst)]);
        } else
            ImGui::Text("%s", str_status[static_cast<int>(pst)]);
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

} // namespace glvle
} // namespace vle
