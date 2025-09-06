cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "25")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KSHISEN_BASE_VERSION once new features are added
set(KSHISEN_BASE_VERSION "1.11")
set(KSHISEN_VERSION "${KSHISEN_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(kshisen VERSION ${KSHISEN_VERSION})

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")

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

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Gui
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Config
    IconThemes
    ConfigWidgets
    Crash
    DBusAddons
    DocTools
    I18n
    WidgetsAddons
    XmlGui

)

find_package(KMahjongglib6 6.0.0 REQUIRED)
find_package(KDEGames6 6.0.0 REQUIRED)

ecm_set_disabled_deprecation_versions(
    QT 6.9.0
    KF 6.18.0
    KDEGAMES 6.0
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
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
