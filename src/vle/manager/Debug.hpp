/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems.
 * http://www.vle-project.org
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

#ifndef VLE_MANAGER_DEBUG_HPP
#define VLE_MANAGER_DEBUG_HPP

#include <tuple>
#include <string>
#include <vector>

namespace vle { namespace debug {

enum class model_transition { init, internal, external, confluent, output};

enum class simulation_status { not_started, paused, finished, failure };

struct VLE_API  breakpoint {
	int id;
	breakpoint_type type;
};

struct VLE_API  relative_time
{
	vle::devs::Time m_time;

	vle::devs::Time next(vle::devs::Time current)
	{
		return current + m_time;
	}
};

struct VLE_API absolute_time
{
	vle::devs::Time m_time;

	vle::devs::Time next(vle::devs::Time current)
	{
		if (current < m_time)
			return m_time;

		return vle::devs::infinity;
	}
};

struct VLE_API atomic_model
{
	std::string name;
	std::string dynamics;
	std::vector<std::string> inputs;
	std::vector<std::string> outputs;
};

struct VLE_API coupled_model
{
	std::string name;
	std::vector<std::string> inputs;
	std::vector<std::string> outputs;
	std::vector<int> children;
	std::vector<std::tuple<int, std::string, int std::string>> connections;
};

class VLE_API Debugger
{
private:
	struct pimpl;
	std::unique_ptr<Debugger::pimpl> m_pimpl;

public:
	Debugger(const vle::utils::ContextPtr& ctx, const vle::vpz::Vpz& vpz);
	~Debugger();

	int add_breakpoint(std::string model, model_transition transition);
	int add_breakpoint(relative_time time);
	int add_breakpoint(absolute_time time);
	void remove_break(int break);
	std::vector<int> list_breakpoints();
	breakpoint show_breakpoint(int break);

	simulation_status run();
	simulation_status cont();
	simulation_status next();
	simulation_status step(unsigned number);

	std::vector<int> list_models();
	model show_model(int model);

	bool add_output_event(int model, std::string port, std::unique_ptr<vle::value::Value> value);
	bool add_input_event(int model, std::string port, std::unique_ptr<vle::value::Value> value);
};

}}

#endif
