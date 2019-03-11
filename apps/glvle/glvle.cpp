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

namespace vle {
namespace glvle {

struct logger : vle::utils::Context::LogFunctor
{
    Glvle& gv;

    logger(Glvle& gv_)
      : gv(gv_)
    {}

    void write(const vle::utils::Context& ctx,
               int priority,
               const char* format,
               va_list args) noexcept final
    {
        if (ctx.get_log_priority() >= priority)
            gv.log_w.log(priority, format, args);
    }

    void write(const vle::utils::Context& ctx,
               int priority,
               const std::string& str) noexcept final
    {
        if (ctx.get_log_priority() >= priority)
            gv.log_w.log(priority, "%s", str.c_str());
    }
};

Glvle::Glvle()
  : ctx(vle::utils::make_context())
{
    ctx->set_log_function(std::make_unique<logger>(*this));
    ctx->set_log_priority(7);
}

} // glvle
} // vle
