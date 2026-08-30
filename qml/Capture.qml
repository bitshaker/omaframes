import QtQuick

Study {
    id: captureRoot

    automatedCapture: true
    capturePath: Qt.resolvedUrl("../captures/qml-study.png").toString().replace("file://", "")
    growthProgress: 0.93

    Timer {
        id: captureTimer
        interval: 700
        repeat: false
        running: true

        onTriggered: {
            const accepted = captureRoot.grabToImage(function(result) {
                const saved = result.saveToFile(captureRoot.capturePath)
                console.log(saved ? "Saved capture to " + captureRoot.capturePath : "Failed to save " + captureRoot.capturePath)
                Qt.quit()
            })

            if (!accepted) {
                console.log("The offscreen renderer did not accept the capture request")
                Qt.quit()
            }
        }
    }

}
