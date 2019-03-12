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

#include <algorithm>
#include <fstream>
#include <sstream>

#include <vle/vpz/AtomicModel.hpp>
#include <vle/vpz/CoupledModel.hpp>

#include "glvle.hpp"

#include <cmath>

namespace vle {
namespace glvle {

void
ShowHelpMarker(const std::string& desc)
{
    ImGui::TextDisabled("[?]");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

#if 0
static int
get_slot_index(const std::vector<std::string>& vec,
               const std::string& str) noexcept
{
    auto found = std::find(vec.cbegin(), vec.cend(), str);

    assert(found != vec.cend());

    return static_cast<int>(std::distance(vec.cbegin(), found));
}

static Glgraph
convert_vpz(const vle::vpz::CoupledModel* mdl)
{
    Glnode node;
    std::vector<Glnode> nodes;
    std::vector<Gllink> links;
    std::vector<Glinputlink> input_links;
    std::vector<Gloutputlink> output_links;

    Glcache cache;

    node.name = mdl->getName();
    node.position =
      ImVec2(static_cast<float>(mdl->x()), static_cast<float>(mdl->y()));
    node.type = Glnode::model_type::coupled;

    std::transform(mdl->getInputPortList().cbegin(),
                   mdl->getInputPortList().cend(),
                   std::back_inserter(node.input_slots),
                   [](const auto& elem) { return elem.first; });

    std::transform(mdl->getOutputPortList().cbegin(),
                   mdl->getOutputPortList().cend(),
                   std::back_inserter(node.output_slots),
                   [](const auto& elem) { return elem.first; });

    for (const auto& child : mdl->getModelList()) {
        cache.emplace(child.second->getName(), static_cast<int>(cache.size()));

        nodes.emplace_back();

        nodes.back().name = child.second->getName();
        nodes.back().position = ImVec2(static_cast<float>(child.second->x()),
                                       static_cast<float>(child.second->y()));

        std::transform(child.second->getInputPortList().cbegin(),
                       child.second->getInputPortList().cend(),
                       std::back_inserter(nodes.back().input_slots),
                       [](const auto& elem) { return elem.first; });

        std::transform(child.second->getOutputPortList().cbegin(),
                       child.second->getOutputPortList().cend(),
                       std::back_inserter(nodes.back().output_slots),
                       [](const auto& elem) { return elem.first; });

        if (child.second->isAtomic()) {
            const auto* atom =
              static_cast<const vle::vpz::AtomicModel*>(child.second);
            nodes.back().conditions = atom->conditions();
            nodes.back().dynamics = atom->dynamics();
            nodes.back().observables = atom->observables();
            nodes.back().type = Glnode::model_type::atomic;
        } else {
            nodes.back().type = Glnode::model_type::coupled;
        }
    }

    cache.sort();

    for (const auto& it : mdl->getInternalOutputPortList()) {
        int output_slot = get_slot_index(node.output_slots, it.first);

        const auto& lst = it.second;
        for (const auto& jt : lst) {
            int input_id = cache.get(jt.first->getName());
            int input_slot =
              get_slot_index(nodes[input_id].output_slots, jt.second);

            output_links.emplace_back(input_id, input_slot, output_slot);
        }
    }

    for (const auto& it : mdl->getInternalInputPortList()) {
        int input_slot = get_slot_index(node.input_slots, it.first);
        const auto& lst = it.second;
        for (const auto& jt : lst) {
            int output_id = cache.get(jt.first->getName());
            int output_slot =
              get_slot_index(nodes[output_id].input_slots, jt.second);

            input_links.emplace_back(input_slot, output_id, output_slot);
        }
    }

    for (const auto& it : mdl->getModelList()) {
        const auto& cnts(it.second->getOutputPortList());
        for (const auto& cnt : cnts) {
            for (auto kt = cnt.second.begin(); kt != cnt.second.end(); ++kt) {
                if (kt->first != mdl) {
                    int input_id = cache.get(it.second->getName());
                    int input_slot =
                      get_slot_index(nodes[input_id].input_slots, cnt.first);
                    int output_id = cache.get(kt->first->getName());
                    int output_slot = get_slot_index(
                      nodes[output_id].output_slots, kt->second);

                    links.emplace_back(
                      input_id, input_slot, output_id, output_slot);
                }
            }
        }
    }

    return Glgraph(node, nodes, links, input_links, output_links);
}
#endif

std::string
get_new_name(vle::vpz::CoupledModel* mdl)
{
    int i = 0;

    do {
        std::string name = "model-" + std::to_string(i++);
        if (!mdl->exist(name))
            return name;
    } while (i < INT_MAX);

    return std::string("model");
}

void
Glvpz::show_grid()
{
    const ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
    const float GRID_SZ = 64.0f;
    const ImVec2 win_pos = ImGui::GetCursorScreenPos();
    const ImVec2 canvas_sz = ImGui::GetWindowSize();

    for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
        draw_list->AddLine(ImVec2(x, 0.0f) + win_pos,
                           ImVec2(x, canvas_sz.y) + win_pos,
                           GRID_COLOR);

    for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
        draw_list->AddLine(ImVec2(0.0f, y) + win_pos,
                           ImVec2(canvas_sz.x, y) + win_pos,
                           GRID_COLOR);
}

ImVec2
GetInputSlotPos(vle::vpz::BaseModel* mdl, const std::string& port)
{
    return ImVec2(
      mdl->x(),
      mdl->y() + mdl->height() *
                   (static_cast<float>(mdl->getInputPortIndex(port)) + 1.f) /
                   ((static_cast<float>(mdl->getInputPortNumber()) + 1.f)));
}

ImVec2
GetOutputSlotPos(vle::vpz::BaseModel* mdl, const std::string& port)
{
    return ImVec2(
      mdl->x() + mdl->width(),
      mdl->y() + mdl->width() *
                   (static_cast<float>(mdl->getOutputPortIndex(port)) + 1.f) /
                   (static_cast<float>(mdl->getOutputPortNumber()) + 1.f));
}

ImVec2
GetInputSlotPos(vle::vpz::BaseModel* mdl, const int port, const int size)
{
    return ImVec2(mdl->x(),
                  mdl->y() + mdl->height() * (static_cast<float>(port) + 1.f) /
                               ((static_cast<float>(size) + 1.f)));
}

ImVec2
GetOutputSlotPos(vle::vpz::BaseModel* mdl, const int port, const int size)
{
    return ImVec2(mdl->x() + mdl->width(),
                  mdl->y() + mdl->width() * (static_cast<float>(port) + 1.f) /
                               (static_cast<float>(size) + 1.f));
}

void
Glvpz::show_links()
{
    draw_list->ChannelsSplit(2);
    draw_list->ChannelsSetCurrent(0);

    for (const auto& it : top->getModelList()) {
        const auto& cnts(it.second->getOutputPortList());
        for (const auto& cnt : cnts) {
            ImVec2 p1 = offset + GetOutputSlotPos(it.second, cnt.first);

            for (auto kt = cnt.second.begin(); kt != cnt.second.end(); ++kt) {
                if (kt->first != top) {
                    ImVec2 p2 =
                      offset + GetInputSlotPos(kt->first, kt->second);

                    draw_list->AddBezierCurve(p1,
                                              p1 + ImVec2(+50, 0),
                                              p2 + ImVec2(-50, 0),
                                              p2,
                                              IM_COL32(200, 200, 100, 255),
                                              3.0f);
                }
            }
        }
    }

    // for (const auto& it : mdl->getInternalOutputPortList()) {
    //     int output_slot = get_slot_index(node.output_slots, it.first);

    //     const auto& lst = it.second;
    //     for (const auto& jt : lst) {
    //         int input_id = cache.get(jt.first->getName());
    //         int input_slot =
    //           get_slot_index(nodes[input_id].output_slots, jt.second);

    //         output_links.emplace_back(input_id, input_slot, output_slot);
    //     }
    // }

    // for (const auto& it : mdl->getInternalInputPortList()) {
    //     int input_slot = get_slot_index(node.input_slots, it.first);
    //     const auto& lst = it.second;
    //     for (const auto& jt : lst) {
    //         int output_id = cache.get(jt.first->getName());
    //         int output_slot =
    //           get_slot_index(nodes[output_id].input_slots, jt.second);

    //         input_links.emplace_back(input_slot, output_id, output_slot);
    //     }
    // }
}

void
Glvpz::show_models()
{
    const float NODE_SLOT_RADIUS = 4.0f;
    const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

    int id = 0;
    for (const auto& it : top->getModelList()) {
        ++id;
        ImGui::PushID(id);
        ImVec2 node_rect_min(offset.x + static_cast<float>(it.second->x()),
                             offset.y + static_cast<float>(it.second->y()));

        // Display node contents first
        draw_list->ChannelsSetCurrent(1); // Foreground
        bool old_any_active = ImGui::IsAnyItemActive();
        ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
        ImGui::BeginGroup(); // Lock horizontal position
        ImGui::Text("%s", it.first.c_str());

        if (it.second->isAtomic()) {
            auto* atom = static_cast<vle::vpz::AtomicModel*>(it.second);

            selected_condition.resize(static_cast<int>(vpz->project()
                                                         .experiment()
                                                         .conditions()
                                                         .conditionlist()
                                                         .size()),
                                      false);

            int i = 0;
            for (const auto& elem : vpz->project().experiment().conditions()) {
                const auto& name = elem.first;
                selected_condition[i] =
                  (std::find_if(atom->conditions().cbegin(),
                                atom->conditions().cend(),
                                [&name](const auto& elem) -> bool {
                                    return elem == name;
                                }) != atom->conditions().cend());
                ++i;
            }

            if (ImGui::BeginCombo("Dynamic", atom->dynamics().c_str(), 0)) {
                for (const auto& elem : vpz->project().dynamics()) {
                    bool is_selected = elem.first == atom->dynamics();

                    if (ImGui::Selectable(elem.first.c_str(), is_selected))
                        atom->setDynamics(elem.first);

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo(
                  "Observable", atom->observables().c_str(), 0)) {
                for (const auto& elem :
                     vpz->project().experiment().views().observables()) {
                    bool is_selected = elem.first == atom->observables();

                    if (ImGui::Selectable(elem.first.c_str(), is_selected))
                        atom->setObservables(elem.first);

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGuiComboFlags flags =
              ImGuiComboFlags_HeightSmall; // | ImGuiComboFlags_NoPreview;

            if (ImGui::BeginCombo("Conditions", "...", flags)) {
                int i = 0;
                for (const auto& elem :
                     vpz->project().experiment().conditions()) {

                    ImGui::Selectable(elem.first.c_str(),
                                      &selected_condition[i],
                                      ImGuiSelectableFlags_DontClosePopups);

                    ++i;
                }

                atom->conditions().clear();

                i = 0;
                for (const auto& elem :
                     vpz->project().experiment().conditions()) {
                    if (selected_condition[i])
                        atom->addCondition(elem.first);

                    ++i;
                }

                ImGui::EndCombo();
            }
        }

        // ImGui::SliderFloat("##value", &node->Value, 0.0f, 1.0f, "Alpha
        // %.2f"); ImGui::ColorEdit3("##color", &node->Color.x);
        ImGui::EndGroup();

        // Save the size of what we have emitted and whether any of the widgets
        // are being used
        bool node_widgets_active =
          (!old_any_active && ImGui::IsAnyItemActive());

        auto Size =
          ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
        it.second->setSize(Size.x, Size.y);
        ImVec2 node_rect_max = node_rect_min + Size;

        // Display node box
        draw_list->ChannelsSetCurrent(0); // Background
        ImGui::SetCursorScreenPos(node_rect_min);
        ImGui::InvisibleButton("node", Size);
        if (ImGui::IsItemHovered()) {
            node_hovered_in_scene = it.second;
            open_context_menu |= ImGui::IsMouseClicked(1);
        }
        bool node_moving_active = ImGui::IsItemActive();
        if (node_widgets_active || node_moving_active)
            node_selected = it.second;
        if (node_moving_active && ImGui::IsMouseDragging(0)) {
            auto delta = ImGui::GetIO().MouseDelta;
            it.second->setX(it.second->x() + delta.x);
            it.second->setY(it.second->y() + delta.y);
        }

        ImU32 node_bg_color =
          (node_hovered_in_list == it.second ||
           node_hovered_in_scene == it.second ||
           (!node_hovered_in_list && node_selected == it.second))
            ? IM_COL32(75, 75, 75, 255)
            : IM_COL32(60, 60, 60, 255);

        draw_list->AddRectFilled(
          node_rect_min, node_rect_max, node_bg_color, 4.0f);
        draw_list->AddRect(
          node_rect_min, node_rect_max, IM_COL32(100, 100, 100, 255), 4.0f);

        {
            const int size = static_cast<int>(it.second->getInputPortNumber());
            for (int slot_idx = 0; slot_idx < size; slot_idx++)
                draw_list->AddCircleFilled(
                  offset + GetInputSlotPos(it.second, slot_idx, size),
                  NODE_SLOT_RADIUS,
                  IM_COL32(150, 150, 150, 150));
        }

        {
            const int size =
              static_cast<int>(it.second->getOutputPortNumber());
            for (int slot_idx = 0; slot_idx < size; slot_idx++)
                draw_list->AddCircleFilled(
                  offset + GetOutputSlotPos(it.second, slot_idx, size),
                  NODE_SLOT_RADIUS,
                  IM_COL32(150, 150, 150, 150));
        }

        ImGui::PopID();
    }
}

static vle::vpz::CoupledModel*
show_vpz(vle::vpz::CoupledModel* mdl)
{
    vle::vpz::CoupledModel* ret = nullptr;

    auto& list = mdl->getModelList();

    auto it = std::find_if(list.begin(), list.end(), [](const auto& child) {
        if (child.second->isCoupled())
            return true;

        return false;
    });

    if (it == list.end()) {
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                        ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                        ImGuiTreeNodeFlags_Leaf |
                                        ImGuiTreeNodeFlags_NoTreePushOnOpen;

        if (ImGui::TreeNodeEx(mdl->getName().c_str(), node_flags))
            ret = mdl;
    } else {
        if (ImGui::TreeNode(mdl->getName().c_str())) {

            for (auto& elem : list) {
                if (elem.second->isCoupled()) {
                    auto* tmp = show_vpz(
                      static_cast<vle::vpz::CoupledModel*>(elem.second));
                    if (tmp)
                        ret = tmp;
                }
            }
            ImGui::TreePop();
        }
    }

    return ret;
}

bool
Glvpz::show()
{
    bool ret = true;

    ImGui::SetNextWindowSize(ImVec2(600, 600), true);
    if (!ImGui::Begin(id.c_str(), &ret)) {
        ImGui::End();
        return ret;
    }

    switch (st) {
    case vle::glvle::Glvpz::status::uninitialized:
        ImGui::Text("uninitialized.");
        ImGui::End();
        return true;
    case vle::glvle::Glvpz::status::loading:
        ImGui::Text("loading.");
        ImGui::End();
        return true;
    case vle::glvle::Glvpz::status::read_error:
        printf("Error reading file\n");
        ImGui::End();
        return false;
    case vle::glvle::Glvpz::status::success:
        show_left();
        show_center();
        break;
    }

    ImGui::End();
    return ret;
}

void
Glvpz::show_left()
{
    ImGui::BeginChild(file.c_str(), ImVec2(250, 0), true);

    if (ImGui::TreeNode("Hierarchy")) {
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_DefaultOpen;
        top =
          static_cast<vle::vpz::CoupledModel*>(vpz->project().model().node());

        if (ImGui::TreeNodeEx(top->getName().c_str(), node_flags)) {
            for (auto& child :
                 static_cast<vle::vpz::CoupledModel*>(top)->getModelList())
                if (child.second->isCoupled()) {
                    auto* tmp = show_vpz(
                      static_cast<vle::vpz::CoupledModel*>(child.second));

                    if (tmp && tmp != top)
                        top = tmp;
                }

            ImGui::TreePop();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Conditions")) {
        static auto selected = -1;
        const auto& map =
          vpz->project().experiment().conditions().conditionlist();
        auto i = 0;

        for (auto& elem : map)
            if (ImGui::Selectable(elem.second.name().c_str(), selected == i))
                selected = i;

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Dynamics")) {
        ImGui::Columns(3, "dynamics-columns");
        ImGui::Separator();
        ImGui::Text("ID");
        ImGui::NextColumn();
        ImGui::Text("Library");
        ImGui::NextColumn();
        ImGui::Text("Package");
        ImGui::NextColumn();
        ImGui::Separator();

        static auto selected = -1;
        const auto& map = vpz->project().dynamics().dynamiclist();
        auto i = 0;

        for (auto& elem : map) {
            if (ImGui::Selectable(elem.second.name().c_str(),
                                  selected == i,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                selected = i;
            }

            ImGui::NextColumn();
            ImGui::Text("%s", elem.second.library().c_str());
            ImGui::NextColumn();
            ImGui::Text("%s", elem.second.package().c_str());
            ImGui::NextColumn();
        }

        ImGui::Columns(1);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Views")) {
        ImGui::Columns(2, "views-columns");
        ImGui::Separator();
        ImGui::Text("ID");
        ImGui::NextColumn();
        ImGui::Text("Enable");
        ImGui::NextColumn();
        ImGui::Separator();

        auto& map = vpz->project().experiment().views().viewlist();
        for (auto& elem : map) {
            ImGui::Text("%s", elem.first.c_str());
            ImGui::NextColumn();
            bool active = elem.second.is_enable();
            ImGui::Checkbox("##value", &active);

            if (active)
                elem.second.enable();
            else
                elem.second.disable();

            ImGui::NextColumn();
        }

        ImGui::Columns(1);
        ImGui::TreePop();
    }

    ImGui::EndChild();
}

void
Glvpz::show_center()
{
    if (!top)
        return;

    ImGui::SameLine();
    ImGui::BeginGroup();

    draw_list = ImGui::GetWindowDrawList();
    // ImGuiIO& io = ImGui::GetIO();

    pos = ImGui::GetWindowPos();
    size = ImGui::GetWindowSize();
    mouse = ImGui::GetMousePos();

    ImRect canvas(pos, pos + size);

    // if (!ImGui::IsMouseDown(0) && canvas.Contains(mouse)) {
    //     if (ImGui::IsMouseDragging(1))
    //         scroll += io.MouseDelta;

    //     if (io.KeyShift && !io.KeyCtrl)
    //         scroll.x += io.MouseWheel * 16.0f;

    //     if (!io.KeyShift && !io.KeyCtrl)
    //         scroll.y += io.MouseWheel * 16.0f;

    //     if (!io.KeyShift && io.KeyCtrl) {
    //         ImVec2 focus = (mouse - scroll - pos) / scale;

    //         if (io.MouseWheel < 0.0f)
    //             for (float zoom = io.MouseWheel; zoom < 0.0f; zoom += 1.0f)
    //                 scale = ImMax(0.3f, scale / 1.05f);

    //         if (io.MouseWheel > 0.0f)
    //             for (float zoom = io.MouseWheel; zoom > 0.0f; zoom -= 1.0f)
    //                 scale = ImMin(3.0f, scale * 1.05f);

    //         ImVec2 shift = scroll + (focus * scale);
    //         scroll += mouse - shift - pos;
    //     }

    //     if (ImGui::IsMouseReleased(1))
    //         if (io.MouseDragMaxDistanceSqr[1] <
    //             (io.MouseDragThreshold * io.MouseDragThreshold))
    //             ImGui::OpenPopup("NodesContextMenu");
    // }

    ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)",
                static_cast<double>(scrolling.x),
                static_cast<double>(scrolling.y));

    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    ImGui::Checkbox("Show grid", &with_grid);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(60, 60, 70, 200));
    ImGui::BeginChild("scrolling_region",
                      ImVec2(0, 0),
                      true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::PushItemWidth(120.0f);

    // draw_list = ImGui::GetWindowDrawList();
    offset = ImGui::GetCursorScreenPos() + scrolling;

    if (with_grid)
        show_grid();

    show_links();
    show_models();

    draw_list->ChannelsMerge();

    if (!ImGui::IsAnyItemHovered() &&
        ImGui::IsWindowHovered(
          ImGuiHoveredFlags_AllowWhenBlockedByPopup |
          ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
        ImGui::IsMouseClicked(1)) {
        node_selected = node_hovered_in_list = node_hovered_in_scene = nullptr;
        open_context_menu = true;
    }

    if (open_context_menu) {
        ImGui::OpenPopup("context_menu");
        if (node_hovered_in_list)
            node_selected = node_hovered_in_list;
        if (node_hovered_in_scene)
            node_selected = node_hovered_in_scene;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("context_menu")) {
        vle::vpz::BaseModel* node = node_selected;
        ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
        if (node) {
            ImGui::Text("Node '%s'", node->getName().c_str());
            ImGui::Separator();
            if (ImGui::MenuItem("Rename..", NULL, false, false)) {
            }
            if (ImGui::MenuItem("Delete", NULL, false, false)) {
            }
            if (ImGui::MenuItem("Copy", NULL, false, false)) {
            }
        } else {
            if (ImGui::MenuItem("Add atomic mode")) {
                auto* newmdl = top->addAtomicModel(get_new_name(top).c_str());
                newmdl->setX(scene_pos.x);
                newmdl->setY(scene_pos.y);
            }
            if (ImGui::MenuItem("Add coupled model")) {
                auto* newmdl = top->addCoupledModel(get_new_name(top).c_str());
                newmdl->setX(scene_pos.x);
                newmdl->setY(scene_pos.y);
            }
            node_selected = nullptr;
            node_hovered_in_list = nullptr;
            node_hovered_in_scene = nullptr;
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();

    // Scrolling
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() &&
        ImGui::IsMouseDragging(2, 0.0f))
        scrolling = scrolling + ImGui::GetIO().MouseDelta;

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    ImGui::EndGroup();
}

bool
Glvpz::open(const std::string& file_)
{
    st = vle::glvle::Glvpz::status::loading;
    try {
        vpz = std::make_shared<vle::vpz::Vpz>(file_);
        vpz->parseFile(file_);
        st = vle::glvle::Glvpz::status::success;
        file = file_;
        return true;
    } catch (const std::exception& e) {
        st = vle::glvle::Glvpz::status::read_error;
        return false;
    }
}

static inline ImVec2
operator+(const ImVec2& lhs, const ImVec2& rhs)
{
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}
static inline ImVec2
operator-(const ImVec2& lhs, const ImVec2& rhs)
{
    return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

} // namespace glvle
} // namespace vle
