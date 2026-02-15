# FastPay — Qt for Android (and desktop)
QT       += core gui widgets network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# OpenSSL linking
unix:!android {
    LIBS += -lssl -lcrypto
}

# Qt for Android: custom package dir
android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    # Android ABIs - uncomment for multi-ABI builds (slower)
    # ANDROID_ABIS = arm64-v8a armeabi-v7a
    
    # OpenSSL for Android - Qt provides prebuilt binaries
    # Make sure to install "OpenSSL for Android" from Qt Maintenance Tool
    contains(ANDROID_TARGET_ARCH, arm64-v8a) {
        ANDROID_EXTRA_LIBS += \
            $$[QT_INSTALL_PREFIX]/lib/libssl_3.so \
            $$[QT_INSTALL_PREFIX]/lib/libcrypto_3.so
    }
}

TARGET = FastPayQt
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    transactionengine.cpp \
    ultrasoundhelper.cpp \
    transactionhistory.cpp \
    pindialog.cpp \
    Crypto/AES.cpp \
    Crypto/ECDSA.cpp \
    Crypto/RSA.cpp \
    Crypto/Ultrasound.cpp \
    Crypto/transaction.cpp

HEADERS += \
    mainwindow.h \
    transactionengine.h \
    ultrasoundhelper.h \
    transactionhistory.h \
    pindialog.h \
    server_config.h \
    Crypto/AES.h \
    Crypto/CryptoHandler.h \
    Crypto/DigitalSignature.h \
    Crypto/Ultrasound.h

INCLUDEPATH += $$PWD/Crypto

RESOURCES += resources.qrc

android: INSTALLS += target
!android: CONFIG += console
!android: CONFIG -= app_bundle
