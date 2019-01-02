import QtQuick 2.2
import Sailfish.Silica 1.0

Item {
    id: hint

    anchors.fill: parent

    property bool swipeRight
    property bool hintEnabled
    property alias hintRunning: touchInteractionHint.running
    property alias text: label.text

    signal hintShown()

    function showHint() {
        touchInteractionHint.start()
    }

    onHintEnabledChanged: if (hint.hintEnabled) showHint();
    Component.onCompleted: if (hint.hintEnabled) showHint();

    InteractionHintLabel {
        id: label

        anchors.bottom: parent.bottom
        opacity: touchInteractionHint.running ? 1.0 : 0.0
        Behavior on opacity { FadeAnimator { duration: 1000 } }
    }

    TouchInteractionHint {
        id: touchInteractionHint

        direction: swipeRight ? TouchInteraction.Right : TouchInteraction.Left
        anchors.verticalCenter: parent.verticalCenter
        onRunningChanged: if (!running) hint.hintShown()
    }
}
