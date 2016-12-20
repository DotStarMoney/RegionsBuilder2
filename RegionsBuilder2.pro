
QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RegionsBuilder2
TEMPLATE = app

#--------------- specifying opencv libs -----------------

CV_LIB_PATH = ../../Libs/x64/opencv/build/x64/vc12/lib/
CV_VERSION = 249
CV_LIBS += opencv_core        \
           opencv_features2d  \
           opencv_imgproc     \


#--------------------------------------------------------

CV_DEBUG =
debug {
    #CV_DEBUG += d
}

for(libName, CV_LIBS){
    LIBS += $${CV_LIB_PATH}$${libName}$${CV_VERSION}$${CV_DEBUG}.lib
}

INCLUDEPATH += ../../Includes \
               ../../Includes/opencv/includes

LIBS +=

SOURCES += main.cpp\
    myglwidget.cpp \
    glminimap.cpp \
    glzmap.cpp \
    qwidget_slicesheet.cpp \
    gldrawsurface.cpp \
    spritemachine.cpp \
    primitivemachine.cpp \
    slicesheet_preview.cpp \
    autoslicer.cpp \
    mainwin.cpp \
    regionedit.cpp \
    textmachine/fontface.cpp \
    textmachine/textmachine.cpp \
    textmachine/character.cpp \
    errlog.cpp \
    snapping_dialog.cpp \
    vectorutility.cpp

DEFINES += _CRT_SECURE_NO_WARNINGS

HEADERS  += \
    glzmap.h \
    glminimap.h \
    qwidget_slicesheet.h \
    gldrawsurface.h \
    spritemachine.h \
    primitivemachine.h \
    slicesheet_preview.h \
    autoslicer.h \
    mainwin.h \
    myglwidget.h \
    regionedit.h \
    textmachine/fontface.h \
    textmachine/textmachine.h \
    textmachine/character.h \
    errlog.h \
    udynamicarray.h \
    snapping_dialog.h \
    vectorutility.h \
    keybank.h

FORMS    += \
    qwidget_slicesheet.ui \
    mainwin.ui \
    regionedit.ui \
    snapping_dialog.ui

RESOURCES += \
    res.qrc \
    builtin_shaders.qrc

OTHER_FILES += \
    lineart.vert \
    lineart.frag \
    sprite.vert  \
    sprite.frag \
    text.frag \
    text.vert



