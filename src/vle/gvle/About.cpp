/**
 * @file vle/gvle/About.cpp
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


#include <vle/gvle/About.hpp>
#include "config.h"

namespace vle { namespace gvle {

About::About(Glib::RefPtr < Gnome::Glade::Xml > refXml) :
    mRefXml(refXml)
{
    mRefXml->get_widget("DialogAbout", mAbout);
    std::string extra(VLE_EXTRA_VERSION);
    if (extra.empty()) {
        mAbout->set_version(VLE_VERSION);
    } else {
        std::string version(VLE_VERSION);
        version += extra;
        mAbout->set_version(version);
    }

    mAbout->signal_response().connect(
        sigc::mem_fun(*this, &About::onAboutClose));
}

void About::run()
{
    mAbout->run();
}

void About::onAboutClose(int /* response */)
{
    mAbout->hide();
}

}} // namespace vle gvle
