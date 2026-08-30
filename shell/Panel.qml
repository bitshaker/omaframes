pragma ComponentBehavior: Bound

import QtQuick
import Quickshell
import Quickshell.Io
import qs.Commons
import qs.Ui

Panel {
  id: root

  moduleName: "bitshaker.omaframes"
  ipcTarget: "bitshaker.omaframes"
  manageIpc: false

  property int selectedIndex: 0
  property bool cursorActive: false

  readonly property color foreground: bar ? bar.foreground : Color.foreground
  readonly property color urgent: bar ? bar.urgent : Color.urgent
  readonly property color dim: Qt.darker(foreground, 1.55)
  readonly property string fontFamily: bar ? bar.fontFamily : Style.font.family
  readonly property color barIconColor: omaframes.loaded
    ? (omaframes.anyActive ? barForeground : Qt.darker(barForeground, 1.35))
    : Qt.darker(barForeground, 1.65)
  readonly property string statusText: {
    if (!omaframes.reachable) return "Waiting for Hyprland"
    if (!omaframes.loaded) return "Native host unloaded"
    if (omaframes.activeCount === 0) return "Loaded · effects paused"
    return "Loaded · " + omaframes.activeCount + (omaframes.activeCount === 1 ? " effect active" : " effects active")
  }
  readonly property string barTooltip: {
    if (!omaframes.loaded) return "OmaFrames · unloaded"
    return "OmaFrames · " + omaframes.activeCount + (omaframes.activeCount === 1 ? " effect" : " effects") + " active"
  }

  function moveCursor(delta) {
    cursorActive = true
    selectedIndex = Math.max(0, Math.min(2, selectedIndex + delta))
  }

  function setCursor(index) {
    cursorActive = true
    selectedIndex = index
  }

  function activateCursor() {
    if (selectedIndex === 0) omaframes.toggleHost()
    else if (selectedIndex === 1) omaframes.togglePack("vines")
    else if (selectedIndex === 2) omaframes.togglePack("critter")
  }

  implicitWidth: button.implicitWidth
  implicitHeight: button.implicitHeight

  onOpenedChanged: if (opened) {
    cursorActive = false
    selectedIndex = 0
    omaframes.refresh()
    Qt.callLater(function() { keyCatcher.forceActiveFocus() })
  }

  Service {
    id: omaframes
    settings: root.settings
  }

  IpcHandler {
    target: root.ipcTarget
    function open(): void { root.open() }
    function close(): void { root.close() }
    function show(): void { root.open() }
    function hide(): void { root.close() }
    function toggle(): void { root.toggle() }
    function refresh(): string { omaframes.refresh(); return "ok" }
    function toggleHost(): string { omaframes.toggleHost(); return "ok" }
    function toggleVines(): string { omaframes.togglePack("vines"); return "ok" }
    function toggleCritter(): string { omaframes.togglePack("critter"); return "ok" }
    function status(): string {
      return JSON.stringify({
        loaded: omaframes.loaded,
        vines: omaframes.vinesEnabled,
        critter: omaframes.critterEnabled,
        pluginPath: omaframes.pluginPath,
        error: omaframes.lastError
      })
    }
  }

  BarIconButton {
    id: button
    anchors.fill: parent
    bar: root.bar
    tooltipText: root.barTooltip
    iconComponent: Component {
      OmaFramesIcon {
        anchors.centerIn: parent
        iconSize: Style.space(12)
        color: root.barIconColor
        loaded: omaframes.loaded
        active: omaframes.anyActive
      }
    }
    onPressed: function(buttonCode) {
      if (buttonCode === Qt.RightButton) omaframes.toggleHost()
      else if (buttonCode === Qt.MiddleButton) omaframes.refresh()
      else root.toggle()
    }
  }

  KeyboardPanel {
    id: popup
    anchorItem: button
    owner: root
    bar: root.bar
    open: root.opened
    focusTarget: keyCatcher
    contentWidth: popup.fittedContentWidth(Style.space(340))
    contentHeight: popup.fittedContentHeight(content.implicitHeight, Style.space(500))

    PanelKeyCatcher {
      id: keyCatcher
      anchors.fill: parent
      onMoveRequested: function(dx, dy) {
        if (!root.cursorActive) {
          root.cursorActive = true
          return
        }
        if (dy !== 0) root.moveCursor(dy)
      }
      onActivateRequested: if (root.cursorActive) root.activateCursor()
      onCloseRequested: root.close()
      onTabRequested: function(direction) { root.switchPanel(direction) }
      onTextKey: function(text) {
        if (text === "r" || text === "R") omaframes.refresh()
        else if (text === "v" || text === "V") omaframes.togglePack("vines")
        else if (text === "c" || text === "C") omaframes.togglePack("critter")
      }

      Column {
        id: content
        width: parent.width
        spacing: Style.space(10)

        Row {
          width: parent.width
          spacing: Style.space(10)

          Item {
            width: Style.space(38)
            height: width

            OmaFramesIcon {
              anchors.centerIn: parent
              iconSize: Style.space(26)
              color: root.foreground
              loaded: omaframes.loaded
              active: omaframes.anyActive
            }
          }

          Column {
            width: parent.width - Style.space(48)
            anchors.verticalCenter: parent.verticalCenter
            spacing: Style.space(2)

            Text {
              width: parent.width
              text: "OmaFrames"
              color: root.foreground
              font.family: root.fontFamily
              font.pixelSize: Style.font.subtitle
              font.bold: true
            }

            Text {
              width: parent.width
              text: root.statusText
              color: root.dim
              font.family: root.fontFamily
              font.pixelSize: Style.font.bodySmall
              elide: Text.ElideRight
            }
          }
        }

        Text {
          visible: omaframes.actionStatus !== "" || omaframes.lastError !== ""
          width: parent.width
          text: omaframes.lastError !== "" ? omaframes.lastError : omaframes.actionStatus
          color: omaframes.lastError !== "" ? root.urgent : root.dim
          font.family: root.fontFamily
          font.pixelSize: Style.font.bodySmall
          wrapMode: Text.WordWrap
        }

        PanelSeparator {
          foreground: root.foreground
        }

        Toggle {
          width: parent.width
          label: "Native host"
          description: omaframes.loaded
            ? "Unload the compositor renderer"
            : "Load the compositor renderer"
          checked: omaframes.loaded
          hasCursor: root.cursorActive && root.selectedIndex === 0
          enabled: !omaframes.busy && omaframes.pluginAvailable
          opacity: enabled ? 1.0 : 0.52
          foreground: root.foreground
          fontFamily: root.fontFamily
          onHovered: function(on) { if (on) root.setCursor(0) }
          onClicked: omaframes.toggleHost()
        }

        PanelSectionHeader {
          text: "EFFECT PACKS"
          foreground: root.foreground
          fontFamily: root.fontFamily
        }

        Toggle {
          width: parent.width
          label: "Vines"
          description: "Living stems, leaves and buds around windows"
          checked: omaframes.vinesEnabled
          hasCursor: root.cursorActive && root.selectedIndex === 1
          enabled: omaframes.loaded && !omaframes.busy
          opacity: enabled ? 1.0 : 0.52
          foreground: root.foreground
          fontFamily: root.fontFamily
          onHovered: function(on) { if (on) root.setCursor(1) }
          onClicked: omaframes.togglePack("vines")
        }

        Toggle {
          width: parent.width
          label: "Chameleon"
          description: "Changes with window focus and themes as it walks and jumps"
          checked: omaframes.critterEnabled
          hasCursor: root.cursorActive && root.selectedIndex === 2
          enabled: omaframes.loaded && !omaframes.busy
          opacity: enabled ? 1.0 : 0.52
          foreground: root.foreground
          fontFamily: root.fontFamily
          onHovered: function(on) { if (on) root.setCursor(2) }
          onClicked: omaframes.togglePack("critter")
        }

        Text {
          visible: !omaframes.loaded
          width: parent.width
          text: "Load the native host to choose individual effects. Right-click the bar icon to load or unload it quickly."
          color: root.dim
          font.family: root.fontFamily
          font.pixelSize: Style.font.caption
          wrapMode: Text.WordWrap
        }
      }
    }
  }
}
