/**
 * \file ActionSelectionPopover.qml
 * Popup with list of actions.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015  Urs Fleisch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.2
import ".."

DropDownList {
  id: dropDownList

  property ActionList actions

  dropDownRoot: root
  width: constants.gu(25)
  model: actions.items
  onStateChanged: {
    if (state === "dropDown") {
      // Called before parent change, move popup menu to left of parent.
      x = -width
    }
  }
}
