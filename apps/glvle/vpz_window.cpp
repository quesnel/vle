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

#include <vle/vpz/CoupledModel.hpp>

#include "glvle.hpp"

#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cmath>

namespace vle {
namespace glvle {

static void
show_vpz(const vle::vpz::CoupledModel* mdl)
{
    const auto& list = mdl->getModelList();

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

        ImGui::TreeNodeEx(mdl->getName().c_str(), node_flags);
    } else {
        if (ImGui::TreeNode(mdl->getName().c_str())) {
            std::for_each(list.begin(), list.end(), [](const auto& mdl) {
                if (mdl.second->isCoupled())
                    show_vpz(static_cast<vle::vpz::CoupledModel*>(mdl.second));
            });
            ImGui::TreePop();
        }
    }
}

struct Glnode
{
    enum class model_type
    {
        atomic,
        coupled
    };

    std::string name;
    std::string dynamics;
    std::string observables;
    std::vector<std::string> conditions;
    std::vector<std::string> input_slots;
    std::vector<std::string> output_slots;
    model_type type;

    ImVec2 position;
    ImVec2 size;

    Glnode()
      : type(model_type::atomic)
    {}

    void update_size() noexcept
    {
        size = ImVec2(size.x,
                      size.y * static_cast<float>(std::max(
                                 input_slots.size(), output_slots.size())));
    }

    ImVec2 get_input_slot_pos(const int slot_no) noexcept
    {
        return ImVec2(position.x,
                      position.y + size.y * static_cast<float>(slot_no + 1) /
                                     static_cast<float>(input_slots.size()));
    }

    ImVec2 get_output_slot_pos(const int slot_no) noexcept
    {
        return ImVec2(position.x + size.x,
                      position.y + size.y * static_cast<float>(slot_no + 1) /
                                     static_cast<float>(output_slots.size()));
    }
};

int
get_slot_index(const std::vector<std::string>& vec,
               const std::string& str) noexcept
{
    auto found = std::find(vec.cbegin(), vec.cend(), str);

    assert(found != vec.cend());

    return static_cast<int>(std::distance(vec.cbegin(), found));
}

struct Glinputlink
{
    Glinputlink(int input_slot_, int output_id_, int output_slot_)
      : input_slot(input_slot_)
      , output_id(output_id_)
      , output_slot(output_slot_)
    {}

    int input_slot;
    int output_id;
    int output_slot;
};

struct Gloutputlink
{
    Gloutputlink(int input_id_, int input_slot_, int output_slot_)
      : input_id(input_id_)
      , input_slot(input_slot_)
      , output_slot(output_slot_)
    {}

    int input_id;
    int input_slot;
    int output_slot;
};

struct Gllink
{
    Gllink(int input_id_, int input_slot_, int output_id_, int output_slot_)
      : input_id(input_id_)
      , input_slot(input_slot_)
      , output_id(output_id_)
      , output_slot(output_slot_)
    {}

    int input_id;
    int input_slot;
    int output_id;
    int output_slot;
};

struct Glgraph
{
    Glgraph() = default;

    Glgraph(Glnode node_,
            std::vector<Glnode> nodes_,
            std::vector<Gllink> links_,
            std::vector<Glinputlink> input_links_,
            std::vector<Gloutputlink> output_links_)
      : node(node_)
      , nodes(nodes_)
      , links(links_)
      , input_links(input_links_)
      , output_links(output_links_)
    {}

    Glnode node;
    std::vector<Glnode> nodes;
    std::vector<Gllink> links;
    std::vector<Glinputlink> input_links;
    std::vector<Gloutputlink> output_links;
};

class Glcache
{
public:
    struct element
    {
        element(const std::string& name_, const int position_)
          : name(name_)
          , position(position_)
        {}

        std::string name;
        int position;
    };

private:
    std::vector<element> elements;

public:
    Glcache()
    {
        elements.reserve(size_t{ 32 });
    }

