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

#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "glvle.hpp"

#include <vle/utils/Filesystem.hpp>

#include <algorithm>
#include <vector>

namespace vle {
namespace glvle {

static vle::utils::Path
get_default_directory(const std::string path)
{
    vle::utils::Path ret(path);

    return ret.empty() || !ret.exists() ? vle::utils::Path::current_path()
                                        : ret;
}

static std::vector<vle::utils::Path>
get_all_entities(const vle::utils::Path& path, const char* filter)
{
    std::vector<vle::utils::Path> ret;

    std::copy_if(vle::utils::DirectoryIterator(path),
                 vle::utils::DirectoryIterator(),
                 std::back_inserter(ret),
                 [](const auto& p) {
                     auto filename = p.filename();
                     return !(filename.empty() || filename[0] == '.');
                 });

    if (filter) {
        ret.erase(std::remove_if(ret.begin(),
                                 ret.end(),
                                 [&filter](const auto& p) {
                                     if (p.is_file() &&
                                         p.extension() == filter)
                                         return true;

                                     return false;
                                 }),
                  ret.end());
    }

    std::sort(ret.begin(), ret.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.is_directory()) {
            if (rhs.is_directory()) {
                return lhs.filename() < rhs.filename();
            } else {
                return true;
            }
        } else {
            if (rhs.is_directory()) {
                return false;
            } else {
                return lhs.filename() < rhs.filename();
            }
        }
    });

    return ret;
}

static std::vector<vle::utils::Path>
get_directory_entities(const vle::utils::Path& path)
{
    std::vector<vle::utils::Path> ret;

    std::copy_if(vle::utils::DirectoryIterator(path),
                 vle::utils::DirectoryIterator(),
                 std::back_inserter(ret),
                 [](const auto& p) {
                     if (p.is_directory()) {
                         auto filename = p.filename();
                         return !(filename.empty() || filename[0] == '.');
                     }

                     return false;
                 });

    std::sort(ret.begin(), ret.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.filename() < rhs.filename();
    });

    return ret;
}

#define MAX_FILE_DIALOG_NAME_BUFFER 1024

struct file_dialog
{
    static std::string selected;
    static vle::utils::Path current;
    static std::vector<vle::utils::Path> content;
    static char input[MAX_FILE_DIALOG_NAME_BUFFER];
    static int filter;

    static void clear()
    {
        selected.clear();
        current.clear();
        content.clear();
        filter = -1;
    }

