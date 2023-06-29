cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "23")
set(RELEASE_SERVICE_VERSION_MINOR "04")
set(RELEASE_SERVICE_VERSION_MICRO "3")
set(RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KSHISEN_BASE_VERSION once new features are added
set(KSHISEN_BASE_VERSION "1.11")
set(KSHISEN_VERSION "${KSHISEN_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(kshisen VERSION ${KSHISEN_VERSION})

set(QT_MIN_VERSION "5.15.0")
set(KF_MIN_VERSION "5.91.0")

# Found this somewhere but cannot re-find it now ... but want to investigate later, so keeping this here for now.
#set(CXX_CLANG_TIDY ON)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

if(ENABLE_CLAZY)
    set(CMAKE_CXX_COMPILER clazy)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)
include(ECMDeprecationSettings)
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Gui)

find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Config
    Crash
    DBusAddons
    DocTools
    XmlGui
)

find_package(KF5KMahjongglib 5.1.0 REQUIRED)
find_package(KF5KDEGames 7.3.0 REQUIRED)

ecm_set_disabled_deprecation_versions(
    QT 6.4
    KF 5.103
)

add_subdirectory(src)
add_subdirectory(icons)
add_subdirectory(sounds)
add_subdirectory(doc)

install(PROGRAMS org.kde.kshisen.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kshisen.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ki18n_install(po)
kdoctools_install(po)

########### Display Summary ###############
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
