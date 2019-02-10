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

#include <vle/DllDefines.hpp>
#include <vle/manager/Simulation.hpp>
#include <vle/utils/Spawn.hpp>
#include <vle/utils/Tools.hpp>

#include "devs/RootCoordinator.hpp"
#include "utils/ContextPrivate.hpp"
#include "utils/i18n.hpp"

#include <boost/format.hpp>
#include <boost/progress.hpp>
#include <boost/timer.hpp>

#include <fstream>
#include <memory>
#include <sstream>
#include <utility>

namespace vle {
namespace manager {

static utils::Path
make_temp(const char* format)
{
    assert(format);

    utils::Path tmp = utils::Path::temp_directory_path();
    tmp /= format;
    tmp = utils::Path::unique_path(tmp.string());

    return tmp;
}

/* Perhaps we can returns directly the std::shared_ptr reads from the file with
 the vpz::parseValue function but all the manager's API use std::unique_ptr.

 From now, we convert the shared_ptr to unique_ptr.

 TODO perhaps check if pointer counter is 1, and release and build the
 std::unique_ptr.
 */
std::unique_ptr<value::Map>
read_value(const utils::Path& p)
{
    std::ifstream ifs(p.string());
    if (ifs.is_open()) {
        std::stringstream ss;
        ss << ifs.rdbuf();
        std::string buffer(ss.str());

        auto v = vpz::Vpz::parseValue(buffer);
        if (v and v->isMap()) {
            return std::make_unique<value::Map>(v->toMap());
        }
    }

    return std::unique_ptr<value::Map>{};
}

class Simulation::Pimpl
{
public:
    utils::ContextPtr m_context;
    std::chrono::milliseconds m_timeout;
    std::ostream* m_out;
    utils::UnlinkPath m_vpz_file;
    utils::UnlinkPath m_output_file;
    LogOptions m_logoptions;
    SimulationOptions m_simulationoptions;

    Pimpl(utils::ContextPtr context,
          LogOptions logoptions,
          SimulationOptions simulationoptionts,
          std::chrono::milliseconds timeout,
          std::ostream* output)
      : m_context(std::move(context))
      , m_timeout(timeout)
      , m_out(output)
      , m_vpz_file(make_temp("vle-%%%%-%%%%-%%%%-%%%%.vpz"))
      , m_output_file(make_temp("vle-%%%%-%%%%-%%%%-%%%%.value"))
      , m_logoptions(logoptions)
      , m_simulationoptions(simulationoptionts)
    {
        if (timeout != std::chrono::milliseconds::zero())
            m_simulationoptions |= vle::manager::SIMULATION_SPAWN_PROCESS;
    }

    template<typename T>
    void write(const T& t)
    {
        if (m_out)
            (*m_out) << t;
    }

    std::unique_ptr<value::Map> runVerboseRun(std::unique_ptr<vpz::Vpz> vpz,
                                              Error* error)
    {
        std::unique_ptr<value::Map> result;
        boost::timer timer;

        try {
            devs::RootCoordinator root(m_context);

            const double duration = vpz->project().experiment().duration();
            const double begin = vpz->project().experiment().begin();

            write(utils::format(_("[%s]\n"), vpz->filename().c_str()));
            write(_(" - Coordinator load models ......: "));

            root.load(*vpz);

            write(_("ok\n"));

            write(_(" - Clean project file ...........: "));
            vpz->clear();
            vpz.reset(nullptr);
            write(_("ok\n"));

            write(_(" - Coordinator initializing .....: "));
            root.init();
            write(_("ok\n"));

            write(_(" - Simulation run................: "));

            boost::progress_display display(
              100, *m_out, "\n   ", "   ", "   ");
            long previous = 0;

            while (root.run()) {
                auto pc = static_cast<long>(std::floor(
                  100. * (root.getCurrentTime() - begin) / duration));

                display += pc - previous;
                previous = pc;
            }

            display += 100 - previous;

            write(_(" - Coordinator cleaning .........: "));
            result = root.finish();
            write(_("ok\n"));

            write(utils::format(_(" - Time spent in kernel .........: %f s"),
                                timer.elapsed()));

            error->code = 0;
        } catch (const std::exception& e) {
            error->message =
              utils::format(_("\n/!\\ error reported: %s\n"), e.what());
            error->code = -1;
        }

        return result;
    }

    std::unique_ptr<value::Map> runVerboseSummary(
      std::unique_ptr<vpz::Vpz> vpz,
      Error* error)
    {
        std::unique_ptr<value::Map> result;
        boost::timer timer;

        try {
            devs::RootCoordinator root(m_context);

            write(utils::format(_("[%s]\n"), vpz->filename().c_str()));
            write(_(" - Coordinator load models ......: "));

            root.load(*vpz);

            write(_("ok\n"));

            write(_(" - Clean project file ...........: "));
            vpz->clear();
            vpz.reset(nullptr);
            write(_("ok\n"));

            write(_(" - Coordinator initializing .....: "));
            root.init();
            write(_("ok\n"));

            write(_(" - Simulation run................: "));

            while (root.run())
                ;
            write(_("ok\n"));

            write(_(" - Coordinator cleaning .........: "));
            result = root.finish();
            write(_("ok\n"));

            write(utils::format(_(" - Time spent in kernel .........: %f s"),
                                timer.elapsed()));

            error->code = 0;
        } catch (const std::exception& e) {
            error->message =
              utils::format(_("\n/!\\ error reported: %s\n"), e.what());
            error->code = -1;
        }

        return result;
    }

