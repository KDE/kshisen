project( kshisen )

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)

set (QT_MIN_VERSION "5.2.0")

find_package(ECM 1.7.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Gui )
find_package(KF5 REQUIRED COMPONENTS CoreAddons DBusAddons XmlGui Config KIO DNSSD NewStuff Declarative DocTools )

include(FeatureSummary)
include(ECMAddAppIcon)
include(GenerateExportHeader)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)
include(KDEInstallDirs)
include(ECMInstallIcons)
include(KDECompilerSettings)
include(KDECMakeSettings)

find_package(KF5KMahjongglib REQUIRED)
find_package(KF5KDEGames 4.9.0 REQUIRED)
find_package(Phonon4Qt5 REQUIRED)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DTRANSLATION_DOMAIN="kshisen")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

add_subdirectory( src )
add_subdirectory( sounds )
add_subdirectory( doc )

install( PROGRAMS org.kde.kshisen.desktop DESTINATION ${KDE_INSTALL_APPDIR} )

ecm_install_icons( ICONS
                   128-apps-kshisen.png
                   64-apps-kshisen.png
                   48-apps-kshisen.png
                   32-apps-kshisen.png
                   22-apps-kshisen.png
                   16-apps-kshisen.png
                   DESTINATION  ${KDE_INSTALL_ICONDIR}
                   THEME hicolor   )

########### Display Summary ###############
feature_summary(WHAT ALL
                INCLUDE_QUIET_PACKAGES
                FATAL_ON_MISSING_REQUIRED_PACKAGES)

