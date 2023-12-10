import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import QtQuick.Templates as T
import Qcm.Material as MD

T.Switch {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding, implicitIndicatorHeight + topPadding + bottomPadding)

    padding: 8
    spacing: 8

    icon.width: 16
    icon.height: 16
    icon.color: checked ? (Material.theme === Material.Light ? enabled ? Qt.darker(Material.switchCheckedTrackColor, 1.8) : Material.switchDisabledCheckedIconColor : enabled ? Material.primaryTextColor : Material.switchDisabledCheckedIconColor) : enabled ? Material.switchUncheckedTrackColor : Material.switchDisabledUncheckedIconColor

    indicator: Rectangle {
        id: indicator
        width: 52
        height: 32
        radius: height / 2
        y: parent.height / 2 - height / 2
        color: item_state.backgroundColor
        border.width: 2
        border.color: MD.Token.color.outline

        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }
        Behavior on border.color {
            ColorAnimation {
                duration: 200
            }
        }

        Rectangle {
            id: handle
            x: Math.max(offset, Math.min(parent.width - offset - width, control.visualPosition * parent.width - (width / 2)))
            y: (parent.height - height) / 2
            // We use scale to allow us to enlarge the circle from the center,
            // as using width/height will cause it to jump due to the position x/y bindings.
            // However, a large enough scale on certain displays will show the triangles
            // that make up the circle, so instead we make sure that the circle is always
            // its largest size so that more triangles are used, and downscale instead.
            width: normalSize * largestScale
            height: normalSize * largestScale
            radius: width / 2
            color: item_state.handleColor
            scale: normalSize / largestSize

            readonly property int offset: 2
            readonly property real normalSize: item_state.handleSize
            readonly property real largestSize: 28
            readonly property real largestScale: largestSize / normalSize
            readonly property bool hasIcon: control.icon.name.length > 0 || control.icon.source.toString().length > 0

            Behavior on x {
                enabled: !control.pressed
                SmoothedAnimation {
                    duration: 300
                }
            }

            Behavior on scale {
                NumberAnimation {
                    duration: 100
                }
            }

            Behavior on color {
                ColorAnimation {
                    duration: 200
                }
            }

            /*
            IconImage {
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                name: control.icon.name
                source: control.icon.source
                sourceSize: Qt.size(control.icon.width, control.icon.height)
                color: control.icon.color
                visible: handle.hasIcon
            }
*/

        }
        Ripple {
            x: handle.x + handle.width / 2 - width / 2
            y: handle.y + handle.height / 2 - height / 2
            width: 28
            height: 28
            pressed: control.pressed
            active: enabled && (control.down || control.visualFocus || control.hovered)
            color: item_state.stateLayerColor
        }
    }

    contentItem: Text {
        leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
        rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0

        text: control.text
        font: control.font
        color: control.enabled ? control.Material.foreground : control.Material.hintTextColor
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    MD.State {
        id: item_state
        visible: false

        elevation: MD.Token.elevation.level1
        textColor: control.checked ? MD.Token.color.on_primary_container : MD.Token.color.surface_container_highest
        backgroundColor: control.checked ? MD.Token.color.primary : MD.Token.color.surface_container_highest
        stateLayerColor: "#00000000"
        property color handleColor: control.checked ? MD.Token.color.on_primary : MD.Token.color.outline
        property int handleSize: control.checked ? 24 : 16

        states: [
            State {
                name: "Disabled"
                when: !enabled
                PropertyChanges {
                    item_state.elevation: MD.Token.elevation.level0
                    item_state.textColor: MD.Token.color.on_surface
                    item_state.backgroundColor: MD.Token.color.on_surface
                    control.contentItem.opacity: 0.38
                    control.background.opacity: 0.12
                }
            },
            State {
                name: "Hovered"
                when: control.enabled && control.hovered && !control.down
                PropertyChanges {
                    item_state.textColor: control.checked ? MD.Token.color.on_primary_container : MD.Token.color.surface_container_highest
                    item_state.backgroundColor: control.checked ? MD.Token.color.primary : MD.Token.color.surface_container_highest
                    item_state.handleColor: control.checked ? MD.Token.color.primary_container : MD.Token.color.on_surface_variant
                }
                PropertyChanges {
                    restoreEntryValues: false
                    item_state.stateLayerColor: {
                        const c = control.checked ? MD.Token.color.primary : MD.Token.color.on_surface;
                        return MD.Util.transparent(c, MD.Token.state.hover.state_layer_opacity);
                    }
                }
            },
            State {
                name: "Pressed"
                when: control.enabled && control.down
                PropertyChanges {
                    item_state.textColor: control.checked ? MD.Token.color.on_primary_container : MD.Token.color.surface_container_highest
                    item_state.backgroundColor: control.checked ? MD.Token.color.primary : MD.Token.color.surface_container_highest
                    item_state.handleColor: control.checked ? MD.Token.color.primary_container : MD.Token.color.on_surface_variant
                    item_state.handleSize: 28
                }
                PropertyChanges {
                    restoreEntryValues: false
                    item_state.stateLayerColor: {
                        const c = control.checked ? MD.Token.color.primary : MD.Token.color.on_surface;
                        return MD.Util.transparent(c, MD.Token.state.pressed.state_layer_opacity);
                    }
                }
            }
        ]
    }
}