    std::unique_ptr<value::Map> runQuiet(std::unique_ptr<vpz::Vpz> vpz,
                                         Error* error)
    {
        std::unique_ptr<value::Map> result;

        try {
            devs::RootCoordinator root(m_context);

            root.load(*vpz);
            vpz->clear();
            vpz.reset(nullptr);

            root.init();
            while (root.run()) {
            }
            result = root.finish();

            error->code = 0;
        } catch (const std::exception& e) {
            error->message =
              utils::format(_("\n/!\\ error reported: %s\n"), e.what());
            error->code = -1;
        }

        return result;
    }

    std::unique_ptr<value::Map> runSubProcess(std::unique_ptr<vpz::Vpz> vpz,
                                              Error* error)
    {
        auto pwd = utils::Path::current_path();
        std::string command;

        vpz->write(m_vpz_file.string());

        try {
            m_context->get_setting("vle.command.vle.simulation", &command);
            command = (boost::format(command) % m_output_file.string() %
                       m_vpz_file.string())
                        .str();
            vle::utils::Spawn spawn(m_context);
            auto argv = spawn.splitCommandLine(command);
            auto exe = std::move(argv.front());
            argv.erase(argv.begin());

            if (not spawn.start(exe, pwd.string(), argv, m_timeout)) {
                error->code = -1;
                error->message = "fail to spawn";
                return {};
            }

            std::string output, err;
            std::string message;
            bool success;

            if (m_timeout != std::chrono::milliseconds::zero()) {
                auto duration = m_timeout;
                auto starttime = std::chrono::system_clock::now();

                while (not spawn.isfinish()) {
                    if (spawn.get(&output, &err)) {
                        if (not output.empty()) {
                            m_context->info("%s", output.c_str());
                        }
                        if (not err.empty()) {
                            m_context->error("%s", err.c_str());
                        }
                        output.clear();
                        err.clear();

                        std::this_thread::sleep_for(
                          std::chrono::microseconds(200));

                        auto endtime = std::chrono::system_clock::now();
                        auto elapsed = endtime - starttime;
                        if (elapsed > duration) {
                            printf("kill process. Too long\n");
                            spawn.kill();
                            break;
                        }
                    } else {
                        break;
                    }
                }
            } else {
                while (not spawn.isfinish()) {
                    if (spawn.get(&output, &err)) {
                        if (not output.empty()) {
                            m_context->info("%s", output.c_str());
                        }
                        if (not err.empty()) {
                            m_context->error("%s", err.c_str());
                        }
                        output.clear();
                        err.clear();

                        std::this_thread::sleep_for(
                          std::chrono::microseconds(200));
                    } else {
                        break;
                    }
                }
            }
            spawn.wait();
            spawn.status(&message, &success);

            if (not success and not message.empty()) {
                m_context->error(_("VLE failure: %s\n"), message.c_str());
                error->code = -1;
                error->message = message;

                return {};
            }
            std::unique_ptr<value::Map> results =
              read_value(m_output_file.path());
            return results;
        } catch (const std::exception& e) {
            m_context->error(_("VLE sub process: unable to start "
                               "'%s' in '%s' with "
                               "the '%s' command (%s)\n"),
                             m_vpz_file.string().c_str(),
                             pwd.string().c_str(),
                             command.c_str(),
                             e.what());

            error->code = -1;
            error->message = "fail to run";
        }

        return {};
    }
};

Simulation::Simulation(utils::ContextPtr context,
                       LogOptions logoptions,
                       SimulationOptions simulationoptionts,
                       std::chrono::milliseconds timeout,
                       std::ostream* output)
  : mPimpl(std::make_unique<Simulation::Pimpl>(context,
                                               logoptions,
                                               simulationoptionts,
                                               timeout,
                                               output))
{}

Simulation::~Simulation() = default;

std::unique_ptr<value::Map>
Simulation::run(std::unique_ptr<vpz::Vpz> vpz, Error* error)
{
    error->code = 0;
    std::unique_ptr<value::Map> result;

    if (mPimpl->m_simulationoptions & SIMULATION_SPAWN_PROCESS) {
        result = mPimpl->runSubProcess(std::move(vpz), error);
    } else {
        if (mPimpl->m_logoptions != manager::LOG_NONE) {
            if (mPimpl->m_logoptions & manager::LOG_RUN and mPimpl->m_out) {
                result = mPimpl->runVerboseRun(std::move(vpz), error);
            } else {
                result = mPimpl->runVerboseSummary(std::move(vpz), error);
            }
        } else {
            result = mPimpl->runQuiet(std::move(vpz), error);
        }
    }

    if (mPimpl->m_simulationoptions & manager::SIMULATION_NO_RETURN) {
        return {};
    } else {
        return result;
    }
}
}
}