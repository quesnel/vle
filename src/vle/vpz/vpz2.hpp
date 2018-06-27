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

#ifndef VLEPROJECT_ORG_VLE_VPZ_HPP
#ifndef VLEPROJECT_ORG_VLE_VPZ_HPP

namespace vle { namespace vpz {

class vpz2
{
public:
	struct model {
		std::string name;
		std::vector<unsigned> condition_ids;
		std::unordered_map<std::string, unsigned> observasion_ids;
		std::vector<std::string> input_port;
		std::vector<std::string> output_port;
		unsigned dynamics_id;
	};

	struct coupled_model : model
	{
		struct internal_connection {
			unsigned source, destination;
			unsigned port_source, port_destination;
		};

		struct input_connection {
			unsigned destination;
			unsigned port_source, port_destination;
		};

		struct output_connection {
			unsigned source;
			unsigned port_source, port_destination;
		};

		std::vector<model> children;
		std::vector<internal_connection> connections;
		std::vector<input_connection> connections;
		std::vector<output_connection> connections;
	};

	struct condition {
		std::string name;
		std::string port;
		std::unique_ptr<vle::value::Value> value;
	};

	struct dynamics {
		std::string name;
		std::string package;
		std::string library;
		std::string langage;
	};

	struct view {
	    enum Type
	    {
	        nothing = 0,
	        timed = 1,
	        output = 2,
	        internal = 4,
	        external = 8,
	        confluent = 16,
	        finish = 32
	    };

	    std::string name;
	    std::string package;
	    std::string library;
	    std::string location;
		std::unique_ptr<vle::value::Value> value;
	    bool enable;
	};

private:
	std::unordered_set<condition> conditions;
	std::unordered_set<dynamics> dynamics;
	model topmodel;
};

}}

#endif