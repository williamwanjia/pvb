#######################################
# project file for pvbrowser          #
#######################################

CONFIG += static

# Uncomment the following lines to build a version with debugging consoles
#CONFIG += DEBUG
#DEFINES += DEBUG

DEBUG {
    CONFIG += console
}

CONFIG      += USE_VTK
DEFINES     += USE_VTK
DEFINES     += NO_QWT
DEFINES     += NO_WEBKIT

QT          += printsupport multimedia uitools widgets xml svg network

HEADERS       = mainwindow.h \
                dlgopt.h \
                opt.h \
                tcputil.h \
                interpreter.h \
                pvserver.h \
                MyWidgets.h \
                qimagewidget.h \
                qdrawwidget.h \
                qwtwidgets.h \
                qwtplotwidget.h \
                pvdefine.h

SOURCES       = main.cpp \
                mainwindow.cpp \
                dlgopt.cpp \
                opt.cpp \
                tcputil.cpp \
                interpreter.cpp \
                MyWidgets.cpp \
                QDrawWidget.cpp \
                QImageWidget.cpp \
                qwtplotwidget.cpp \

DEFINES      += USE_OPEN_GL
HEADERS      += pvglwidget.h
SOURCES      += pvglwidget.cpp \
                gldecode.cpp

INCLUDEPATH  += ../qwt/src
#LIBS         += ../qwt/lib/libqwt.a


### begin USE_VTK #############################################
USE_VTK {
DEFINES      -= UNICODE

DEFINES      += STATIC_BUILD
DEFINES      += GLEW_STATIC

HEADERS      += vtktcl_static_packages.h \
                vtktcl_static_prototypes.h \
                vtkTkAppInitConfigure.h  \
                pvVtkTclWidget.h \
                QVTKWidget.h                 

SOURCES      += tkAppinit.cpp \
                pvVtkTclWidget.cpp \
                QVTKWidget.cpp

LIBS += rc/tk_base.res.o

LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkWrappingTools-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkViewsQt-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkViewsInfovisTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkViewsInfovis-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkViewsContextIIDTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkViewsContext2D-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingVolumeOpenGLIITCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingVolumeOpenGL2-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingQtTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingQt-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingLabelTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingLabel-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingLODTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingLOD-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingImageTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingImage-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingContextOpenGLIITCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingContextOpenGL2-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkInteractionImageTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkInteractionImage-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingStencilTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingStencil-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingStatisticsTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingStatistics-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingMorphologicalTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingMorphological-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingMathTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingMath-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOVideoTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOVideo-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOTecplotTableTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOTecplotTable-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOParallelXMLTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOParallelXML-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOParallelTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOParallel-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkjsoncpp-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOPLYTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOPLY-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIONetCDFTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIONetCDF-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtknetcdfcpp-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOMovieTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOMovie-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOMINCTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOMINC-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOLSDynaTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOLSDyna-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOInfovisTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOInfovis-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtklibxml2-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOImportTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOImport-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOGeometryTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOGeometry-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOExportOpenGLIITCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOExportOpenGL2-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOExportTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOExport-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtklibharu-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingGLtoPSOpenGLIITCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingGL2PSOpenGL2-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkgl2ps-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOExodusTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOExodus-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkexoIIc-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkNetCDF-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOEnSightTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOEnSight-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOAMRTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOAMR-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkGeovisCoreTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkGeovisCore-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkproj4-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkViewsCoreTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkViewsCore-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkInteractionWidgetsTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkInteractionWidgets-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingVolumeTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingVolume-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingAnnotationTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingAnnotation-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingColorTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingColor-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkInfovisLayoutTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkInfovisLayout-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingHybridTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingHybrid-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOImageTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOImage-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkpng-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkjpeg-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkmetaio-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkGUISupportQtSQL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOSQLTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOSQL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtksqlite-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkGUISupportQt-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkInteractionStyleTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkInteractionStyle-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersVerdictTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersVerdict-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkverdict-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersTopologyTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersTopology-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersTextureTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersTexture-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersSelectionTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersSelection-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersSMPTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersSMP-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersProgrammableTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersProgrammable-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersPointsTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersPoints-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersParallelImagingTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersParallelImaging-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersParallelTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersParallel-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersModelingTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersModeling-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersImagingTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersImaging-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingGeneralTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingGeneral-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersHyperTreeTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersHyperTree-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersHybridTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersHybrid-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingSourcesTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingSources-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersGenericTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersGeneric-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersFlowPathsTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersFlowPaths-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersAMRTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersAMR-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkParallelCoreTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkParallelCore-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOXMLTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOXML-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingChemistryOpenGLIITCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkDomainsChemistryOpenGL2-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingOpenGLIITCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingOpenGL2-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkDomainsChemistryTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkDomainsChemistry-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOXMLParserTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOXMLParser-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkexpat-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOLegacyTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOLegacy-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOCoreTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkIOCore-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtklz4-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkDICOMParser-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkChartsCoreTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkChartsCore-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingContextIIDTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingContext2D-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingFreeTypeTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingFreeType-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkfreetype-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkzlib-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingTkTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingCoreTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkRenderingCore-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersSourcesTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersSources-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersGeometryTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersGeometry-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkInfovisCoreTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkInfovisCore-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersExtractionTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersExtraction-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersStatisticsTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersStatistics-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkalglib-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingFourierTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingFourier-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingCoreTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkImagingCore-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersGeneralTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersGeneral-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersCoreTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkFiltersCore-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonComputationalGeometryTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonComputationalGeometry-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonExecutionModelTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonExecutionModel-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonColorTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonColor-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonDataModelTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonDataModel-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonTransformsTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonTransforms-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonSystemTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonSystem-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonMiscTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonMisc-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonMathTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonMath-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonCoreTCL-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtkCommonCore-8.1.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libvtksys-8.1.a

LIBS += -lhdf5_hl
LIBS += -lhdf5

LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libGLEW.a

LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libtk86.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libtcl86.a
LIBS += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/lib/libtclstub86.a

INCLUDEPATH += /home/inge/Software/mxe/usr/i686-w64-mingw32.static/include/vtk-8.1

}
### end USE_VTK ###############################################

LIBS += -lopengl32
LIBS += -lglu32
LIBS += -lcomctl32

RESOURCES     = pvbrowser.qrc
TARGET        = pvbrowser

Release:DESTDIR = release_mingw
Release:OBJECTS_DIR = release_mingw/.obj
Release:MOC_DIR = release_mingw/.moc
Release:RCC_DIR = release_mingw/.rcc
Release:UI_DIR = release_mingw/.ui
