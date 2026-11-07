# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS WebEngineWidgets Widgets)

qt_add_executable(PageCheck PageCheck.cpp)
target_link_libraries(PageCheck PRIVATE crossparty_core Qt6::WebEngineWidgets Qt6::Widgets)
