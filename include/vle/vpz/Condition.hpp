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

#ifndef VLE_VPZ_CONDITION_HPP
#define VLE_VPZ_CONDITION_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <vle/DllDefines.hpp>
#include <vle/value/Map.hpp>
#include <vle/vpz/Base.hpp>

namespace vle {
namespace vpz {

/**
 * @brief Define the ConditionValues like a dictionnary, (portname, values).
 */
using ConditionValues =
  std::unordered_map<std::string, std::shared_ptr<value::Value>>;

/**
 * @brief A condition define a couple model name, port name and a Value.
 * This class allow loading and writing a condition.
 */
class VLE_API Condition : public Base
{
public:
    using iterator = ConditionValues::iterator;
    using const_iterator = ConditionValues::const_iterator;
    using size_type = ConditionValues::size_type;
    using value_type = ConditionValues::value_type;

    /**
     * @brief Build a new Condition with only a name.
     * @param name The name of the condition.
     */
    Condition(const std::string& name);

    /**
     * @brief Copy constructor. All values are cloned.
     * @param cnd The Condition to copy.
     */
    Condition(const Condition& cnd);

    /**
     * @brief Assignment operator. All values are cloned.
     */
    Condition& operator=(const Condition& cnd);

    /**
     * @brief Delete all the values attached to this Conditon.
     */
    ~Condition() override = default;

    /**
     * @brief Add Condition informations to the stream.
     * @code
     * <condition name="">
     *  <port name="">
     *   <!-- vle::value XML representation. -->
     *  </port>
     * </condition>
     * @endcode
     *
     * @param out
     */
    void write(std::ostream& out) const override;

    /**
     * @brief Get the type of this class.
     * @return CONDITION.
     */
    inline Base::type getType() const override
    {
        return VLE_VPZ_CONDITION;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
     *
     * Manage the ConditionValues
     *
     * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /**
     * @brief Build a list of string that contains all port names.
     * @param lst An output string list.
     */
    std::vector<std::string> portnames() const;

    /**
     * @brief Tells if a port exist on this condition
     * @param portname name of the port.
     * @param true if the port portname exists on this condition.
     */
    bool exist(const std::string& portname) const;

    /**
     * @brief Add a port to the value list.
     * @param portname name of the port.
     */
    void add(const std::string& portname);

    /**
     * @brief Remove a port of the value list.
     * @param portname name of the port.
     */
    void del(const std::string& portname);

    /**
     * @brief Set a value to a specified port. If port contains already
     * value, these values are deleted.
     * @param portname The name of the port to add value.
     * @param value the value to push.
     * @throw utils::ArgError if portname does not exist.
     */
    void setValueToPort(const std::string& portname,
                        std::shared_ptr<value::Value> value);
    /**
     * @brief Clear the specified port.
     * @param portname The name of the port to clear.
     * @throw utils::ArgError if portname does not exist.
     */
    void clearValueOfPort(const std::string& portname);

    /**
     * @brief Return a reference to the first value::Value of the specified
     * port.
     * @param portname the name of the port to test.
     * @return A reference to a value::Value.
     * @throw utils::ArgError if portname not exist.
     */
    const std::shared_ptr<value::Value>& valueOfPort(
      const std::string& portname) const;

    /**
     * @brief Return a reference to the value::Set of the latest added port.
     * This function is principaly used in Sax parser.
     * @return A reference to the value::Set of the port.
     * @throw utils::ArgError if port does not exist.
     */
    std::shared_ptr<value::Value>& lastAddedPort();

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
     *
     * Get/Set functions.
     *
     * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /**
     * @brief Get a reference to the ConditionValues.
     * @return A constant reference to the ConditionValues.
     */
    inline const ConditionValues& conditionvalues() const
    {
        return m_list;
    }

    /**
     * @brief Get a reference to the ConditionValues.
     * @return A constant reference to the ConditionValues.
     */
    inline ConditionValues& conditionvalues()
    {
        return m_list;
    }

    /**
     * @brief Get a iterator the begin of the vpz::ConditionValues.
     * @return Get a iterator the begin of the vpz::ConditionValues.
     */
    iterator begin()
    {
        return m_list.begin();
    }

    /**
     * @brief Get a iterator the end of the vpz::ConditionValues.
     * @return Get a iterator the end of the vpz::ConditionValues.
     */
    iterator end()
    {
        return m_list.end();
    }

    /**
     * @brief Get a constant iterator the begin of the vpz::ConditionValues.
     * @return Get a constant iterator the begin of the
     * vpz::ConditionValues.
     */
    const_iterator begin() const
    {
        return m_list.begin();
    }

    /**
     * @brief Get a constant iterator the end of the vpz::ConditionValues.
     * @return Get a constant iterator the end of the vpz::ConditionValues.
     */
    const_iterator end() const
    {
        return m_list.end();
    }

    /**
     * @brief Return the name of the condition.
     * @return
     */
    inline const std::string& name() const
    {
        return m_name;
    }

    /**
     * @brief Set a new name to this condition.
     * @param new_name The new name for this condition.
     * @return
     */
    inline void setName(const std::string& new_name)
    {
        m_name.assign(new_name);
    }

    /**
     * @brief Return true if this condition is a permanent data for the
     * devs::ModelFactory.
     * @return True if this condition is a permanent value.
     */
    inline bool isPermanent() const
    {
        return m_ispermanent;
    }

    /**
     * @brief Set the permanent value of this condition.
     * @param value True to conserve this condition in devs::ModelFactory.
     */
    inline void permanent(bool value = true)
    {
        m_ispermanent = value;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
     *
     * Functors
     *
     * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /**
     * @brief Predicate functor to check if this condition is permanet or
     * not. To use with std::find_if:
     * Example:
     * @code
     * std::vector < Condition > v;
     * [...]
     * std::vector < Condition >::iterator it =
     *    std::find_if(v.beging(), v.end(), Condition::IsPermanent());
     * @endcode
     */
    struct IsPermanent
    {
        /**
         * @brief Check if the Condition is permanent.
         * @param x the Condition to check.
         * @return True if the Condition is permanent.
         */
        bool operator()(const Condition& x) const
        {
            return x.isPermanent();
        }
    };

private:
    Condition() = delete;

    ConditionValues m_list;  /* list of port, values. */
    std::string m_name;      /* name of the condition. */
    std::string m_last_port; /* latest added port. */
    bool m_ispermanent;
};
}
} // namespace vle vpz

#endif
