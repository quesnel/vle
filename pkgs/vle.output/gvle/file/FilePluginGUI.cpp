/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * https://www.vle-project.org
 *
 * Copyright (c) 2015-2018 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and contributors
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

#include "FilePluginGUI.h"
#include "FilePluginGUItab.h"
#include <QObject>
#include <QtPlugin>
#include <iostream>

namespace vle {
namespace gvle {

FilePluginGUI::FilePluginGUI()
  : mLogger(0)
  , mWidgetTab(0)
{}

FilePluginGUI::~FilePluginGUI()
{
    // Nothing to do ...
}

QString
FilePluginGUI::getname()
{
    QString name = "gvle.output/file";
    return name;
}

void
FilePluginGUI::setLogger(Logger* logger)
{
    mLogger = logger;
}

void
FilePluginGUI::init(vleVpz* vpz, const QString& viewName)
{
    getWidget();
    mWidgetTab->init(vpz, viewName);
}

QWidget*
FilePluginGUI::getWidget()
{
    // If the widget has already been allocated
    if (mWidgetTab == 0) {
        // Allocate a new tab widget
        mWidgetTab = new FilePluginGUItab();
        QObject::connect(mWidgetTab,
                         SIGNAL(destroyed(QObject*)),
                         this,
                         SLOT(onTabDeleted(QObject*)));
    }
    return mWidgetTab;
}

void
FilePluginGUI::delWidget()
{
    // If widget is not allocated, nothing to do
    if (mWidgetTab == 0)
        return;

    // Delete widget and clear pointer
    delete mWidgetTab;
    mWidgetTab = 0;
}

void
FilePluginGUI::onTabDeleted(QObject* obj)
{
    // If the deleted object is the tab widget
    if (obj == mWidgetTab)
        // Update local pointer
        mWidgetTab = 0;
}
}
} // namespaces
