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

#ifndef VLEPROJECT_ORG_VLE_DEVS_MULTICOMPONENT_HPP
#define VLEPROJECT_ORG_VLE_DEVS_MULTICOMPONENT_HPP

#include <vle/devs/Dynamics.hpp>

#include <algorithm>
#include <vector>

namespace package {

struct heap_element
{
    vle::devs::Time time;
    unsigned id;
};

struct heap_element_compare
{
    bool operator()(const heap_element& lhs, const heap_element& rhs) const
    {
        return lhs.time > rhs.time;
    }
};

struct Scheduler
{
    std::vector<heap_element> data;
    std::vector<unsigned> imminent;

    void make_heap()
    {
        std::make_heap(data.begin(), data.end(), heap_element_compare());
    }

    void make_imminent()
    {
        imminent.clear();

        if (data.empty())
            return;

        vle::devs::Time t = data.back().time;
        imminent.emplace_back(data.back().id);
        data.pop_back();

        while (not data.empty() and t == data.back().time) {
            imminent.emplace_back(data.back().id);
            data.pop_back();
        }
    }

    void remove(const std::vector<unsigned>& models)
    {
        std::size_t element_to_swap{ data.size() - 1 };
        std::size_t i{ 0 };

        while (i <= element_to_swap) {
            for (auto model : models) {
                if (data[i].id == model) {
                    data[i].time = data[element_to_swap].time;
                    data[i].id = data[element_to_swap].id;
                    data.pop_back();
                    element_to_swap--;
                    --i;
                    break;
                }
            }
            ++i;
        }

        make_heap();
    }
};

/**
 * @brief Wraps a multicomponent system.
 *
 * @details Stores a list of @c vle::devs::Component and a scheduler to
 * perform simulation of a DEVS MultiComponent.
 */
template<typename State, typename ChangeState, typename ModelList>
class MultiComponent : public vle::devs::Dynamics
{
public:
    using state_t = State;
    using change_t = ChangeState;
    using modellist_t = ModelList;

private:
    struct react
    {
        std::vector<change_t> changes;
        unsigned id;
    };

    mutable Scheduler m_scheduler;
    mutable modellist_t m_components;

public:
    MultiComponent(const vle::devs::DynamicsInit& init,
                   const vle::devs::InitEventList& events)
      : vle::devs::Dynamics(init, events)
      , m_components(getModel(), events)
    {}

    ~MultiComponent() override = default;

    vle::devs::Time init(vle::devs::Time time) override
    {
        auto ret = std::numeric_limits<vle::devs::Time>::max();
        const auto range = m_components.range();

        for (unsigned i = range.first; i != range.second; ++i) {
            auto tn = m_components.init(i, time);
            if (tn < 0.0)
                throw vle::utils::ModellingError(
                  "Negative init function from "
                  "component %u in MultiComponent  "
                  "'%s' (%f)",
                  i,
                  getModel().getName().c_str(),
                  tn);

            if (not vle::devs::isInfinity(tn))
                m_scheduler.data.emplace_back(tn, i);

            ret = std::min(tn, ret);
        }

        m_scheduler.make_heap();

        return ret;
    }

    void output(vle::devs::Time time,
                vle::devs::ExternalEventList& output) const override
    {
        m_scheduler.make_imminent();

        for (const auto& elem : m_scheduler.imminent)
            m_components.output(elem, time, output);
    }

    vle::devs::Time timeAdvance() const override
    {
        const auto copy_imminent = m_scheduler.imminent;
        m_scheduler.imminent.clear();

        auto ret = std::numeric_limits<vle::devs::Time>::max();
        for (auto elem : copy_imminent) {
            auto tn = m_components.timeAdvance(elem);
            if (tn < 0.0)
                throw vle::utils::ModellingError(
                  "Negative ta function from "
                  "component %u in MultiComponent  "
                  "'%s' (%f)",
                  elem,
                  getModel().getName().c_str(),
                  tn);

            if (not vle::devs::isInfinity(tn))
                m_scheduler.data.emplace_back(tn, elem);

            ret = std::min(tn, ret);
        }

        m_scheduler.make_heap();

        return ret;
    }

    void internalTransition(vle::devs::Time time) override
    {
        //
        // m_scheduler.imminent is filled by the previous call to the output
        // function that fills the imminent list.
        //

        for (const auto& elem : m_scheduler.imminent)
            m_components.internalTransition(elem, time);
    }

    void externalTransition(const vle::devs::ExternalEventList& events,
                            vle::devs::Time time) override
    {
        for (const auto& event : events) {

            auto models = m_components.influenced(event.getPortName());

            m_scheduler.remove(models);
            m_scheduler.imminent.insert(
              m_scheduler.imminent.end(), models.begin(), models.end());

            for (auto model : models)
                m_components.externalTransition(model, event, time);
        }
    }

    void confluentTransitions(
      vle::devs::Time time,
      const vle::devs::ExternalEventList& extEventlist) override
    {
        auto models = m_components.getInfluencees(extEventlist);

        //
        // At least on component is in conflict (internal and external event
        // occurred at the same time). Try to found this or these components
        // and call the internal, confluent or external transition.
        //

        m_scheduler.make_imminent();
        m_scheduler.remove(models);

        auto imm = m_scheduler.imminent;

        while (not imm.empty()) {
            auto it = std::find_if(models.begin(),
                                   models.end(),
                                   std::equal_to<unsigned>(imm.back()));

            //
            // @c `it' is in conflict from the imminent list and from the
            //     perturbed model list. Call confluent transition.
            //

            if (it != models.end()) {
                m_components.confluentTransitions(
                  imm.back(), time, extEventlist);

                *it = models.back();
                models.pop_back();
            } else {
                m_components.internalTransition(imm.back(), time);
            }

            imm.pop_back();
        }

        //
        // If their is still model in influenced list, call external
        // transition.
        //

        for (auto model : models)
            m_components.externalTransition(model, extEventlist, time);

        //
        // Finally, compute the imminent list to be call by the time advance
        // function.
        //

        m_scheduler.imminent.insert(
          m_scheduler.imminent.end(), models.begin(), models.end());
    }

    void finish() override
    {
        m_components.finish();
    }
};

} // namespace package

#endif
