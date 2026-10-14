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

  onSelectedIndexChanged: Qt.callLater(function() {
    var rows = [hostToggle, vinesToggle, critterToggle, firefliesToggle, frostToggle, railwayToggle]
    var row = rows[selectedIndex]
    if (!row) return
    if (row.y < effectsScroll.contentY) effectsScroll.contentY = row.y
    else if (row.y + row.height > effectsScroll.contentY + effectsScroll.height)
      effectsScroll.contentY = row.y + row.height - effectsScroll.height
  })

  function moveCursor(delta) {
    cursorActive = true
    selectedIndex = Math.max(0, Math.min(5, selectedIndex + delta))
  }

  function setCursor(index) {
    cursorActive = true
    selectedIndex = index
  }

  function activateCursor() {
    if (selectedIndex === 0) omaframes.toggleHost()
    else if (selectedIndex === 1) omaframes.togglePack("vines")
    else if (selectedIndex === 2) omaframes.togglePack("critter")
    else if (selectedIndex === 3) omaframes.togglePack("fireflies")
    else if (selectedIndex === 4) omaframes.togglePack("frost")
    else if (selectedIndex === 5) omaframes.togglePack("railway")

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
    function toggleFireflies(): string { omaframes.togglePack("fireflies"); return "ok" }
    function toggleFrost(): string { omaframes.togglePack("frost"); return "ok" }
    function toggleRailway(): string { omaframes.togglePack("railway"); return "ok" }
    function status(): string {
      return JSON.stringify({
        loaded: omaframes.loaded,
        vines: omaframes.vinesEnabled,
        critter: omaframes.critterEnabled,
        fireflies: omaframes.firefliesEnabled,
        frost: omaframes.frostEnabled,
        railway: omaframes.railwayEnabled,

        pluginPath: omaframes.pluginPath,
        managedByHyprpm: omaframes.managedByHyprpm,
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
    contentHeight: popup.fittedContentHeight(content.implicitHeight, Style.space(760))

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
        else if (text === "f") omaframes.togglePack("fireflies")
        else if (text === "i") omaframes.togglePack("frost")
        else if (text === "t") omaframes.togglePack("railway")
      }

      Flickable {
        id: effectsScroll
        anchors.fill: parent
        contentWidth: width
        contentHeight: content.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

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
            id: hostToggle
            width: parent.width
            label: "Native host"
            description: omaframes.managedByHyprpm
              ? (omaframes.loaded ? "Disable the HyprPM compositor renderer" : "Enable the HyprPM compositor renderer")
              : (omaframes.loaded ? "Unload the development renderer" : "Load the development renderer")
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
            id: vinesToggle
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
            id: critterToggle
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

          Toggle {
            id: firefliesToggle
            width: parent.width
            label: "Fireflies"
            description: "Soft glowing lights drifting around window edges"
            checked: omaframes.firefliesEnabled
            hasCursor: root.cursorActive && root.selectedIndex === 3
            enabled: omaframes.loaded && !omaframes.busy
            opacity: enabled ? 1.0 : 0.52
            foreground: root.foreground
            fontFamily: root.fontFamily
            onHovered: function(on) { if (on) root.setCursor(3) }
            onClicked: omaframes.togglePack("fireflies")
          }

          Toggle {
            id: frostToggle
            width: parent.width
            label: "Frost"
            description: "Branching ice crystals growing from the corners"
            checked: omaframes.frostEnabled
            hasCursor: root.cursorActive && root.selectedIndex === 4
            enabled: omaframes.loaded && !omaframes.busy
            opacity: enabled ? 1.0 : 0.52
            foreground: root.foreground
            fontFamily: root.fontFamily
            onHovered: function(on) { if (on) root.setCursor(4) }
            onClicked: omaframes.togglePack("frost")
          }

          Toggle {
            id: railwayToggle
            width: parent.width
            label: "Railway"
            description: "Twin rails with a tiny train circling the window"
            checked: omaframes.railwayEnabled
            hasCursor: root.cursorActive && root.selectedIndex === 5
            enabled: omaframes.loaded && !omaframes.busy
            opacity: enabled ? 1.0 : 0.52
            foreground: root.foreground
            fontFamily: root.fontFamily
            onHovered: function(on) { if (on) root.setCursor(5) }
            onClicked: omaframes.togglePack("railway")
          }

          Text {
            visible: !omaframes.loaded
            width: parent.width
            text: "Enable the native host to choose individual effects. Install it first with HyprPM if needed; right-click the bar icon to enable or disable it quickly."
            color: root.dim
            font.family: root.fontFamily
            font.pixelSize: Style.font.caption
            wrapMode: Text.WordWrap
          }
        }
      }
    }
  }
}