    template<class... Args>
    void emplace(Args&&... args)
    {
        elements.emplace_back(std::forward<Args>(args)...);
    }

    size_t size() const noexcept
    {
        return elements.size();
    }

    void sort() noexcept
    {
        std::sort(
          elements.begin(),
          elements.end(),
          [](const auto& lhs, const auto& rhs) { return lhs.name < rhs.name; });
    }

    int get(const std::string& str) const noexcept
    {
        struct comparator
        {
            bool operator()(const element& lhs, const std::string& str) const
              noexcept
            {
                return lhs.name < str;
            }

            bool operator()(const std::string& str, const element& rhs) const
              noexcept
            {
                return str < rhs.name;
            }
        };

        auto found = std::equal_range(
          elements.cbegin(), elements.cend(), str, comparator{});

        assert(std::distance(found.first, found.second) == 1);

        return found.first->position;
    }
};

Glgraph
convert_vpz(const vle::vpz::CoupledModel* mdl)
{
    Glnode node;
    std::vector<Glnode> nodes;
    std::vector<Gllink> links;
    std::vector<Glinputlink> input_links;
    std::vector<Gloutputlink> output_links;

    Glcache cache;

    // Convert the coupled model.

    node.name = mdl->getName();
    node.position = ImVec2(mdl->x(), mdl->y());
    node.type = Glnode::model_type::coupled;

    std::transform(mdl->getInputPortList().cbegin(),
                   mdl->getInputPortList().cend(),
                   std::back_inserter(node.input_slots),
                   [](const auto& elem) { return elem.first; });

    std::transform(mdl->getOutputPortList().cbegin(),
                   mdl->getOutputPortList().cend(),
                   std::back_inserter(node.output_slots),
                   [](const auto& elem) { return elem.first; });

    // Convert the internal models.
    // std::sort<std::string, int> name;

    for (const auto& child : mdl->getModelList()) {
        cache.emplace(child.second->getName(), static_cast<int>(cache.size()));

        nodes.emplace_back();

        nodes.back().name = child.second->getName();
        nodes.back().position = ImVec2(child.second->x(), child.second->y());

        std::transform(child.second->getInputPortList().cbegin(),
                       child.second->getInputPortList().cend(),
                       std::back_inserter(nodes.back().input_slots),
                       [](const auto& elem) { return elem.first; });

        std::transform(child.second->getOutputPortList().cbegin(),
                       child.second->getOutputPortList().cend(),
                       std::back_inserter(nodes.back().output_slots),
                       [](const auto& elem) { return elem.first; });

        if (child.second->isAtomic()) {
            nodes.back().conditions =
              static_cast<vle::vpz::AtomicModel* const>(child.second)
                ->conditions();
            nodes.back().dynamiacs =
              static_cast<vle::vpz::AtomicModel* const>(child.second)
                ->dynamics();
            nodes.back().observables =
              static_cast<vle::vpz::AtomicModel* const>(child.second)
                ->observables();
            nodes.back().type = Glnode::model_type::atomic;
        } else {
            nodes.back().type = Glnode::model_type::coupled;
        }
    }

    cache.sort();

    // Convert internal connections

    for (const auto& it : mdl->getInternalOutputList()) {
        int output_slot = get_slot_index(node.output_slots);

        const std::string& port(it.first);
        const ModelPortList& lst(it.second);
        for (const auto& jt : lst) {
            int input_id = cache.get(jf.first->getName());
            int input_slot =
              get_slot_index(nodes[input_id].output_slots, jt.second);

            ouput_links.emplace_back(input_id, input_slot, output_slot);
        }
    }

    for (const auto& it : mdl->getInternalInputList()) {
        int input_slot = get_slot_index(node.input_slots);

        const std::string& port(it.first);
        const ModelPortList& lst(it.second);
        for (const auto& jt : lst) {
            int output_id = cache.get(jt.first->getName());
            int output_slot =
              get_slot_index(nodes[output_id].input_slot, jt.second);

            input_links.emplace_back(input_slot, output_id, output_slot);
        }
    }

    for (const auto& it : mdl->getModelList()) {
        const ConnectionList& cnts(it.second->getOutputPortList());
        for (const auto& cnt : cnts) {
            for (auto kt = cnt.second.begin(); kt != cnt.second.end(); ++kt) {
                if (kt->first != this) {
                    int input_id = cache.get(it.second->getName());
                    int input_slot =
                      get_slot_index(nodes[input_id].input_slots, cnt.first);
                    int output_id = cache.get(kt->first->getName());
                    int output_slot =
                      get_slot_index(nodes[output_id].output_slots, kt->second);

                    links.emplace_back(
                      input_id, input_slot, output_id, output_slot);
                }
            }
        }
    }

    return Glgraph(node, nodes, links, input_links, output_links);
}

static void
show_vpz(Glvle& /*gl*/, const std::string& /*file*/, Glvpz& vpz)
{
    ImGui::BeginChild("node_list", ImVec2(100, 0));
    ImGui::Text("Hierarchy");
    ImGui::Separator();

    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_DefaultOpen;
    const auto* root = vpz.vpz->project().model().node();

    if (ImGui::TreeNodeEx(root->getName().c_str(), node_flags)) {
        for (const auto& child :
             static_cast<const vle::vpz::CoupledModel*>(root)->getModelList())
            if (child.second->isCoupled())
                show_vpz(
                  static_cast<const vle::vpz::CoupledModel*>(child.second));

        ImGui::TreePop();
    }

    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginGroup();

    static ImVec2 scrolling = ImVec2(0.0f, 0.0f);

    const float NODE_SLOT_RADIUS = 4.0f;
    const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);
    static bool show_grid = true;

