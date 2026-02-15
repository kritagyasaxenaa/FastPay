#!/bin/bash

# ============================================
# FastPay Pre-Build Verification
# ============================================
# Run this before building to check everything is configured correctly

echo "=========================================="
echo "  FastPay Pre-Build Verification"
echo "=========================================="
echo ""

ERRORS=0
WARNINGS=0

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

check_pass() {
    echo -e "${GREEN}✅ $1${NC}"
}

check_fail() {
    echo -e "${RED}❌ $1${NC}"
    ((ERRORS++))
}

check_warn() {
    echo -e "${YELLOW}⚠️  $1${NC}"
    ((WARNINGS++))
}

# ============================================
# 1. Check Server Configuration
# ============================================

echo "1. Server Configuration"
echo "   --------------------"

if [ ! -f "server_config.h" ]; then
    check_fail "server_config.h not found!"
else
    if grep -q "your-app-name.onrender.com" server_config.h; then
        check_warn "Server URL not configured (still using placeholder)"
        echo "      Edit server_config.h and set PRODUCTION_SERVER"
    else
        SERVER_URL=$(grep "PRODUCTION_SERVER" server_config.h | sed 's/.*"\(.*\)".*/\1/')
        check_pass "Server configured: $SERVER_URL"
    fi
fi

echo ""

# ============================================
# 2. Check Required Files
# ============================================

echo "2. Required Files"
echo "   --------------"

REQUIRED_FILES=(
    "CMakeLists.txt"
    "FastPayQt.pro"
    "main.cpp"
    "mainwindow.cpp"
    "mainwindow.h"
    "transactionengine.cpp"
    "transactionengine.h"
    "ultrasoundhelper.cpp"
    "ultrasoundhelper.h"
    "server_config.h"
    "android/AndroidManifest.xml"
    "android/build.gradle"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        check_pass "$file"
    else
        check_fail "$file missing!"
    fi
done

echo ""

# ============================================
# 3. Check Crypto Module
# ============================================

echo "3. Crypto Module Files"
echo "   -------------------"

CRYPTO_FILES=(
    "Crypto/AES.cpp"
    "Crypto/ECDSA.cpp"
    "Crypto/RSA.cpp"
    "Crypto/Ultrasound.cpp"
    "Crypto/transaction.cpp"
)

for file in "${CRYPTO_FILES[@]}"; do
    if [ -f "$file" ]; then
        check_pass "$file"
    else
        check_fail "$file missing!"
    fi
done

echo ""

# ============================================
# 4. Check CMakeLists.txt Content
# ============================================

echo "4. Build Configuration"
echo "   -------------------"

if grep -q "Qt6::Multimedia" CMakeLists.txt; then
    check_pass "Qt Multimedia included in CMakeLists.txt"
else
    check_fail "Qt Multimedia missing from CMakeLists.txt"
fi

if grep -q "Crypto/AES.cpp" CMakeLists.txt; then
    check_pass "Crypto files included in CMakeLists.txt"
else
    check_fail "Crypto files missing from CMakeLists.txt"
fi

if grep -q "OpenSSL" CMakeLists.txt; then
    check_pass "OpenSSL configured in CMakeLists.txt"
else
    check_warn "OpenSSL not found in CMakeLists.txt"
fi

echo ""

# ============================================
# 5. Check Android Configuration
# ============================================

echo "5. Android Configuration"
echo "   ---------------------"

if [ -f "android/AndroidManifest.xml" ]; then
    if grep -q "RECORD_AUDIO" android/AndroidManifest.xml; then
        check_pass "RECORD_AUDIO permission in manifest"
    else
        check_warn "RECORD_AUDIO permission missing"
    fi
    
    if grep -q "INTERNET" android/AndroidManifest.xml; then
        check_pass "INTERNET permission in manifest"
    else
        check_warn "INTERNET permission missing"
    fi
fi

echo ""

# ============================================
# 6. Check Build Scripts
# ============================================

echo "6. Build Scripts"
echo "   -------------"

if [ -f "build_android.sh" ] && [ -x "build_android.sh" ]; then
    check_pass "build_android.sh is executable"
else
    check_warn "build_android.sh not executable (run: chmod +x build_android.sh)"
fi

if [ -f "configure.sh" ] && [ -x "configure.sh" ]; then
    check_pass "configure.sh is executable"
else
    check_warn "configure.sh not executable (run: chmod +x configure.sh)"
fi

echo ""

# ============================================
# 7. Check System Requirements
# ============================================

echo "7. System Requirements"
echo "   -------------------"

# Check Java
if command -v java &> /dev/null; then
    JAVA_VERSION=$(java -version 2>&1 | head -n 1)
    check_pass "Java installed: $JAVA_VERSION"
else
    check_fail "Java not found (install OpenJDK 17)"
fi

# Check if Qt is likely installed
if [ -d "$HOME/Qt" ]; then
    check_pass "Qt directory found at ~/Qt"
else
    check_warn "Qt directory not found at default location ~/Qt"
fi

# Check if Android SDK is likely installed
if [ -d "$HOME/Android/Sdk" ]; then
    check_pass "Android SDK found at ~/Android/Sdk"
else
    check_warn "Android SDK not found at default location"
fi

# Check ADB
if command -v adb &> /dev/null; then
    check_pass "ADB (Android Debug Bridge) available"
else
    check_warn "ADB not found (needed for installing APK)"
fi

echo ""

# ============================================
# Summary
# ============================================

echo "=========================================="
echo "  Verification Summary"
echo "=========================================="
echo ""

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo -e "${GREEN}✅ All checks passed! Ready to build.${NC}"
    echo ""
    echo "Run: ./build_android.sh"
elif [ $ERRORS -eq 0 ]; then
    echo -e "${YELLOW}⚠️  $WARNINGS warning(s) found${NC}"
    echo ""
    echo "You can proceed with the build, but review warnings above."
    echo ""
    echo "Run: ./build_android.sh"
else
    echo -e "${RED}❌ $ERRORS error(s) and $WARNINGS warning(s) found${NC}"
    echo ""
    echo "Please fix the errors above before building."
    exit 1
fi

echo ""
