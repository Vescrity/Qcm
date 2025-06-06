import QtQuick

import Qcm.App as QA
import Qcm.Material as MD

MD.Action {
    id: root
    icon.name: MD.Token.icon.queue
    text: qsTr('add to mix')
    required property QA.item_id songId
    onTriggered: {
        MD.Util.showPopup('qrc:/Qcm/App/qml/dialog/AddToMixDialog.qml', {
            "songId": root.songId
        }, QA.Global.main_win.Overlay.overlay);
    }
}
