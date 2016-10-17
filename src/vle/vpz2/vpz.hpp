/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems.
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2016 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2016 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2016 INRA http://www.inra.fr
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

#ifndef VLE_VPZ2_JSON_HPP
#define VLE_VPZ2_JSON_HPP

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <vle/DllDefines.hpp>
#include <vle/value/Value.hpp>

namespace vle {
namespace vpz2 {

/**
 * @brief Checks if the string can be used as a identifier
 * @details All characters must be std::isalnum and the string length must be
 *     lesser than 10 or 22 characters for respectively 32 bits or 64 bits
 *     processors.
 *
 * @param id The string to check.
 * @return true if the string is a valid identifier.
 */
VLE_API bool
is_valid(std::string id) noexcept;

using index = int;

struct identifiers
{
    struct identifier
    {
        identifier()
          : name()
          , position(-1)
        {}

        identifier(std::string name_, index position_)
          : name(std::move(name_))
          , position(position_)
        {}

        std::string name;
        index position;
    };

    std::vector<identifier> ids;

    void sort() noexcept
    {
        std::sort(ids.begin(), ids.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.name < rhs.name;
        });
    }

    /**
     * @brief Search the index of the specific name.

     * @details Search the index of the specific name using \c std::lower_bound
     * (O(log(n))) function. If \c name is not found, the algorithm uses the
     * \c std::find_if (O(n)) function. If the index is found, the algorithm
     * returns the correct index otherwise returns -1.
     *
     * @param name The \c name to find.
     * @return -1 if the \c name is not found, a correct index otherwise.
     */
    index get(const std::string& name) const noexcept
    {
        auto it = std::lower_bound(ids.begin(),
                                   ids.end(),
                                   name,
                                   [](const auto& id, const std::string& name) {
                                       return id.name == name;
                                   });

        if (it != ids.end() and it->name == name)
            return it->position;

        it = std::find_if(ids.begin(), ids.end(), [&name](const auto& id) {
            return id.name == name;
        });

        if (it != ids.end())
            return it->position;

        return -1;
    }
};

struct node
{
    std::vector<index> conditions;
    std::vector<index> observables;
    index input_ports;
    index output_ports;
    index dynamics;
    index submodel;
    index parent;
    bool debug;
};

struct connection
{
    index model_source;
    index port_source;
    index model_destination;
    index port_destination;
};

struct submodels
{
    std::vector<index> nodes;
    std::vector<connection> connections;
};

struct graph
{
    identifiers ids;
    std::vector<node> nodes;
    std::vector<submodels> models;
    std::vector<connection> connections;
};

struct dynamics
{
    struct dynamic_data
    {
        std::string package;
        std::string library;
    };

    std::vector<dynamic_data> data;
};

struct conditions
{
    identifiers ids;

    struct condition_data
    {
        std::vector<std::string> ports;
        std::vector<std::unique_ptr<value::Value>> values;
    };

    std::vector<condition_data> data;
};

struct classes
{
    identifiers ids;

    std::vector<graph> model;
};

struct views
{
    identifiers ids;

    enum type
    {
        NOTHING = 0,
        TIMED = 1 << 1,
        OUTPUT = 1 << 2,
        INTERNAL = 1 << 3,
        EXTERNAL = 1 << 4,
        CONFLUENT = 1 << 5,
        FINISH = 1 << 6
    };

    struct view_data
    {
        std::string output;
        std::unique_ptr<value::Value> data;
        views::type type;
        bool enabled;
    };

    std::vector<view_data> data;
};

struct vpz
{
    std::string name;
    std::string author;
    std::string date;
    graph model;

    dynamics dyns;
    conditions conds;
    classes cls;
    views vs;
};
}
} // namespace vle vpz

#endif
