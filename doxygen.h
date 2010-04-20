/**************************************************************************
**
** This file is part of Doxygen plugin for Qt Creator
**
** Copyright (c) 2009 Kevin Tanguy (kofee@kofee.org).
**
** This plugin is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as
** published by the Free Software Foundation, either version 2.1
** of the License, or (at your option) any later version.
**
** This plugin is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Doxygen Plugin. If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef DOXYGEN_H
#define DOXYGEN_H

#include "doxygensettingsstruct.h"

namespace DoxyPlugin {
namespace Internal {

class Doxygen
{
public:
    static Doxygen* instance();

    void createDocumentation(const DoxygenSettingsStruct::DoxygenComment &DoxyComment) const;

private:
    Doxygen();

    static Doxygen* m_instance;
};

} // namespace Internal
} // namespace DoxyPlugin
#endif // DOXYGEN_H
