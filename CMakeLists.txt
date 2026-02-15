# FastPay — Qt for Android (and desktop)
cmake_minimum_required(VERSION 3.16)
project(FastPayQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find all required Qt modules including Multimedia for audio
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Multimedia)

# Find OpenSSL for cryptographic operations
find_package(OpenSSL REQUIRED)

qt_add_executable(FastPayQt
    main.cpp
    mainwindow.cpp
    transactionengine.cpp
    ultrasoundhelper.cpp
    transactionhistory.cpp
    pindialog.cpp
    # Crypto module files
    Crypto/AES.cpp
    Crypto/ECDSA.cpp
    Crypto/RSA.cpp
    Crypto/Ultrasound.cpp
    Crypto/transaction.cpp
)

# Include directories for headers
target_include_directories(FastPayQt PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Crypto
    ${OPENSSL_INCLUDE_DIR}
)

# Link all required libraries
target_link_libraries(FastPayQt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Add resources
qt_add_resources(FastPayQt "resources"
    PREFIX "/"
    FILES assets/logo.png
)

# Android-specific configuration
if(ANDROID)
    # Qt for Android: use our android/ package (manifest, Gradle, res)
    set_property(TARGET FastPayQt PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    
    # Set Android API levels
    set_property(TARGET FastPayQt PROPERTY QT_ANDROID_MIN_SDK_VERSION 24)
    set_property(TARGET FastPayQt PROPERTY QT_ANDROID_TARGET_SDK_VERSION 34)
    
    # Build for arm64-v8a (modern devices) - single ABI for faster builds
    # For multi-ABI support, use: "arm64-v8a;armeabi-v7a"
    set_property(TARGET FastPayQt PROPERTY QT_ANDROID_ABIS "arm64-v8a")
    
    # OpenSSL for Android - Qt provides prebuilt binaries
    # Make sure Qt's OpenSSL libs are included in the APK
    set(ANDROID_EXTRA_LIBS
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
    )
endif()
