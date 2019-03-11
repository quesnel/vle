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

#ifndef ORG_VLEPROJECT_GLVLE_HPP
#define ORG_VLEPROJECT_GLVLE_HPP

#include <algorithm>
#include <future>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <vle/utils/Package.hpp>
#include <vle/vpz/CoupledModel.hpp>
#include <vle/vpz/Vpz.hpp>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace vle {
namespace glvle {

struct Gltxt
{
    enum class status
    {
        success,
        uninitialized,
        reading,
        internal_error,
        access_file_error,
        open_file_error,
        big_file_error,
    };

    Gltxt(const std::string& file);

    bool show();

    std::string file;
    std::string content;
    std::future<std::tuple<std::string, status>> future_content;
    status st = status::uninitialized;
};

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
        std::sort(elements.begin(),
                  elements.end(),
                  [](const auto& lhs, const auto& rhs) {
                      return lhs.name < rhs.name;
                  });
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

struct Glvpz
{
    Glvpz() = default;

    bool open(const std::string& file);

    bool show();
    void show_left();
    void show_center();

    enum class status
    {
        success,
        uninitialized,
        loading,
        read_error
    };

    enum class show_vpz_type
    {
        none,
        node,
        conditions,
        dynamics,
        view
    };

    std::string id;
    std::string file;
    std::shared_ptr<vle::vpz::Vpz> vpz;
    status st = status::uninitialized;
    show_vpz_type show_type = show_vpz_type::none;
};

/**
 * @brief Modal dialog box to select a directory.
 *
 * @param dialog box name.
 * @param descriptiopn of the dialog box.
 * @param path default search path.
 *
 * @return out A file name and a boolean. If boolean is true, directory name is
 *     valid.
 */
bool
select_file_dialog(const char* name,
                   const char* description,
                   const char* filters,
                   const char* path,
                   std::string& out);

/**
 * @brief Modal dialog box to select a directory.
 *
 * @param dialog box name.
 * @param descriptiopn of the dialog box.
 * @param path default search path.
 *
 * @return A directory name and a boolean. If boolean is true, directory name
 *     is valid.
 */
bool
select_directory_dialog(const char* name,
                        const char* descriptiopn,
                        const char* path,
                        std::string& pathname,
                        std::string& dirname);

/**
 * @brief Modal dialog box to select a directory.
 *
 * @param dialog box name.
 * @param descriptiopn of the dialog box.
 * @param path default search path.
 *
 * @return A directory name and a boolean. If boolean is true, directory name
 *     is valid.
 */
bool
select_new_directory_dialog(const char* name,
                            const char* descriptiopn,
                            const char* path,
                            std::string& pathname,
                            std::string& dirname);

struct Glvle;

struct Glpackage
{
    enum class status
    {
        success,
        uninitialized,
        reading,
        open_package_error
    };

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
        std::vector<directory> dir_child;
        std::vector<file> file_child;

        directory() = default;
        directory& operator=(const directory& dir) = default;
        directory& operator=(directory&& dir) noexcept = default;
        directory(const directory& dir) = default;
        directory(directory&& dir) noexcept = default;

        directory(vle::utils::Path path_)
          : path(std::move(path_))
        {}

        void clear()
        {
            path.clear();
            dir_child.clear();
            file_child.clear();
        }

        void refresh();
    };

    std::shared_ptr<vle::utils::Package> package;
    directory current;
    unsigned long id_generator = 0;
    status st = status::uninitialized;

    Glpackage() = default;

    void open(vle::utils::ContextPtr ctx,
              const std::string& package_name,
              bool create);
    bool show(Glvle& gv);
    void clear() noexcept;
};

struct Glvle
{
    Glvle()
      : ctx(vle::utils::make_context())
    {}

    std::string working_dir;
    std::string package_name;

    vle::utils::ContextPtr ctx;

    std::unordered_map<std::string, Gltxt> txt_files;
    std::unordered_map<std::string, Glvpz> vpz_files;

    Glpackage package;

    void open(const std::string& path_name,
              const std::string& dir_name,
              bool create)
    {
        clear();

        vle::utils::Path::current_path(path_name);
        package.open(ctx, dir_name, create);

        package_name = dir_name;
        working_dir = path_name;

        have_package = true;
        show_package_window = true;
    }

    void clear()
    {
        package_name.clear();
        txt_files.clear();
        vpz_files.clear();
        package.clear();

        have_package = false;
        show_package_window = false;
    }

    bool have_package = false;
    bool show_package_window = false;
};

void
show_app_menubar(Glvle& gv);

} // namespace glvle
} // namespace vle

#endif
