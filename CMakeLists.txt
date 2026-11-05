# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.25)

project(Crossparty VERSION 0.0.1 LANGUAGES CXX)

# The one place the application ID is written down. Everything else (the
# desktop file name, the Wayland app ID, settings paths, keyring entries)
# is derived from it, so it must never change once anyone has installed the app.
set(CROSSPARTY_APP_ID "io.github.KreatureofKreation.Crossparty")

set(QT_MIN_VERSION "6.8")
set(KF_MIN_VERSION "6.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# KDE's standard build settings: install paths, strict compiler warnings and
# BUILD_TESTING (on by default).
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core DBus Gui Network Qml Quick QuickControls2 Test WebEngineCore WebEngineQuick Widgets)
# Encryption for the saved cookies (docs/security.md#sec-009).
find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto)
find_package(KF6Kirigami ${KF_MIN_VERSION} REQUIRED)

qt_standard_project_setup(REQUIRES ${QT_MIN_VERSION})

# clang-tidy and clazy read this file to see how each source file is compiled.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

install(FILES data/${CROSSPARTY_APP_ID}.desktop DESTINATION ${KDE_INSTALL_APPDIR})
