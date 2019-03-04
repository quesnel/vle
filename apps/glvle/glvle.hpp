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

#ifndef ORG_VLEPROJECT_GLVLE_HPP
#define ORG_VLEPROJECT_GLVLE_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <vle/utils/Package.hpp>
#include <vle/vpz/CoupledModel.hpp>
#include <vle/vpz/Vpz.hpp>

namespace vle {
namespace glvle {

struct Gltxt
{
    enum class status
    {
        success,
        uninitialized,
        access_file_error,
        open_file_error,
        big_file_error,
    };

    std::string content;
    status st = status::uninitialized;
};

struct Glvpz
{
    Glvpz() = default;
    // Glvpz(Glvpz&& other) = default;
    // Glvpz(const Glvpz& other) = default;

    enum class status
    {
        success,
        uninitialized,
        read_error
    };

    std::shared_ptr<vle::vpz::Vpz> vpz;
    vle::vpz::CoupledModel* coupled = nullptr;
    status st = status::uninitialized;
};

struct Glvle
{
    Glvle()
      : ctx(vle::utils::make_context())
    {}

    std::string working_dir;
    std::string package;

    vle::utils::ContextPtr ctx;
    std::shared_ptr<vle::utils::Package> pkg;

    std::unordered_map<std::string, Gltxt> txt_files;
    std::unordered_map<std::string, Glvpz> vpz_files;

    bool have_package = false;
    bool show_main_menubar = true;
    bool show_package_window = false;
};

/**
 * @brief Modal dialog box to select a directory.
 *
 * @param dialog box name.
 * @param descriptiopn of the dialog box.
 * @param path default search path.
 *
 * @return out A file name and a boolean. If boolean is true, directory name is
 *     valid.
 */
bool
select_file_dialog(const char* name,
                   const char* description,
                   const char* filters,
                   const char* path,
                   std::string& out);

/**
 * @brief Modal dialog box to select a directory.
 *
 * @param dialog box name.
 * @param descriptiopn of the dialog box.
 * @param path default search path.
 *
 * @return A directory name and a boolean. If boolean is true, directory name
 *     is valid.
 */
bool
select_directory_dialog(const char* name,
                        const char* descriptiopn,
                        const char* path,
                        std::string& pathname,
                        std::string& dirname);

/**
 * @brief Modal dialog box to select a directory.
 *
 * @param dialog box name.
 * @param descriptiopn of the dialog box.
 * @param path default search path.
 *
 * @return A directory name and a boolean. If boolean is true, directory name
 *     is valid.
 */
bool
select_new_directory_dialog(const char* name,
                            const char* descriptiopn,
                            const char* path,
                            std::string& pathname,
                            std::string& dirname);

void
package_window(Glvle& gv);

void
show_app_menubar(Glvle& gv);

void
show_main_window(Glvle& gv);

bool
text_window(const std::string& file, Gltxt& txt);

bool
vpz_window(Glvle& gl, const std::string& file, Glvpz& vpz);

} // namespace glvle
} // namespace vle

#endif