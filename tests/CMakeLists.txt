# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT

ecm_add_test(MainWindowTest.cpp
    TEST_NAME MainWindowTest
    LINK_LIBRARIES Qt6::Test Qt6::Gui Qt6::Qml Qt6::Quick
)

target_compile_definitions(MainWindowTest PRIVATE
    CROSSTALK_MAIN_QML="${PROJECT_SOURCE_DIR}/src/qml/Main.qml"
)

# Tests run with no display at all, the same in CI and inside the Flatpak
# build sandbox, so they never need a Wayland or X11 socket.
set_tests_properties(MainWindowTest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
