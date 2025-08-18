#include <sentry.h>
#include <string>
#include <cstring>

extern "C" {

// Initialize Sentry with options  
double sentry_gm_init(const char* dsn, const char* database_path, const char* release, double debug) {
    sentry_options_t* options = sentry_options_new();
    
    // Set DSN
    if (dsn && strlen(dsn) > 0) {
        sentry_options_set_dsn(options, dsn);
    }
    
    // Set database path (default if empty)
    if (database_path && strlen(database_path) > 0) {
        sentry_options_set_database_path(options, database_path);
    } else {
        sentry_options_set_database_path(options, ".sentry-native");
    }
    
    // Set release (optional)
    if (release && strlen(release) > 0) {
        sentry_options_set_release(options, release);
    }
    
    // Set debug mode
    sentry_options_set_debug(options, (int)debug);
    
    int result = sentry_init(options);
    return (double)result;
}

// Capture a message with level
double sentry_gm_capture_message(double level, const char* message) {
    if (!message) {
        return -1.0;
    }
    
    sentry_level_t sentry_level;
    switch ((int)level) {
        case 0: sentry_level = SENTRY_LEVEL_DEBUG; break;
        case 1: sentry_level = SENTRY_LEVEL_INFO; break;
        case 2: sentry_level = SENTRY_LEVEL_WARNING; break;
        case 3: sentry_level = SENTRY_LEVEL_ERROR; break;
        case 4: sentry_level = SENTRY_LEVEL_FATAL; break;
        default: sentry_level = SENTRY_LEVEL_INFO; break;
    }
    
    sentry_value_t event = sentry_value_new_message_event(sentry_level, "gamemaker", message);
    sentry_uuid_t uuid = sentry_capture_event(event);
    
    return sentry_uuid_is_nil(&uuid) ? 0.0 : 1.0;
}

// Close Sentry
double sentry_gm_close() {
    sentry_close();
    return 1.0;
}

} // extern "C"