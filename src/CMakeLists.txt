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
    core/GameModeDetection.cpp
    core/GameModeDetection.h
    core/InviteWatch.cpp
    core/InviteWatch.h
    core/MicrophoneStreams.cpp
    core/MicrophoneStreams.h
    core/NavigationPolicy.cpp
    core/NavigationPolicy.h
    core/PermissionPolicy.cpp
    core/PermissionPolicy.h
    core/PortalRequest.cpp
    core/PortalRequest.h
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
    QML_FILES qml/BackgroundHolder.qml qml/CornerNotices.qml qml/HandheldBar.qml qml/Main.qml qml/OverlayWindow.qml qml/PartyPage.qml qml/SignOutDialog.qml qml/TopBar.qml qml/WindowEdges.qml
    SOURCES core/AppSettings.cpp core/AppSettings.h core/GameMode.cpp core/GameMode.h core/GlobalShortcuts.cpp core/GlobalShortcuts.h core/MicrophoneMute.cpp core/MicrophoneMute.h core/TrayIcon.cpp core/TrayIcon.h core/WebSession.cpp core/WebSession.h core/WindowExposure.cpp core/WindowExposure.h
    NO_IMPORT_SCAN
)

target_link_libraries(crossparty_ui PUBLIC
    crossparty_core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::WebEngineQuick
    Qt6::Widgets
    PkgConfig::Pulse
)

qt_add_executable(crossparty main.cpp core/SingleInstance.cpp core/SingleInstance.h)

target_compile_definitions(crossparty PRIVATE
    CROSSPARTY_APP_ID="${CROSSPARTY_APP_ID}"
    CROSSPARTY_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(crossparty PRIVATE
    crossparty_ui
    crossparty_uiplugin
    Qt6::DBus
    Qt6::QuickControls2
    Qt6::Widgets
)

# Qt is a shared library here (Fedora and the KDE Flatpak runtime), so QML
# plugins such as Kirigami load at runtime and must not be linked into the
# binary. Scanning would only produce "plugin will not be linked" warnings.
set_target_properties(crossparty PROPERTIES QT_QML_MODULE_NO_IMPORT_SCAN TRUE)

install(TARGETS crossparty ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
