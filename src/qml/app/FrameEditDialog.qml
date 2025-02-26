/**
 * \file FrameEditDialog.qml
 * Dialog to edit frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015-2018  Urs Fleisch
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

import QtQuick 2.11
import QtQuick.Controls 2.4
import Kid3 1.1 as Kid3

Dialog {
  id: page

  property QtObject frameObject
  property string imageSource
  signal frameEdited(variant frame)

  modal: true
  width: Math.min(parent.width, constants.gu(70))
  x: (parent.width - width) / 2
  y: 0
  standardButtons: Dialog.Ok | Dialog.Cancel

  function openFrame(frame) {
    page.title = frame.internalName
    fieldList.model = frame.fields
    frameObject = frame
    page.open()
    if (frame.type === Kid3.Frame.FT_Picture) {
      app.setCoverArtImageData(frame.getBinaryData())
    }
  }

  Component {
    id: textLineEdit
    TextField {
      text: _modelData.value
      selectByMouse: true
      onAccepted: {
        focus = false
      }
      onActiveFocusChanged: {
        if (!activeFocus) {
          _modelData.value = text
        }
      }
    }
  }

  Component {
    id: textEdit

    Row {
      width: parent.width

      Frame {
        // Some space is left on the right side for flicking on a touch device.
        width: parent.width - (fieldList.atYBeginning ? 0 : constants.gu(6))
        TextArea {
          id: textArea
          width: parent.width
          text: _modelData.value
          selectByMouse: true
          wrapMode: TextEdit.Wrap
          onActiveFocusChanged: {
            if (!activeFocus) {
              _modelData.value = text
            }
          }
        }
      }
    }
  }

  Component {
    id: comboBoxEdit

    ComboBox {
      model: if (_modelData.id === Kid3.Frame.ID_TextEnc)
               script.getTextEncodingNames()
             else if (_modelData.id === Kid3.Frame.ID_PictureType)
               script.getPictureTypeNames()
             else if (_modelData.id === Kid3.Frame.ID_TimestampFormat)
               script.getTimestampFormatNames()
             else if (_modelData.id === Kid3.Frame.ID_ContentType)
               script.getContentTypeNames()
      currentIndex: _modelData.value
      onCurrentIndexChanged: _modelData.value = currentIndex
    }
  }

  MessageDialog {
    signal completed(bool ok)

    id: confirmOverwriteDialog
    width: Math.min(parent.width, constants.gu(70))
    x: (parent.width - width) / 2
    y: parent.height / 6
    parent: page.parent  // Overlay.overlay
    title: qsTr("Warning")
    text: qsTr("File already exists. Overwrite?")
    standardButtons: Dialog.Yes | Dialog.No
    onYes: completed(true)
    onNo: completed(false)
  }

  Component {
    id: exportFileSelectDialog
    FileSelectDialog {
      property variant field
      parent: page.parent  // Overlay.overlay
      title: qsTr("Export")
      saveMode: true
      nameFilters: ["*.jpg", "*.jpeg", "*.png", "*.JPG", "*.JPEG", "*.PNG"]
      onFinished: {
        if (path) {
          if (script.fileExists(path)) {
            function writeIfOk(ok) {
              confirmOverwriteDialog.completed.disconnect(writeIfOk)
              if (ok) {
                script.writeFile(path, field.value)
              }
            }

            confirmOverwriteDialog.completed.connect(writeIfOk)
            confirmOverwriteDialog.open()
          } else {
            script.writeFile(path, field.value)
          }
        }
      }
    }
  }

  Component {
    id: importFileSelectDialog
    FileSelectDialog {
      property variant field
      parent: page.parent  // Overlay.overlay
      title: qsTr("Import")
      nameFilters: ["*.jpg", "*.jpeg", "*.png", "*.JPG", "*.JPEG", "*.PNG"]
      onFinished: {
        if (path) {
          field.value = script.readFile(path)
          page.imageSource = "file://" + path
        } else {
          page.imageSource = app.coverArtImageId
        }
      }
    }
  }

  Component {
    id: imageView

    Column {
      width: parent.width
      spacing: constants.spacing

      Button {
        id: importButton
        width: parent.width
        text: qsTr("Import")
        onClicked: {
          var coverFileName = configs.fileConfig().defaultCoverFileName
          if (coverFileName.indexOf("%") !== -1) {
            coverFileName = app.selectionInfo.formatString(Kid3.Frame.TagV2V1,
                                                           coverFileName)
          }
          constants.openPopup(importFileSelectDialog, importButton,
                        {"folder": app.dirName,
                         "currentFile": coverFileName,
                         "field": _modelData})
        }
      }

      Button {
        id: exportButton
        width: parent.width
        text: qsTr("Export")

        onClicked: {
          var coverFileName = configs.fileConfig().defaultCoverFileName
          if (coverFileName.indexOf("%") !== -1) {
            coverFileName = app.selectionInfo.formatString(Kid3.Frame.TagV2V1,
                                                           coverFileName)
          }
          constants.openPopup(exportFileSelectDialog, exportButton,
                        {"folder": app.dirName,
                         "currentFile": coverFileName,
                         "field": _modelData})
        }
      }

      Item {
        id: imageItem
        width: parent.width
        height: 120
        Image {
          anchors.fill: parent
          fillMode: Image.PreserveAspectFit
          source: page.imageSource
          cache: false
        }
      }
    }
  }

  contentItem: ListView {
    id: fieldList
    clip: true

    // height: someFunction(contentHeight) will report a binding loop,
    // therefore the height is updated manually.
    function updateHeight() {
      /// TODO: Calculate button height instead of 145.
      page.height = Math.min(contentHeight + 145,
                        page.parent.height -
                        3 * constants.rowHeight -
                        3 * constants.margins)
    }
    onVisibleChanged: if (visible) updateHeight()
    onContentHeightChanged: updateHeight()

    delegate: Column {
      width: parent.width
      spacing: constants.spacing
      Label {
        id: nameLabel
        text: modelData.name
      }

      Loader {
        width: parent.width
        property QtObject _modelData: modelData
        sourceComponent:
            if (typeof modelData.value === "number")
              if (modelData.id === Kid3.Frame.ID_TextEnc ||
                  modelData.id === Kid3.Frame.ID_PictureType ||
                  modelData.id === Kid3.Frame.ID_TimestampFormat ||
                  modelData.id === Kid3.Frame.ID_ContentType)
                comboBoxEdit
              else
                textLineEdit
            else if (typeof modelData.value === "string")
              if (modelData.id === Kid3.Frame.ID_Text)
                textEdit
              else
                textLineEdit
            else if (typeof modelData.value === "object")
              if (modelData.id === Kid3.Frame.ID_Data &&
                  modelData.type === Kid3.Frame.FT_Picture)
                imageView
      }
      ThinDivider {
        anchors {
          left: parent.left
          right: parent.right
        }
        visible: index != fieldList.count - 1
      }
    }
  }

  onOpened: {
    page.imageSource = app.coverArtImageId
  }
  onRejected: {
    page.close()
    page.frameEdited(null)
    frameObject = null
  }
  onAccepted: {
    fieldList.focus = false // to force editingFinished on delegate
    page.close()
    page.frameEdited(frameObject)
    // To force update of picture collapsible
    if (frameObject.type === Kid3.Frame.FT_Picture) {
      app.setCoverArtImageData(frameObject.getBinaryData())
    }
    frameObject = null
  }
}
