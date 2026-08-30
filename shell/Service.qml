import QtQuick
import Quickshell.Io

Item {
  id: root

  property var settings: ({})

  property bool reachable: false
  property bool loaded: false
  property bool vinesEnabled: false
  property bool critterEnabled: false
  property bool refreshing: false
  property string lastError: ""
  property string actionStatus: ""

  property string _pluginListOutput: ""
  property string _pluginListError: ""
  property string _vinesOutput: ""
  property string _vinesError: ""
  property string _critterOutput: ""
  property string _critterError: ""
  property string _actionOutput: ""
  property string _actionError: ""
  property string _pendingAction: ""
  property string _pendingPack: ""

  readonly property bool busy: actionProc.running || pluginListProc.running
    || vinesOptionProc.running || critterOptionProc.running
  readonly property int activeCount: (loaded && vinesEnabled ? 1 : 0)
    + (loaded && critterEnabled ? 1 : 0)
  readonly property bool anyActive: activeCount > 0
  readonly property bool pluginAvailable: pluginPath !== ""

  readonly property string pluginPath: {
    var configured = settings && settings.pluginPath !== undefined
      ? String(settings.pluginPath).trim()
      : ""
    return configured !== "" ? configured : localPath(Qt.resolvedUrl("../native/omaframes-native.so"))
  }

  function localPath(url) {
    var value = String(url || "")
    if (value.indexOf("file://") === 0) value = value.substring(7)
    try { return decodeURIComponent(value) } catch (e) { return value }
  }

  function friendlyError(raw, fallback) {
    var text = String(raw || "").replace(/\s+/g, " ").trim()
    if (text.indexOf("Couldn't set socket timeout") >= 0)
      return "Hyprland is not reachable from the shell."
    return text || fallback
  }

  function optionEnabled(raw) {
    try {
      var value = JSON.parse(String(raw || "{}"))
      if (value.bool !== undefined) return value.bool === true
      if (value.int !== undefined) return Number(value.int) !== 0
      if (value.value !== undefined) return value.value === true || Number(value.value) !== 0
    } catch (e) {}
    return false
  }

  function refresh() {
    if (busy) return
    refreshing = true
    _pluginListOutput = ""
    _pluginListError = ""
    pluginListProc.running = true
  }

  function applyPluginList(raw) {
    var plugins
    try {
      plugins = JSON.parse(String(raw || "[]"))
    } catch (e) {
      reachable = false
      refreshing = false
      lastError = friendlyError(_pluginListError, "Could not read Hyprland's plugin list.")
      return
    }

    reachable = true
    loaded = false
    for (var i = 0; i < plugins.length; i++) {
      if (String(plugins[i].name || "") === "omaframes") {
        loaded = true
        break
      }
    }

    if (!loaded) {
      vinesEnabled = false
      critterEnabled = false
      refreshing = false
      if (_pendingAction === "") actionStatus = ""
      lastError = ""
      return
    }

    _vinesOutput = ""
    _vinesError = ""
    _critterOutput = ""
    _critterError = ""
    vinesOptionProc.running = true
    critterOptionProc.running = true
  }

  function finishOptionRefresh() {
    if (vinesOptionProc.running || critterOptionProc.running) return
    refreshing = false
    if (_vinesOutput !== "") vinesEnabled = optionEnabled(_vinesOutput)
    if (_critterOutput !== "") critterEnabled = optionEnabled(_critterOutput)

    var error = _vinesError !== "" ? _vinesError : _critterError
    lastError = error === "" ? "" : friendlyError(error, "Could not read an OmaFrames setting.")
    if (_pendingAction === "") actionStatus = ""
  }

  function toggleHost() {
    if (actionProc.running) return
    if (pluginPath === "" || pluginPath.charAt(0) !== "/") {
      lastError = "Set an absolute path to omaframes-native.so in the widget settings."
      return
    }

    _pendingAction = loaded ? "unload" : "load"
    _pendingPack = ""
    _actionOutput = ""
    _actionError = ""
    lastError = ""
    actionStatus = loaded ? "Unloading OmaFrames…" : "Loading OmaFrames…"
    actionProc.command = ["hyprctl", "plugin", _pendingAction, pluginPath]
    actionProc.running = true
  }

  function setPack(pack, enabled) {
    if (!loaded || actionProc.running) return
    if (pack !== "vines" && pack !== "critter") return

    _pendingAction = "set"
    _pendingPack = pack
    _actionOutput = ""
    _actionError = ""
    lastError = ""
    actionStatus = (enabled ? "Enabling " : "Disabling ")
      + (pack === "vines" ? "Vines…" : "OmaCritter…")

    // Hyprland 0.56 exposes transient plugin values through its Lua config API.
    var lua = "hl.config({ plugin = { omaframes = { " + pack
      + " = { enabled = " + (enabled ? "true" : "false") + " } } } })"
    actionProc.command = ["hyprctl", "eval", lua]
    actionProc.running = true
  }

  function togglePack(pack) {
    if (pack === "vines") setPack(pack, !vinesEnabled)
    else if (pack === "critter") setPack(pack, !critterEnabled)
  }

  Timer {
    interval: 2500
    repeat: true
    running: true
    triggeredOnStart: true
    onTriggered: root.refresh()
  }

  Timer {
    id: actionSettleTimer
    interval: 180
    repeat: false
    onTriggered: root.refresh()
  }

  Process {
    id: pluginListProc
    command: ["hyprctl", "-j", "plugin", "list"]
    stdout: StdioCollector {
      waitForEnd: true
      onStreamFinished: {
        root._pluginListOutput = String(text || "")
        root.applyPluginList(root._pluginListOutput)
      }
    }
    stderr: StdioCollector {
      waitForEnd: true
      onStreamFinished: root._pluginListError = String(text || "").trim()
    }
    onExited: function(exitCode) {
      if (exitCode !== 0 && root._pluginListOutput === "") {
        root.reachable = false
        root.refreshing = false
        root.lastError = root.friendlyError(root._pluginListError, "Could not reach Hyprland.")
      }
    }
  }

  Process {
    id: vinesOptionProc
    command: ["hyprctl", "-j", "getoption", "plugin:omaframes:vines:enabled"]
    stdout: StdioCollector {
      waitForEnd: true
      onStreamFinished: root._vinesOutput = String(text || "")
    }
    stderr: StdioCollector {
      waitForEnd: true
      onStreamFinished: root._vinesError = String(text || "").trim()
    }
    onExited: function(exitCode) { Qt.callLater(root.finishOptionRefresh) }
  }

  Process {
    id: critterOptionProc
    command: ["hyprctl", "-j", "getoption", "plugin:omaframes:critter:enabled"]
    stdout: StdioCollector {
      waitForEnd: true
      onStreamFinished: root._critterOutput = String(text || "")
    }
    stderr: StdioCollector {
      waitForEnd: true
      onStreamFinished: root._critterError = String(text || "").trim()
    }
    onExited: function(exitCode) { Qt.callLater(root.finishOptionRefresh) }
  }

  Process {
    id: actionProc
    stdout: StdioCollector {
      waitForEnd: true
      onStreamFinished: root._actionOutput = String(text || "").trim()
    }
    stderr: StdioCollector {
      waitForEnd: true
      onStreamFinished: root._actionError = String(text || "").trim()
    }
    onExited: function(exitCode) {
      var action = root._pendingAction
      var pack = root._pendingPack
      root._pendingAction = ""
      root._pendingPack = ""

      if (exitCode !== 0 || root._actionError !== "") {
        root.lastError = root.friendlyError(root._actionError || root._actionOutput,
                                            "The OmaFrames action failed.")
        root.actionStatus = ""
      } else if (action === "load") {
        root.actionStatus = "OmaFrames loaded"
      } else if (action === "unload") {
        root.actionStatus = "OmaFrames unloaded"
      } else if (action === "set") {
        root.actionStatus = (pack === "vines" ? "Vines" : "OmaCritter") + " updated"
      }
      actionSettleTimer.restart()
    }
  }
}
