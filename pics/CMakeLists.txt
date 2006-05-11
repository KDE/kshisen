kde4_header()




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kshisen/ FILES tileset.png kshisen_bgnd.png )

kde4_footer()



#original Makefile.am contents follow:

#
#pics_DATA = tileset.png kshisen_bgnd.png
#picsdir = $(kde_datadir)/kshisen/
#
#EXTRA_DIST =  $(pics_DATA)
