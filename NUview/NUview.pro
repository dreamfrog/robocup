QT += network \
    opengl
macx { 
    # Mac Specific Includes
    QMAKE_LFLAGS += -F/System/Library/Frameworks/CoreFoundation.framework/
    LIBS += -framework \
        CoreFoundation \
        -L \
        /usr/lib/libz.dylib
}
win32 {
INCLUDEPATH += 'C:/Program Files/boost/'

# Opencv library
#INCLUDEPATH += "C:\Program Files\OpenCV\cv\include" "C:\Program Files\OpenCV\cvaux\include" "C:\Program Files\OpenCV\cxcore\include" "C:\Program Files\OpenCV\otherlibs\highgui"
#LIBS += -L"C:\Program Files\OpenCV\lib" -lcv -lcvaux -lhighgui -lcxcore
# Input
#HEADERS += "C:\Program Files\OpenCV\cv\include\cv.h" "C:\Program Files\OpenCV\otherlibs\highgui\highgui.h"
}
INCLUDEPATH += ../
INCLUDEPATH += 'C:\Qt\2010.01\qt\src\3rdparty\zlib'
HEADERS += ui_mainwindow.h \
    mainwindow.h \
    connectionwidget.h \
    ColorModelConversions.h \
    classificationwidget.h \
    ../Vision/ClassificationColours.h \
    ../Tools/FileFormats/NUbotImage.h \
    ../Vision/Vision.h \
    ../Tools/FileFormats/LUTTools.h \
    virtualnubot.h \
    ../Tools/Image/BresenhamLine.h \
    ../Tools/Math/Vector2.h \
    ../Tools/Math/Line.h \
    ../Kinematics/Horizon.h \
    openglmanager.h \
    GLDisplay.h \
    ../Tools/Image/NUimage.h \
    ../Tools/Image/ClassifiedImage.h \
    ../Vision/ClassifiedSection.h \
    ../Vision/ScanLine.h \
    ../Vision/TransitionSegment.h \
    LayerSelectionWidget.h \
    locWmGlDisplay.h \
    ../Vision/LineDetection.h \
    ../Tools/Math/LSFittedLine.h \
    ../Vision/RobotCandidate.h \
    ../Tools/Math/Vector3.h \
    ../Vision/Object.h \
    ../Vision/FieldObjects/StationaryObject.h \
    ../Vision/FieldObjects/Self.h \
    ../Vision/FieldObjects/Object.h \
    ../Vision/FieldObjects/MobileObject.h \
    ../Vision/FieldObjects/AmbiguousObject.h \
    ../Vision/FieldObjects/FieldObjects.h \
    ../Vision/ObjectCandidate.h \
    ../Localisation/WMPoint.h \
    ../Localisation/WMLine.h \
    ../Localisation/sphere.h \
    ../Localisation/cylinder.h \
    ../Localisation/cameramatrix.h \
    ../Tools/Math/matrix.h \
    localisationwidget.h \
    ../Vision/Ball.h \
    ../Vision/CircleFitting.h
SOURCES += mainwindow.cpp \
    main.cpp \
    connectionwidget.cpp \
    classificationwidget.cpp \
    ../Tools/FileFormats/NUbotImage.cpp \
    ../Vision/Vision.cpp \
    ../Tools/FileFormats/LUTTools.cpp \
    virtualnubot.cpp \
    ../Tools/Image/BresenhamLine.cpp \
    ../Tools/Math/Line.cpp \
    ../Kinematics/Horizon.cpp \
    openglmanager.cpp \
    GLDisplay.cpp \
    ../Tools/Image/NUimage.cpp \
    ../Tools/Image/ClassifiedImage.cpp \
    ../Vision/ClassifiedSection.cpp \
    ../Vision/ScanLine.cpp \
    ../Vision/TransitionSegment.cpp \
    LayerSelectionWidget.cpp \
    locWmGlDisplay.cpp \
    ../Vision/RobotCandidate.cpp \
    ../Vision/ObjectCandidate.cpp \
    ../Vision/LineDetection.cpp \
    ../Tools/Math/LSFittedLine.cpp \
    ../Vision/FieldObjects/StationaryObject.cpp \
    ../Vision/FieldObjects/Self.cpp \
    ../Vision/FieldObjects/Object.cpp \
    ../Vision/FieldObjects/MobileObject.cpp \
    ../Vision/FieldObjects/AmbiguousObject.cpp \
    ../Vision/FieldObjects/FieldObjects.cpp \
    ../Localisation/WMPoint.cpp \
    ../Localisation/WMLine.cpp \
    ../Localisation/sphere.cpp \
    ../Localisation/cylinder.cpp \
    ../Localisation/cameramatrix.cpp \
    ../Tools/Math/matrix.cpp \
    localisationwidget.cpp \
    ../Vision/Ball.cpp \
    ../Vision/CircleFitting.cpp
RESOURCES = textures.qrc
RESOURCES += icons.qrc