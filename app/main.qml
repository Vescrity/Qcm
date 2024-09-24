import QtCore
import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

import Qcm.App as QA
import Qcm.Service.Ncm as QNcm
import Qcm.Material as MD

ApplicationWindow {
    id: win

    // load QA
    readonly property alias snake: m_snake
    property int windowClass: MD.Enum.WindowClassMedium
    property Action barAction: Action {
        icon.name: MD.Token.icon.menu
        text: qsTr('menu')
        onTriggered: {
            QA.Action.open_drawer();
        }
    }

    MD.MatProp.backgroundColor: MD.MatProp.color.background
    MD.MatProp.textColor: MD.MatProp.color.getOn(MD.MatProp.backgroundColor)

    color: MD.MatProp.backgroundColor
    height: 600
    visible: true
    width: 900

    onWidthChanged: {
        windowClass = MD.Token.window_class.select_type(width);
    }

    function back() {
        if (win_stack.currentItem.canBack) {
            win_stack.currentItem.back();
        } else {
            console.log("back to quit");
        }
    }

    StackView {
        id: win_stack
        anchors.fill: parent

        initialItem: Item {
            BusyIndicator {
                anchors.centerIn: parent
                running: QA.Global.userModel.checkResult.status === QA.enums.Querying
            }
        }

        Component.onCompleted: {
            QA.Global.userModel.check_user();
        }

        Connections {
            function onStatusChanged() {
                if (target.status === QA.enums.Error) {
                    win_stack.replace(win_stack.currentItem, comp_retry, {
                        text: target.error,
                        retryCallback: () => {
                            target.query();
                        }
                    });
                } else if (target.status === QA.enums.Finished) {
                    if (QA.Global.userModel.activeUser) {
                        win_stack.replace(win_stack.currentItem, comp_main);
                    } else {
                        win_stack.replace(win_stack.currentItem, comp_login);
                    }
                }
            }

            target: QA.Global.userModel.checkResult
        }
    }

    QA.SnakeView {
        /*
        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(Math.max(implicitWidth, 350), parent.width)
        */
        id: m_snake
        anchors.fill: parent
    }

    QA.PagePopup {
        id: queue_popup
        source: 'qrc:/Qcm/App/qml/page/PlayQueuePage.qml'

        Connections {
            target: QA.Action
            function onPopup_special(str) {
                if (str === QA.enums.SRQueue)
                    queue_popup.open();
            }
        }
    }

    Timer {
        running: !win.active
        interval: 10 * 1000
        onTriggered: {
            QA.App.releaseResources(win);
        }
    }

    Settings {
        property alias height: win.height
        property alias width: win.width
        property alias x: win.x
        property alias y: win.y

        category: 'window'
    }

    Shortcut {
        sequences: [StandardKey.Back, StandardKey.Cancel, StandardKey.Close]
        onActivated: win.back()
    }
    Shortcut {
        sequence: StandardKey.Quit
        context: Qt.ApplicationShortcut
        onActivated: Qt.quit()
    }

    Connections {
        target: QA.Action
        function onToast(text, duration, flags, action) {
            win.snake.show2(text, duration, flags, action);
        }
    }
    Connections {
        function onInstanceStarted() {
            win.raise();
            win.requestActivate();
        }

        target: QA.App
    }

    Component.onCompleted: {
        QA.Global.main_win = win;
    }

    Component {
        id: comp_main
        StackView {
            id: sv_main

            property var playing_page: {
                const page = MD.Util.create_item(comp_playing, {}, sv_main);
                page.visible = false;
                playing_page = page;
            }

            clip: true

            initialItem: QA.MainPage {}

            property bool canBack: currentItem.canBack || depth > 1
            function back() {
                if (currentItem.canBack) {
                    this.currentItem.back();
                } else {
                    this.pop(null);
                }
            }
            Component.onCompleted: {
                console.error("win:", Window.window);
            }

            Component.onDestruction: {
                playing_page.destroy();
            }

            Connections {
                function onRoute_special(name) {
                    if (name === 'main')
                        sv_main.pop(null);
                    else if (name === 'playing')
                        sv_main.push(sv_main.playing_page);
                }

                target: QA.Action
            }
            Component {
                id: comp_playing
                QA.PlayingPage {}
            }
        }
    }
    Component {
        id: comp_login
        QA.LoginPage {}
    }
    Component {
        id: comp_retry
        QA.RetryPage {}
    }
}
