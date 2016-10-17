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

#include <vle/vpz2/vpz.hpp>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace vle {
namespace vpz2 {
namespace details {
template<std::size_t Size>
bool
is_size_valid(std::string id) noexcept
{
    return id.length() < Size;
}

bool
is_characters_valid(std::string id) noexcept
{
    for (auto c : id)
        if (!std::isalnum(c))
            return false;

    return true;
}
}

bool
is_valid(std::string id) noexcept
{
    constexpr std::size_t size = sizeof(std::string);

    return details::is_size_valid<size>(id) and
           details::is_characters_valid(id);
}
}
}
