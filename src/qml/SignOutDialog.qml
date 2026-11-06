// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import Crossparty

// The only pop-up the app has (docs/plan.md, Rules): Sign out and wipe cannot
// be undone, so it is confirmed first. Cancel is the default, so pressing
// Enter or A by habit never erases anything.
Kirigami.PromptDialog {
    id: signOutDialog

    title: qsTr("Sign out and wipe?")
    subtitle: qsTr("This signs you out of your Microsoft account in Crossparty and erases everything it has saved: your sign-in, cookies and page data. If you are in a party, you will leave it. Crossparty then closes; open it again to sign in.")
    standardButtons: Kirigami.Dialog.NoButton

    customFooterActions: [
        Kirigami.Action {
            text: qsTr("Sign Out and Wipe")
            icon.name: "edit-delete"
            onTriggered: WebSession.signOutAndWipe()
        },
        Kirigami.Action {
            id: cancelAction

            text: qsTr("Cancel")
            icon.name: "dialog-cancel"
            onTriggered: signOutDialog.close()
        }
    ]

    onOpened: {
        const cancelButton = signOutDialog.customFooterButton(cancelAction);
        if (cancelButton) {
            cancelButton.forceActiveFocus();
        }
    }
}
