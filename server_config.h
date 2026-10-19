#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <QString>

// ============================================
// SERVER CONFIGURATION
// ============================================
// The release URL is intentionally blank. Configure a trusted demo server in
// the app for local demonstrations. Do not point this prototype at a live
// payment service.
// ============================================

namespace ServerConfig {
    // Local testing server (for development only)
#ifdef Q_OS_ANDROID
    const QString LOCAL_SERVER = "http://10.0.2.2:8000";  // Android Emulator host loopback
#else
    const QString LOCAL_SERVER = "http://127.0.0.1:8000";
#endif
    
    // No production payment backend exists. Keep release builds unconfigured.
    const QString PRODUCTION_SERVER = "";
    
    // Automatic selection based on build type
    #ifdef QT_DEBUG
        const QString CURRENT_SERVER = LOCAL_SERVER;
    #else
        const QString CURRENT_SERVER = PRODUCTION_SERVER;
    #endif
}

#endif // SERVER_CONFIG_H
