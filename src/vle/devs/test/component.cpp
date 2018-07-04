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

#include "MultiComponent.hpp"

#include <experimental/optional>

namespace package {

struct CommonState
{
    double biomass;
    double x;
    double y;
};

enum class variable_type
{
    biomass,
    x,
    y
};

enum class operation_type
{
    reset,
    zero,
    incr,
    decr,
    assign
};

struct ChangeState
{
    double value;
    variable_type var;
    operation_type op;
};

struct Forrest
{
    unsigned m_x;
    unsigned m_y;
    unsigned m_width;
    unsigned m_height;

    Forrest(unsigned x_, unsigned y_, unsigned width_, unsigned height_)
      : m_x(x_)
      , m_y(y_)
      , m_width(width_)
      , m_height(height_)
    {}

    vle::devs::Time init(vle::devs::Time time)
    {
        return 0.0;
    }

    void output(vle::devs::Time time,
                vle::devs::ExternalEventList& output) const
    {}

    vle::devs::Time timeAdvance() const
    {}

    void internalTransition(vle::devs::Time time)
    {}

    void externalTransition(const vle::devs::ExternalEventList& event,
                            vle::devs::Time time);

    void confluentTransitions(
      vle::devs::Time time,
      const vle::devs::ExternalEventList& extEventlist);

    std::unique_ptr<vle::value::Value> observation(
      const vle::devs::ObservationEvent& event) const;

    void finish()
    {}
};

struct Wall
{
    unsigned m_x;
    unsigned m_y;
    unsigned m_width;
    unsigned m_height;

    Wall(unsigned x_, unsigned y_, unsigned width_, unsigned height_)
      : m_x(x_)
      , m_y(y_)
      , m_width(width_)
      , m_height(height_)
    {}

    vle::devs::Time init(vle::devs::Time time)
    {
        return 0;
    }

    void output(vle::devs::Time time,
                vle::devs::ExternalEventList& output) const
    {}

    vle::devs::Time timeAdvance() const
    {}

    void internalTransition(vle::devs::Time time)
    {}

    void externalTransition(const vle::devs::ExternalEventList& event,
                            vle::devs::Time time);

    void confluentTransitions(
      vle::devs::Time time,
      const vle::devs::ExternalEventList& extEventlist);

    std::unique_ptr<vle::value::Value> observation(
      const vle::devs::ObservationEvent& event) const;

    void finish()
    {}
};

struct ModelList
{
    std::vector<Forrest> forrests;
    std::vector<Wall> walls;

    /** Map input port, componant */
    std::map<std::string, std::vector<unsigned>> influencers;

    /** Map output port, influenced */
    std::map<std::string, std::vector<unsigned>> influenced;

    ModelList(const vle::vpz::AtomicModel& atom,
              const vle::devs::InitEventList& /*events*/)
    {
        forrests.reserve(10 * 10);
        walls.reserve(3 * 3);
    }

    std::experimetal::optional<const std::vector<unsigned>>& influenced(
      std::string port) const
    {
        auto it = influencers.find(port);
        if (it != influencers.end())
            return std::make_optional(it->second);

        return std::make_optional{};
    }

    const std::pair<unsigned, unsigned> range() const
    {
        return std::make_pair(
          0u, static_cast<unsigned>(forrests.size() + walls.size()));
    }

    vle::devs::Time init(unsigned id, vle::devs::Time time)
    {
        return (id < forrests.size()) ? forrests[id].init(time)
                                      : walls[id - forrests.size()].init(time);
    }

    void output(unsigned id,
                vle::devs::Time time,
                vle::devs::ExternalEventList& output) const
    {}

    vle::devs::Time timeAdvance(unsigned id) const
    {}

    void internalTransition(unsigned id, vle::devs::Time time)
    {}

    void externalTransition(unsigned id,
                            const vle::devs::ExternalEventList& event,
                            vle::devs::Time time)
    {}

    void confluentTransitions(unsigned id,
                              vle::devs::Time time,
                              const vle::devs::ExternalEventList& extEventlist)
    {}

    void finish()
    {
        for (auto& elem : forrests)
            return elem.finish();

        for (auto& elem : walls)
            return elem.finish();
    }
};

using F = MultiComponent<CommonState, ChangeState, ModelList>;

} // namespace package

DECLARE_DYNAMICS(package::F)
