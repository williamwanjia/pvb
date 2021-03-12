#######################################
# project file for pvbrowser          #
# you can uncomment CONFIG += USE_VTK #
#######################################
CONFIG       += warn_on release
CONFIG      += USE_VTK
#DEFINES     += NO_QWT
DEFINES      += USE_GOOGLE_WEBKIT_FORK
#mobile devices without opengl
#QT           += opengl
QT           += printsupport multimedia uitools webenginewidgets widgets xml svg network printsupport

linux-g++-gles2 {
  DEFINES    += USE_MAEMO
  QT         -= opengl
}  
symbian:CONFIG += USE_SYMBIAN
USE_SYMBIAN {
  DEFINES    += USE_SYMBIAN
  DEFINES    += USE_MAEMO
  DEFINES    += NO_PRINTER
  TARGET.CAPABILITY = "NetworkServices ReadUserData WriteUserData"
  LIBS       += -lqwt.lib
  QT         -= opengl
}

macx:DEFINES += PVMAC
macx:DEFINES += unix
unix:!symbian:LIBS    += -ldl
!symbian:QMAKE_LFLAGS += -static-libgcc

HEADERS       = mainwindow.h \
                dlgopt.h \
                opt.h \
                tcputil.h \
                interpreter.h \
                pvserver.h \
                MyWidgets.h \
                MyTextBrowser_v5pc.h \
                mywebenginepage.h \
                qimagewidget.h \
                qdrawwidget.h \
                qwtwidgets.h \
                qwtplotwidget.h \
                dlgtextbrowser.h \
                webkit_ui_dlgtextbrowser.h \
                dlgmybrowser.h \
                pvdefine.h

SOURCES       = main.cpp \
                mainwindow.cpp \
                dlgopt.cpp \
                opt.cpp \
                tcputil.cpp \
                interpreter.cpp \
                MyWidgets.cpp \
                MyTextBrowser_v5pc.cpp \
                mywebenginepage.cpp \
                QDrawWidget.cpp \
                QImageWidget.cpp \
                qwtplotwidget.cpp \
                dlgtextbrowser.cpp \
                dlgmybrowser.cpp

message($$QMAKE_HOST.arch)

contains(QMAKE_HOST.arch, "i686") {
message("i686 -> USE_OPEN_GL")
DEFINES      += USE_OPEN_GL
HEADERS      += pvglwidget.h
SOURCES      += pvglwidget.cpp \
                gldecode.cpp
}
contains(QMAKE_HOST.arch, "x86_64") {
message("x86_64 -> USE_OPEN_GL")
DEFINES      += USE_OPEN_GL
HEADERS      += pvglwidget.h
SOURCES      += pvglwidget.cpp \
                gldecode.cpp
}

# FORMS        += dlgtextbrowser.ui
#               dlgmybrowser.ui

#INCLUDEPATH  += ../qwt/include
INCLUDEPATH  += ../qwt/src
symbian {

}
else {
LIBS         += ../qwt/lib/libqwt.a
}

### begin USE_VTK #############################################
USE_VTK {

DEFINES      += USE_VTK

HEADERS      += pvVtkTclWidget.h \
                QVTKWidget.h

SOURCES      += tkAppInit.cpp \
                pvVtkTclWidget.cpp \
                QVTKWidget.cpp

INCLUDEPATH  += /usr/include/tcl8.5

LIBS += -L/usr/local/lib
SHARED_LIB_FILES = $$files(/usr/local/lib/libvtk*.so)
for(FILE, SHARED_LIB_FILES) {
        BASENAME = $$basename(FILE)
        BASENAME = $$replace(BASENAME,libvtk,vtk)
        LIBS += -l$$replace(BASENAME,8.1.so,8.1)
    }
INCLUDEPATH += /usr/local/include/vtk-8.1

LIBS += /usr/lib/x86_64-linux-gnu/libtcl8.5.so
LIBS += /usr/lib/x86_64-linux-gnu/libX11.so.6

}
### end USE_VTK ###############################################
android-g++ {
  DEFINES    += USE_ANDROID
  DEFINES    += USE_MAEMO
  QT         -= opengl
  DEFINES    += NO_WEBKIT
  QT         -= webkit
  QT         -= webkitwidgets
  QT         -= webenginewidgets
  HEADERS    -= dlgmybrowser.h
  SOURCES    -= dlgmybrowser.cpp
}

RESOURCES     = pvbrowser.qrc
TARGET        = pvbrowser

# install
target.path   = /usr/local/bin
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS pvbrowser.pro images
sources.path  = /opt/pvb/pvbrowser
INSTALLS     += target sources
