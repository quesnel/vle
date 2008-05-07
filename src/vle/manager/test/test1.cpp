/**
 * @file src/vle/manager/test/test1.cpp
 * @author The VLE Development Team
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment (http://vle.univ-littoral.fr)
 * Copyright (C) 2003 - 2008 The VLE Development Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */




#define BOOST_TEST_MAIN
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE manager_complete_test

#include <boost/test/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <vle/manager.hpp>
#include <vle/vpz.hpp>
#include <vle/value.hpp>
#include <vle/utils.hpp>

using namespace vle;

BOOST_AUTO_TEST_CASE(trylaunch)
{
    manager::RunQuiet r;
    r.start(utils::Path::buildPrefixSharePath(
            utils::Path::path().getPrefixDir(),
            "examples", "unittest.vpz"));

    BOOST_REQUIRE_EQUAL(r.haveError(), false);
}

BOOST_AUTO_TEST_CASE(build_experimental_frames)
{
    vpz::Vpz file(utils::Path::buildPrefixSharePath(
            utils::Path::path().getPrefixDir(),
            "examples", "unittest.vpz"));

    file.project().experiment().replicas().setSeed(123);
    file.project().experiment().replicas().setNumber(4);

    vpz::Conditions& cnds(file.project().experiment().conditions());
    cnds.get("ca").addValueToPort("x", value::DoubleFactory::create(1.0));
    cnds.get("ca").addValueToPort("x", value::DoubleFactory::create(2.0));

    manager::ManagerRunMono r(std::cout, false);
    r.start(file);

    const manager::OutputSimulationMatrix& out(r.outputSimulationMatrix());
    BOOST_REQUIRE_EQUAL(out.shape()[0],
                        (manager::OutputSimulationMatrix::size_type)4);
    BOOST_REQUIRE_EQUAL(out.shape()[1],
                        (manager::OutputSimulationMatrix::size_type)3);
}
