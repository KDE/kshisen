cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "20")
set (RELEASE_SERVICE_VERSION_MINOR "12")
set (RELEASE_SERVICE_VERSION_MICRO "0")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KSHISEN_BASE_VERSION once new features are added
set(KSHISEN_BASE_VERSION "1.10")
set(KSHISEN_VERSION "${KSHISEN_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(kshisen VERSION ${KSHISEN_VERSION})

set (QT_MIN_VERSION "5.10.0")
set (KF5_MIN_VERSION "5.55.0")

set(CMAKE_CXX_STANDARD 14) # C++14...
set(CMAKE_CXX_STANDARD_REQUIRED ON) # ...is required...
set(CMAKE_CXX_EXTENSIONS OFF) # ...without compiler extensions like gnu++14

# Found this somewhere but cannot re-find it now ... but want to investigate later, so keeping this here for now.
#set(CXX_CLANG_TIDY ON)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

if(ENABLE_CLAZY)
    set(CMAKE_CXX_COMPILER clazy)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Gui)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    Crash
    DBusAddons
    DocTools
    XmlGui
)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)

find_package(KF5KMahjongglib REQUIRED)
find_package(KF5KDEGames 4.9.0 REQUIRED)
if (${KF5Config_VERSION} STRGREATER "5.56.0")
        add_definitions(-DQT_NO_FOREACH)
        MESSAGE(STATUS "compile without foreach")
endif()

add_definitions(
    -DQT_STRICT_ITERATORS
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    -DQT_NO_KEYWORDS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050A00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x053700
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

add_subdirectory(src)
add_subdirectory(icons)
add_subdirectory(sounds)
add_subdirectory(doc)

install(PROGRAMS org.kde.kshisen.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kshisen.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

if (${ECM_VERSION} STRGREATER "5.58.0")
    install(FILES kshisen.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES kshisen.categories  DESTINATION ${KDE_INSTALL_CONFDIR})
endif()

ki18n_install(po)
kdoctools_install(po)

########### Display Summary ###############
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
