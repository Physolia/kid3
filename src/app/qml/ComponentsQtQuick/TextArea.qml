/**
 * \file TextArea.qml
 * Multiple line text input area.
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

FocusScope {
  property alias text: textInput.text
  property alias readOnly: textInput.readOnly
  property alias selectByMouse: textInput.selectByMouse

  height: 3 * constants.rowHeight
  Rectangle {
    anchors.fill: parent
    color: constants.editColor

    Flickable {
      id: flick

      anchors.fill: parent
      anchors.leftMargin: constants.margins
      contentWidth: textInput.paintedWidth
      contentHeight: textInput.paintedHeight
      flickableDirection: Flickable.VerticalFlick
      clip: true

      function ensureVisible(r) {
        if (contentX >= r.x)
          contentX = r.x;
        else if (contentX+width <= r.x + r.width)
          contentX = r.x + r.width - width;
        if (contentY >= r.y)
          contentY = r.y;
        else if (contentY + height <= r.y + r.height)
          contentY = r.y + r.height - height;
      }

      TextEdit {
        id: textInput
        width: flick.width
        height: flick.height
        focus: true
        wrapMode: TextEdit.Wrap
        onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
      }
    }
  }
}
