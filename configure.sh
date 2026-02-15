#!/bin/bash

# ============================================
# FastPay Configuration Wizard
# ============================================
# Simple wizard to configure server URL and build settings

echo "=========================================="
echo "  FastPay Configuration Wizard"
echo "=========================================="
echo ""
echo "This wizard will help you configure FastPay before building."
echo ""

# ============================================
# Server URL Configuration
# ============================================

echo "Step 1: Server Configuration"
echo "----------------------------"
echo ""
echo "Where is your Python backend server hosted?"
echo ""
echo "Examples:"
echo "  - https://fastpay-api.onrender.com"
echo "  - https://fastpay-production.up.railway.app"
echo "  - https://api.yourcompany.com"
echo ""
read -p "Enter your production server URL: " SERVER_URL

# Validate URL
if [[ ! "$SERVER_URL" =~ ^https?:// ]]; then
    echo "⚠️  Warning: URL should start with http:// or https://"
    echo "  You entered: $SERVER_URL"
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Update server_config.h
echo ""
echo "Updating server_config.h..."

cat > server_config.h << EOF
#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <QString>

// ============================================
// SERVER CONFIGURATION
// ============================================
// Auto-configured by setup wizard
// Last updated: $(date)
// ============================================

namespace ServerConfig {
    // Local testing server (for development only)
    const QString LOCAL_SERVER = "http://10.0.2.2:8000";  // Android emulator localhost
    
    // Production server URL
    const QString PRODUCTION_SERVER = "${SERVER_URL}";
    
    // Automatic selection based on build type
    #ifdef QT_DEBUG
        const QString CURRENT_SERVER = LOCAL_SERVER;
    #else
        const QString CURRENT_SERVER = PRODUCTION_SERVER;
    #endif
}

#endif // SERVER_CONFIG_H
EOF

echo "✅ Server URL configured: $SERVER_URL"
echo ""

# ============================================
# App Package Name (Optional)
# ============================================

echo "Step 2: App Package Name (Optional)"
echo "-----------------------------------"
echo ""
echo "Current package name: com.fastpay.transaction"
echo ""
read -p "Change package name? (y/N) " -n 1 -r
echo

if [[ $REPLY =~ ^[Yy]$ ]]; then
    read -p "Enter new package name (e.g., com.yourcompany.app): " PACKAGE_NAME
    
    # Validate package name
    if [[ "$PACKAGE_NAME" =~ ^[a-z][a-z0-9_]*(\.[a-z][a-z0-9_]*)+$ ]]; then
        echo "Updating package name..."
        
        # Update AndroidManifest.xml
        sed -i "s/com\.fastpay\.transaction/$PACKAGE_NAME/g" android/AndroidManifest.xml
        
        # Update build.gradle
        sed -i "s/com\.fastpay\.transaction/$PACKAGE_NAME/g" android/build.gradle
        
        echo "✅ Package name updated to: $PACKAGE_NAME"
    else
        echo "⚠️  Invalid package name format. Keeping default."
    fi
fi

echo ""

# ============================================
# Build Type Selection
# ============================================

echo "Step 3: Build Configuration"
echo "---------------------------"
echo ""
echo "Select build type:"
echo "  1) Debug   - For testing, includes debug symbols, connects to LOCAL_SERVER"
echo "  2) Release - For production, optimized, connects to PRODUCTION_SERVER"
echo ""
read -p "Choose (1 or 2): " -n 1 BUILD_CHOICE
echo
echo ""

case $BUILD_CHOICE in
    1)
        BUILD_TYPE="Debug"
        echo "✅ Selected: Debug build"
        echo "   Server: LOCAL_SERVER (http://10.0.2.2:8000)"
        ;;
    2)
        BUILD_TYPE="Release"
        echo "✅ Selected: Release build"
        echo "   Server: PRODUCTION_SERVER ($SERVER_URL)"
        ;;
    *)
        BUILD_TYPE="Debug"
        echo "⚠️  Invalid choice. Defaulting to Debug"
        ;;
esac

# Update build script with selected build type
sed -i "s/BUILD_TYPE=\"Debug\"/BUILD_TYPE=\"$BUILD_TYPE\"/" build_android.sh 2>/dev/null || true

echo ""

# ============================================
# Qt Path Configuration
# ============================================

echo "Step 4: Qt Installation Path"
echo "----------------------------"
echo ""

DEFAULT_QT_PATH="$HOME/Qt/6.6.0/android_arm64_v8a"
echo "Default Qt path: $DEFAULT_QT_PATH"
echo ""

if [ ! -d "$DEFAULT_QT_PATH" ]; then
    echo "⚠️  Default Qt path not found"
    read -p "Enter your Qt for Android path: " QT_PATH
    
    if [ -d "$QT_PATH" ]; then
        # Update build script
        sed -i "s|QT_ANDROID_PATH=.*|QT_ANDROID_PATH=\"$QT_PATH\"|" build_android.sh
        echo "✅ Qt path updated"
    else
        echo "⚠️  Warning: Path does not exist. You may need to edit build_android.sh manually"
    fi
else
    echo "✅ Qt installation found at default location"
fi

echo ""

# ============================================
# Summary
# ============================================

echo "=========================================="
echo "  Configuration Summary"
echo "=========================================="
echo ""
echo "Server URL (Production): $SERVER_URL"
echo "Build Type: $BUILD_TYPE"
echo ""
echo "Configuration complete! ✅"
echo ""
echo "=========================================="
echo "  Next Steps"
echo "=========================================="
echo ""
echo "To build the APK, run:"
echo "  ./build_android.sh"
echo ""
echo "Or open in Qt Creator:"
echo "  1. Open FastPayQt.pro or CMakeLists.txt"
echo "  2. Select Android kit"
echo "  3. Build → Build Project"
echo ""
echo "To reconfigure, run this wizard again:"
echo "  ./configure.sh"
echo ""
