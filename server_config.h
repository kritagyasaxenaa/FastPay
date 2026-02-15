#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <QString>

// ============================================
// SERVER CONFIGURATION
// ============================================
// IMPORTANT: Change PRODUCTION_SERVER before building release APK!
//
// Instructions:
// 1. Deploy your Python server to a hosting service (Render, Railway, etc.)
// 2. Replace the URL below with your actual server URL
// 3. Build the APK in Release mode
// ============================================

namespace ServerConfig {
    // Local testing server (for development only)
    const QString LOCAL_SERVER = "http://10.0.2.2:8000";  // Android emulator localhost
    
    // ⚠️ CHANGE THIS TO YOUR PRODUCTION SERVER URL ⚠️
    // Examples:
    //   "https://fastpay-api.onrender.com"
    //   "https://fastpay-production.up.railway.app"
    //   "https://your-domain.com"
    const QString PRODUCTION_SERVER = "https://trinity-mlg4.onrender.com";
    
    // Automatic selection based on build type
    #ifdef QT_DEBUG
        const QString CURRENT_SERVER = LOCAL_SERVER;
    #else
        const QString CURRENT_SERVER = PRODUCTION_SERVER;
    #endif
}

#endif // SERVER_CONFIG_H
