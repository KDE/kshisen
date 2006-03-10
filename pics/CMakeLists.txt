kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kshisen/ FILES tileset.png kshisen_bgnd.png )

kde4_footer()



#original Makefile.am contents follow:

#
#pics_DATA = tileset.png kshisen_bgnd.png
#picsdir = $(kde_datadir)/kshisen/
#
#EXTRA_DIST =  $(pics_DATA)
