# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT

# The security policies and sign-in storage: plain C++ with no interface, so
# the unit tests can check every decision directly.
add_library(crossparty_core STATIC
    core/CookieCipher.cpp
    core/CookieCipher.h
    core/CookieFormat.cpp
    core/CookieFormat.h
    core/CookieVault.cpp
    core/CookieVault.h
    core/NavigationPolicy.cpp
    core/NavigationPolicy.h
    core/PermissionPolicy.cpp
    core/PermissionPolicy.h
    core/SecretPortal.cpp
    core/SecretPortal.h
    core/SettingsFile.cpp
    core/SettingsFile.h
    core/SiteAllowlist.cpp
    core/SiteAllowlist.h
)
target_include_directories(crossparty_core PUBLIC core)
target_link_libraries(crossparty_core PUBLIC Qt6::Core Qt6::DBus Qt6::Network Qt6::WebEngineCore PRIVATE OpenSSL::Crypto)

# The interface and the objects it talks to, as a QML module. It is a library
# so the tests can load exactly the same module as the app.
qt_add_library(crossparty_ui STATIC)

qt_add_qml_module(crossparty_ui
    URI Crossparty
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Crossparty
    QML_FILES qml/Main.qml qml/PartyPage.qml qml/TopBar.qml qml/WindowEdges.qml
    SOURCES core/AppSettings.cpp core/AppSettings.h core/WebSession.cpp core/WebSession.h
    NO_IMPORT_SCAN
)

target_link_libraries(crossparty_ui PUBLIC
    crossparty_core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::WebEngineQuick
)

qt_add_executable(crossparty main.cpp)

target_compile_definitions(crossparty PRIVATE
    CROSSPARTY_APP_ID="${CROSSPARTY_APP_ID}"
    CROSSPARTY_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(crossparty PRIVATE
    crossparty_ui
    crossparty_uiplugin
    Qt6::QuickControls2
)

# Qt is a shared library here (Fedora and the KDE Flatpak runtime), so QML
# plugins such as Kirigami load at runtime and must not be linked into the
# binary. Scanning would only produce "plugin will not be linked" warnings.
set_target_properties(crossparty PROPERTIES QT_QML_MODULE_NO_IMPORT_SCAN TRUE)

install(TARGETS crossparty ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
