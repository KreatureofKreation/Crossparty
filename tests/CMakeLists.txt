# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT

# The security policies: one test per policy, run on every build.
foreach(policyTest SiteAllowlistTest NavigationPolicyTest PermissionPolicyTest)
    ecm_add_test(${policyTest}.cpp
        TEST_NAME ${policyTest}
        LINK_LIBRARIES Qt6::Test crossparty_core
    )
endforeach()

ecm_add_test(MainWindowTest.cpp
    TEST_NAME MainWindowTest
    LINK_LIBRARIES Qt6::Test Qt6::Gui Qt6::Qml Qt6::Quick Qt6::WebEngineQuick crossparty_ui crossparty_uiplugin
)

# Same reason as for the app: QML plugins load at runtime (src/CMakeLists.txt).
set_target_properties(MainWindowTest PROPERTIES QT_QML_MODULE_NO_IMPORT_SCAN TRUE)

# Tests run with no display at all, the same in CI and inside the Flatpak
# build sandbox, so they never need a Wayland or X11 socket.
set_tests_properties(MainWindowTest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