    bool select_file(const char* name,
                     const char* description,
                     const char* filters,
                     const char* path,
                     std::string& out)
    {
        if (current.empty()) {
            selected.clear();
            current = get_default_directory(path);
            content = get_all_entities(current, nullptr);
            filter = -1;
        }

        auto next = vle::utils::Path{};
        bool res = false;

        if (ImGui::BeginPopupModal(name, nullptr)) {
            bool path_click = false;
            for (auto it = current.begin(), et = current.end(); it != et;
                 ++it) {
                if (it != current.begin())
                    ImGui::SameLine();

                if (ImGui::Button(it->c_str())) {
                    next = current.pop(std::distance(current.begin(), it));
                    selected.clear();
                    path_click = true;
                    break;
                }
            }

            // ImVec2 size = ImGui::GetContentRegionMax() - ImVec2(0.0f,
            // 120.0f); ImGui::BeginChild("##FileDialog_FileList", size);
            ImGui::BeginChild("##FALL");

            if (ImGui::Selectable("..##FALL", (selected == ".."))) {
                if (next.empty()) {
                    next = current.parent_path();
                    selected.clear();
                    path_click = true;
                }
            }

            std::string str;
            for (auto it = content.begin(), et = content.end(); it != et;
                 ++it) {
                if (it->is_directory())
                    str = "[Dir] " + it->filename();
                else
                    str = "[File] " + it->filename();

                if (ImGui::Selectable(str.c_str(),
                                      (it->filename() == selected))) {

                    selected = it->filename();

                    if (it->is_directory()) {
                        if (next.empty()) {
                            selected.clear();
                            next = current;
                            next /= it->filename();
                            path_click = true;
                        }
                    }

                    break;
                }
            }

            ImGui::EndChild();

            if (path_click) {
                content = get_all_entities(next, filters);
                current = next;
            }

            ImGui::Text("File Name:##FALL");
            ImGui::SameLine();

            // float width = ImGui::GetContentRegionAvailWidth();
            // if (filters)
            //     width -= 120.0f;

            // ImGui::PushItemWidth(width);
            // ImGui::InputText(
            //   "##FileName", selected.c_str(), MAX_FILE_DIALOG_NAME_BUFFER);
            // ImGui::PopItemWidth();
            // static int filter;

            // if (!filters.empty()) {
            //     ImGui::SameLine();

            //     ImGui::PushItemWidth(100.0f);
            //     bool comboClick =
            //       ImGui::Combo("##Filters", &FilterIndex, vFilters) ||
            //       m_CurrentFilterExt.size() == 0;
            //     ImGui::PopItemWidth();
            //     if (comboClick == true) {
            //         int itemIdx = 0;
            //         const char* p = vFilters;
            //         while (*p) {
            //             if (FilterIndex == itemIdx) {
            //                 m_CurrentFilterExt = std::string(p);
            //                 break;
            //             }
            //             p += strlen(p) + 1;
            //             itemIdx++;
            //         }
            //     }
            // }

            if (ImGui::Button("Cancel##FALL", ImVec2(120, 0))) {
                vle::utils::Path sel = current;
                sel /= selected;
                out = sel.string();
                ImGui::CloseCurrentPopup();
                res = true;
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();

            if (ImGui::Button("Ok##FALL", ImVec2(180, 0))) {
                ImGui::CloseCurrentPopup();
                res = true;
            }

            ImGui::EndPopup();
        }

        return res;
    }

    bool select_directory(const char* name,
                          const char* description,
                          const char* path,
                          std::string& out)
    {
        if (current.empty()) {
            selected.clear();
            current = get_default_directory(path);
            content = get_directory_entities(current);
            filter = -1;
        }

        auto next = vle::utils::Path{};
        bool res = false;

        if (ImGui::BeginPopupModal(name, nullptr)) {
            bool path_click = false;
            for (auto it = current.begin(), et = current.end(); it != et;
                 ++it) {
                if (it != current.begin())
                    ImGui::SameLine();

                if (ImGui::Button(it->c_str())) {
                    next = current.pop(std::distance(current.begin(), it));
                    selected.clear();
                    path_click = true;
                    break;
                }
            }

            // ImVec2 size = ImGui::GetContentRegionMax() - ImVec2(0.0f,
            // 120.0f); ImGui::BeginChild("##FileDialog_FileList", size);
            ImGui::BeginChild("##FDir");

            if (ImGui::Selectable("..", (selected == ".."))) {
                if (next.empty()) {
                    next = current.parent_path();
                    selected.clear();
                    path_click = true;
                }
            }

            std::string str;
            for (auto it = content.begin(), et = content.end(); it != et;
                 ++it) {
                str = "[Dir] " + it->filename();

                if (ImGui::Selectable(str.c_str(),
                                      (it->filename() == selected))) {

                    selected = it->filename();

                    if (next.empty()) {
                        selected.clear();
                        next = current;
                        next /= it->filename();
                        path_click = true;
                    }

                    break;
                }
            }

            ImGui::EndChild();

            if (path_click) {
                content = get_directory_entities(next);
                current = next;
            }

            ImGui::Text("File Name:##FDir");
            ImGui::SameLine();

            // float width = ImGui::GetContentRegionAvailWidth();
            // if (filters)
            //     width -= 120.0f;

            // ImGui::PushItemWidth(width);
            // ImGui::InputText(
            //   "##FileName", selected.c_str(), MAX_FILE_DIALOG_NAME_BUFFER);
            // ImGui::file_dialog::PopItemWidth();

            // if (!filters.empty()) {
            //     ImGui::SameLine();

            //     ImGui::PushItemWidth(100.0f);
            //     bool comboClick =
            //       ImGui::Combo("##Filters", &FilterIndex, vFilters) ||
            //       m_CurrentFilterExt.size() == 0;
            //     ImGui::PopItemWidth();
            //     if (comboClick == true) {
            //         int itemIdx = 0;
            //         const char* p = vFilters;
            //         while (*p) {
            //             if (FilterIndex == itemIdx) {
            //                 m_CurrentFilterExt = std::string(p);
            //                 break;
            //             }
            //             p += strlen(p) + 1;
            //             itemIdx++;
            //         }
            //     }
            // }

            if (ImGui::Button("Cancel##FDir", ImVec2(120, 0))) {
                vle::utils::Path sel = current;
                sel /= selected;
                out = sel.string();
                ImGui::CloseCurrentPopup();
                res = true;
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();

            if (ImGui::Button("Ok##FDif", ImVec2(180, 0))) {
                ImGui::CloseCurrentPopup();
                res = true;
            }

            ImGui::EndPopup();
        }

        return res;
    }

    bool select_new_directory(const char* name,
                              const char* description,
                              const char* path,
                              std::string& pathname,
                              std::string& dirname)
    {
        if (current.empty()) {
            current = get_default_directory(path);
            content = get_directory_entities(current);
            filter = -1;
        }

        auto next = vle::utils::Path{};
        bool res = false;

        ImGui::SetNextWindowSize(ImVec2(550, 680), true);
        if (ImGui::BeginPopupModal(name, nullptr)) {
            bool path_click = false;
            for (auto it = current.begin(), et = current.end(); it != et;
                 ++it) {
                if (it != current.begin())
                    ImGui::SameLine();

                if (ImGui::Button(it->c_str())) {
                    next = current.pop(std::distance(current.begin(), it));
                    selected.clear();
                    path_click = true;
                    break;
                }
            }

            ImVec2 size = ImGui::GetContentRegionMax();
            size.y -= 120;
            ImGui::BeginChild("##FDir", size);

            if (ImGui::Selectable("..", false)) {
                if (next.empty()) {
                    next = current.parent_path();
                    selected.clear();
                    path_click = true;
                }
            }

            if (ImGui::Selectable(".", false)) {
                if (next.empty()) {
                    next = current;
                    selected.clear();
                    path_click = true;
                }
            }

            std::string str;
            for (auto it = content.begin(), et = content.end(); it != et;
                 ++it) {
                str = "[Dir] " + it->filename();

                if (ImGui::Selectable(str.c_str(),
                                      (it->filename() == selected))) {
                    selected = it->filename();

                    if (next.empty()) {
                        selected.clear();
                        next = current;
                        next /= it->filename();
                        path_click = true;
                    }

                    break;
                }
            }

            ImGui::EndChild();

            if (path_click) {
                content = get_directory_entities(next);
                current = next;
            }

            ImGui::InputText("Package", input, MAX_FILE_DIALOG_NAME_BUFFER);

            bool show_ok_button = false;
            float width = ImGui::GetContentRegionAvailWidth();
            if (strlen(input) > 0) {
                auto new_directory = current;

                if (!selected.empty())
                    new_directory /= selected;

                new_directory /= input;

                if (!new_directory.exists()) {
                    show_ok_button = true;
                    width /= 2.0f;

                    if (ImGui::Button("Ok", ImVec2(width, 0))) {
                        pathname = new_directory.parent_path().string();
                        dirname = new_directory.filename();

                        ImGui::CloseCurrentPopup();
                        res = true;
                    }
                }
            }

            ImGui::SetItemDefaultFocus();

            if (show_ok_button)
                ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(width, 0))) {
                pathname.clear();
                dirname.clear();
                ImGui::CloseCurrentPopup();
                res = true;
            }

            ImGui::EndPopup();
        }

        return res;
    }
};

std::string file_dialog::selected;
vle::utils::Path file_dialog::current;
std::vector<vle::utils::Path> file_dialog::content;
char file_dialog::input[MAX_FILE_DIALOG_NAME_BUFFER];
int file_dialog::filter;

bool
select_file_dialog(const char* name,
                   const char* description,
                   const char* filters,
                   const char* path,
                   std::string& out)
{
    file_dialog box;

    return box.select_file(name, description, filters, path, out);
}

bool
select_new_directory_dialog(const char* name,
                            const char* description,
                            const char* path,
                            std::string& pathname,
                            std::string& dirname)
{
    file_dialog box;

    return box.select_new_directory(
      name, description, path, pathname, dirname);
}

bool
select_directory_dialog(const char* name,
                        const char* description,
                        const char* path,
                        std::string& out)
{
    file_dialog box;

    return box.select_directory(name, description, path, out);
}
}
}
