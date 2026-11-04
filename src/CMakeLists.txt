# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT

qt_add_executable(crosstalk main.cpp)

# NO_IMPORT_SCAN: Qt is a shared library here (Fedora and the KDE Flatpak
# runtime), so QML plugins such as Kirigami load at runtime and must not be
# linked into the binary. Scanning would only produce "plugin will not be
# linked" warnings.
qt_add_qml_module(crosstalk
    URI Crosstalk
    QML_FILES qml/Main.qml
    NO_IMPORT_SCAN
)

target_compile_definitions(crosstalk PRIVATE
    CROSSTALK_APP_ID="${CROSSTALK_APP_ID}"
    CROSSTALK_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(crosstalk PRIVATE
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

install(TARGETS crosstalk ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