    // Create our child canvas
    ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)",
                double{ scrolling.x },
                double{ scrolling.y });
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    ImGui::Checkbox("Show grid", &show_grid);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, IM_COL32(60, 60, 70, 255));
    ImGui::BeginChild("scrolling_region",
                      ImVec2(0, 0),
                      true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::PushItemWidth(120.0f);

    ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // Display grid
    if (show_grid) {
        ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
        float GRID_SZ = 64.0f;
        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetWindowSize();
        for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x;
             x += GRID_SZ)
            draw_list->AddLine(ImVec2(x, 0.0f) + win_pos,
                               ImVec2(x, canvas_sz.y) + win_pos,
                               GRID_COLOR);
        for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y;
             y += GRID_SZ)
            draw_list->AddLine(ImVec2(0.0f, y) + win_pos,
                               ImVec2(canvas_sz.x, y) + win_pos,
                               GRID_COLOR);
    }

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();
}

bool
vpz_window(Glvle& gl, const std::string& file, Glvpz& vpz)
{
    bool ret = true;

    ImGui::SetNextWindowSize(ImVec2(600, 600), true);
    if (!ImGui::Begin(file.c_str(), &ret)) {
        ImGui::End();
        return ret;
    }

    if (vpz.st == vle::glvle::Glvpz::status::uninitialized) {
        try {
            vpz.vpz = std::make_shared<vle::vpz::Vpz>(file);
            vpz.vpz->parseFile(file);
            vpz.st = vle::glvle::Glvpz::status::success;
        } catch (const std::exception& e) {
            vpz.st = vle::glvle::Glvpz::status::read_error;
        }
    }

    switch (vpz.st) {
    case vle::glvle::Glvpz::status::success:
        show_vpz(gl, file, vpz);
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

// Creating a node graph editor for ImGui
// Quick demo, not production code! This is more of a demo of how to use
// ImGui to create custom stuff. Better version by @daniel_collin here
// https://gist.github.com/emoon/b8ff4b4ce4f1b43e79f2 See
// https://github.com/ocornut/imgui/issues/306 v0.03: fixed grid offset
// issue, inverted sign of 'scrolling' Animated gif:
// https://cloud.githubusercontent.com/assets/8225057/9472357/c0263c04-4b4c-11e5-9fdf-2cd4f33f6582.gif

// NB: You can use math functions/operators on ImVec2 if you #define
// IMGUI_DEFINE_MATH_OPERATORS and #include "imgui_internal.h" Here we only
// declare simple +/- operators so others don't leak into the demo code.
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

// Really dumb data structure provided for the example.
// Note that we storing links are INDICES (not ID) to make example code
// shorter, obviously a bad idea for any general purpose code.
// static void
// ShowExampleAppCustomNodeGraph(bool* opened)
// {
//     ImGui::SetNextWindowSize(ImVec2(700, 600),
//     ImGuiSetCond_FirstUseEver); if (!ImGui::Begin("Example: Custom Node
//     Graph", opened)) {
//         ImGui::End();
//         return;
//     }

//     // Dummy
//     struct Node
//     {
//         int ID;
//         char Name[32];
//         ImVec2 Pos, Size;
//         float Value;
//         ImVec4 Color;
//         int InputsCount, OutputsCount;

//         Node(int id,
//              const char* name,
//              const ImVec2& pos,
//              float value,
//              const ImVec4& color,
//              int inputs_count,
//              int outputs_count)
//         {
//             ID = id;
//             strncpy(Name, name, 31);
//             Name[31] = 0;
//             Pos = pos;
//             Value = value;
//             Color = color;
//             InputsCount = inputs_count;
//             OutputsCount = outputs_count;
//         }

//         ImVec2 GetInputSlotPos(int slot_no) const
//         {
//             return ImVec2(Pos.x,
//                           Pos.y + Size.y * ((float)slot_no + 1) /
//                                     ((float)InputsCount + 1));
//         }
//         ImVec2 GetOutputSlotPos(int slot_no) const
//         {
//             return ImVec2(Pos.x + Size.x,
//                           Pos.y + Size.y * ((float)slot_no + 1) /
//                                     ((float)OutputsCount + 1));
//         }
//     };

//     struct NodeLink
//     {
//         int InputIdx, InputSlot, OutputIdx, OutputSlot;

//         NodeLink(int input_idx,
//                  int input_slot,
//                  int output_idx,
//                  int output_slot)
//         {
//             InputIdx = input_idx;
//             InputSlot = input_slot;
//             OutputIdx = output_idx;
//             OutputSlot = output_slot;
//         }
//     };

//     static ImVector<Node> nodes;
//     static ImVector<NodeLink> links;
//     static bool inited = false;
//     static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
//     static bool show_grid = true;
//     static int node_selected = -1;
//     if (!inited) {
//         nodes.push_back(Node(
//           0, "MainTex", ImVec2(40, 50), 0.5f, ImColor(255, 100, 100), 1,
//           1));
//         nodes.push_back(Node(
//           1, "BumpMap", ImVec2(40, 150), 0.42f, ImColor(200, 100, 200),
//           1, 1));
//         nodes.push_back(Node(
//           2, "Combine", ImVec2(270, 80), 1.0f, ImColor(0, 200, 100), 2,
//           2));
//         links.push_back(NodeLink(0, 0, 2, 0));
//         links.push_back(NodeLink(1, 0, 2, 1));
//         inited = true;
//     }

//     // Draw a list of nodes on the left side
//     bool open_context_menu = false;
//     int node_hovered_in_list = -1;
//     int node_hovered_in_scene = -1;
//     ImGui::BeginChild("node_list", ImVec2(100, 0));
//     ImGui::Text("Nodes");
//     ImGui::Separator();
//     for (int node_idx = 0; node_idx < nodes.Size; node_idx++) {
//         Node* node = &nodes[node_idx];
//         ImGui::PushID(node->ID);
//         if (ImGui::Selectable(node->Name, node->ID == node_selected))
//             node_selected = node->ID;
//         if (ImGui::IsItemHovered()) {
//             node_hovered_in_list = node->ID;
//             open_context_menu |= ImGui::IsMouseClicked(1);
//         }
//         ImGui::PopID();
//     }
//     ImGui::EndChild();

//     ImGui::SameLine();
//     ImGui::BeginGroup();

//     const float NODE_SLOT_RADIUS = 4.0f;
//     const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

//     // Create our child canvas
//     ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)",
//                 scrolling.x,
//                 scrolling.y);
//     ImGui::SameLine(ImGui::GetWindowWidth() - 100);
//     ImGui::Checkbox("Show grid", &show_grid);
//     ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
//     ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
//     ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, IM_COL32(60, 60, 70,
//     200)); ImGui::BeginChild("scrolling_region",
//                       ImVec2(0, 0),
//                       true,
//                       ImGuiWindowFlags_NoScrollbar |
//                       ImGuiWindowFlags_NoMove);
//     ImGui::PushItemWidth(120.0f);

//     ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling;
//     ImDrawList* draw_list = ImGui::GetWindowDrawList();
//     // Display grid
//     if (show_grid) {
//         ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
//         float GRID_SZ = 64.0f;
//         ImVec2 win_pos = ImGui::GetCursorScreenPos();
//         ImVec2 canvas_sz = ImGui::GetWindowSize();
//         for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x;
//              x += GRID_SZ)
//             draw_list->AddLine(ImVec2(x, 0.0f) + win_pos,
//                                ImVec2(x, canvas_sz.y) + win_pos,
//                                GRID_COLOR);
//         for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y;
//              y += GRID_SZ)
//             draw_list->AddLine(ImVec2(0.0f, y) + win_pos,
//                                ImVec2(canvas_sz.x, y) + win_pos,
//                                GRID_COLOR);
//     }

//     // Display links
//     draw_list->ChannelsSplit(2);
//     draw_list->ChannelsSetCurrent(0); // Background
//     for (int link_idx = 0; link_idx < links.Size; link_idx++) {
//         NodeLink* link = &links[link_idx];
//         Node* node_inp = &nodes[link->InputIdx];
//         Node* node_out = &nodes[link->OutputIdx];
//         ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link->InputSlot);
//         ImVec2 p2 = offset + node_out->GetInputSlotPos(link->OutputSlot);
//         draw_list->AddBezierCurve(p1,
//                                   p1 + ImVec2(+50, 0),
//                                   p2 + ImVec2(-50, 0),
//                                   p2,
//                                   IM_COL32(200, 200, 100, 255),
//                                   3.0f);
//     }

//     // Display nodes
//     for (int node_idx = 0; node_idx < nodes.Size; node_idx++) {
//         Node* node = &nodes[node_idx];
//         ImGui::PushID(node->ID);
//         ImVec2 node_rect_min = offset + node->Pos;

//         // Display node contents first
//         draw_list->ChannelsSetCurrent(1); // Foreground
//         bool old_any_active = ImGui::IsAnyItemActive();
//         ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
//         ImGui::BeginGroup(); // Lock horizontal position
//         ImGui::Text("%s", node->Name);
//         ImGui::SliderFloat("##value", &node->Value, 0.0f, 1.0f, "Alpha
//         %.2f"); ImGui::ColorEdit3("##color", &node->Color.x);
//         ImGui::EndGroup();

//         // Save the size of what we have emitted and whether any of the
//         widgets
//         // are being used
//         bool node_widgets_active =
//           (!old_any_active && ImGui::IsAnyItemActive());
//         node->Size =
//           ImGui::GetItemRectSize() + NODE_WINDOW_PADDING +
//           NODE_WINDOW_PADDING;
//         ImVec2 node_rect_max = node_rect_min + node->Size;

//         // Display node box
//         draw_list->ChannelsSetCurrent(0); // Background
//         ImGui::SetCursorScreenPos(node_rect_min);
//         ImGui::InvisibleButton("node", node->Size);
//         if (ImGui::IsItemHovered()) {
//             node_hovered_in_scene = node->ID;
//             open_context_menu |= ImGui::IsMouseClicked(1);
//         }
//         bool node_moving_active = ImGui::IsItemActive();
//         if (node_widgets_active || node_moving_active)
//             node_selected = node->ID;
//         if (node_moving_active && ImGui::IsMouseDragging(0))
//             node->Pos = node->Pos + ImGui::GetIO().MouseDelta;

//         ImU32 node_bg_color =
//           (node_hovered_in_list == node->ID ||
//            node_hovered_in_scene == node->ID ||
//            (node_hovered_in_list == -1 && node_selected == node->ID))
//             ? IM_COL32(75, 75, 75, 255)
//             : IM_COL32(60, 60, 60, 255);
//         draw_list->AddRectFilled(
//           node_rect_min, node_rect_max, node_bg_color, 4.0f);
//         draw_list->AddRect(
//           node_rect_min, node_rect_max, IM_COL32(100, 100, 100,
//           255), 4.0f);
//         for (int slot_idx = 0; slot_idx < node->InputsCount; slot_idx++)
//             draw_list->AddCircleFilled(offset +
//                                          node->GetInputSlotPos(slot_idx),
//                                        NODE_SLOT_RADIUS,
//                                        IM_COL32(150, 150, 150, 150));
//         for (int slot_idx = 0; slot_idx < node->OutputsCount; slot_idx++)
//             draw_list->AddCircleFilled(offset +
//                                          node->GetOutputSlotPos(slot_idx),
//                                        NODE_SLOT_RADIUS,
//                                        IM_COL32(150, 150, 150, 150));

//         ImGui::PopID();
//     }
//     draw_list->ChannelsMerge();

//     // Open context menu
//     if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() &&
//         ImGui::IsMouseClicked(1)) {
//         node_selected = node_hovered_in_list = node_hovered_in_scene =
//         -1; open_context_menu = true;
//     }
//     if (open_context_menu) {
//         ImGui::OpenPopup("context_menu");
//         if (node_hovered_in_list != -1)
//             node_selected = node_hovered_in_list;
//         if (node_hovered_in_scene != -1)
//             node_selected = node_hovered_in_scene;
//     }

//     // Draw context menu
//     ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
//     if (ImGui::BeginPopup("context_menu")) {
//         Node* node = node_selected != -1 ? &nodes[node_selected] : NULL;
//         ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() -
//         offset; if (node) {
//             ImGui::Text("Node '%s'", node->Name);
//             ImGui::Separator();
//             if (ImGui::MenuItem("Rename..", NULL, false, false)) {
//             }
//             if (ImGui::MenuItem("Delete", NULL, false, false)) {
//             }
//             if (ImGui::MenuItem("Copy", NULL, false, false)) {
//             }
//         } else {
//             if (ImGui::MenuItem("Add")) {
//                 nodes.push_back(Node(nodes.Size,
//                                      "New node",
//                                      scene_pos,
//                                      0.5f,
//                                      ImColor(100, 100, 200),
//                                      2,
//                                      2));
//             }
//             if (ImGui::MenuItem("Paste", NULL, false, false)) {
//             }
//         }
//         ImGui::EndPopup();
//     }
//     ImGui::PopStyleVar();

//     // Scrolling
//     if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() &&
//         ImGui::IsMouseDragging(2, 0.0f))
//         scrolling = scrolling + ImGui::GetIO().MouseDelta;

//     ImGui::PopItemWidth();
//     ImGui::EndChild();
//     ImGui::PopStyleColor();
//     ImGui::PopStyleVar(2);
//     ImGui::EndGroup();

//     ImGui::End();
// }

} // namespace glvle
} // namespace vle
