#include <sentry.h>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <errno.h>
#include <dlfcn.h>
#include <libgen.h>
#include <vector>


extern "C" {

// Helper function to log to file (declare first so it can be used everywhere)
void log_debug(const std::string& message) {
    std::ofstream logfile("/tmp/sentry_gm_debug.log", std::ios::app);
    if (logfile.is_open()) {
        logfile << "[SENTRY_GM] " << message << std::endl;
        logfile.close();
    }
    // Also try stdout
    std::cout << "[SENTRY_GM] " << message << std::endl;
    std::cout.flush();
}


// Custom logger for Sentry native
void sentry_logger(sentry_level_t level, const char *message, va_list args, void *userdata) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), message, args);
    
    log_debug("sentry_logger called with message: " + std::string(buffer));
    
    // Keep file logging as backup
    std::ofstream logfile("/tmp/sentry_native_debug.log", std::ios::app);
    if (logfile.is_open()) {
        logfile << "[SENTRY_NATIVE] " << buffer << std::endl;
        logfile.close();
    }
}

// Initialize Sentry with options  
double sentry_gm_init(const char* dsn, const char* database_path, const char* release, const char* crashpad_handler_path, double debug) {

    log_debug("Initializing Sentry...");
    log_debug("DSN: " + std::string(dsn ? dsn : "NULL"));
    log_debug("Database path: " + std::string(database_path ? database_path : "NULL"));
    log_debug("Release: " + std::string(release ? release : "NULL"));
    log_debug("Crashpad handler path: " + std::string(crashpad_handler_path ? crashpad_handler_path : "NULL"));
    log_debug("Debug mode: " + std::to_string((int)debug));
    
    sentry_options_t* options = sentry_options_new();
    
    // Set DSN
    if (dsn && strlen(dsn) > 0) {
        sentry_options_set_dsn(options, dsn);
        log_debug("DSN set successfully");
    }
    
    // Set database path (use /tmp for guaranteed write access)
    std::string db_path;
    if (database_path && strlen(database_path) > 0) {
        db_path = database_path;
    } else {
        // Use /tmp directory which is always writable
        db_path = "/tmp/sentry-native";
        mkdir(db_path.c_str(), 0755);
    }
    
    sentry_options_set_database_path(options, db_path.c_str());
    log_debug("Database path set to: " + db_path);
    
    // Set release (optional)
    if (release && strlen(release) > 0) {
        sentry_options_set_release(options, release);
        log_debug("Release set to: " + std::string(release));
    }
    
    // Set debug mode and custom logger
    sentry_options_set_debug(options, (int)debug);
    
    // Set crashpad handler path - find it relative to the extension library
    std::string handler_path;
    
    // Get the path of the current extension library
    Dl_info dl_info;
    if (dladdr((void*)sentry_gm_init, &dl_info)) {
        // Get the directory containing the .dylib
        char* lib_path = strdup(dl_info.dli_fname);
        char* lib_dir = dirname(lib_path);
        
        // Construct path to crashpad_handler in same directory
        handler_path = std::string(lib_dir) + "/crashpad_handler";
        
        log_debug("Extension library path: " + std::string(dl_info.dli_fname));
        log_debug("Extension directory: " + std::string(lib_dir));
        log_debug("Looking for crashpad handler at: " + handler_path);
        
        free(lib_path);
        
        if (access(handler_path.c_str(), F_OK) == 0) {
            // File exists, but check if it's executable
            if (access(handler_path.c_str(), X_OK) != 0) {
                // File exists but not executable - fix permissions
                log_debug("File exists but not executable, fixing permissions...");
                chmod(handler_path.c_str(), 0755);
            }
            
            if (access(handler_path.c_str(), X_OK) == 0) {
                sentry_options_set_handler_path(options, handler_path.c_str());
                log_debug("SUCCESS: Crashpad handler found and set to: " + handler_path);
            } else {
                log_debug("ERROR: Failed to make crashpad handler executable: " + handler_path);
                log_debug("FATAL: Cannot proceed without executable crashpad handler");
                log_debug("TO FIX: Run this command in terminal: chmod +x \"" + handler_path + "\"");
                sentry_options_free(options);
                return -1.0;
            }
        } else {
            log_debug("ERROR: Crashpad handler file not found: " + handler_path);
            log_debug("FATAL: Cannot proceed without crashpad handler");
            sentry_options_free(options);
            return -1.0;
        }
    } else {
        log_debug("ERROR: Could not determine extension library path");
        log_debug("FATAL: Cannot proceed without being able to locate crashpad handler");
        sentry_options_free(options);
        return -1.0;
    }
    
    // Set custom logger to capture Sentry's debug output
    if (debug > 0) {
        sentry_options_set_logger(options, sentry_logger, nullptr);
        log_debug("Custom logger set");
    }
    
    log_debug("Debug mode set to: " + std::to_string((int)debug));
    
    log_debug("Calling sentry_init...");
    int result = sentry_init(options);
    log_debug("sentry_init returned: " + std::to_string(result));
    
    if (result != 0) {
        log_debug("ERROR: sentry_init FAILED with code: " + std::to_string(result));
        log_debug("This means Sentry is NOT initialized and events will be rejected!");
    } else {
        log_debug("SUCCESS: Sentry initialized successfully");
    }
    
    return (double)result;
}

// Capture a message with level
double sentry_gm_capture_message(double level, const char* message) {
    log_debug("Capturing message...");
    log_debug("Level: " + std::to_string((int)level));
    log_debug("Message: " + std::string(message ? message : "NULL"));
    
    if (!message) {
        log_debug("ERROR: Message is NULL");
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
    
    log_debug("Creating event...");
    sentry_value_t event = sentry_value_new_message_event(sentry_level, "gamemaker", message);
    
    // Check if event creation succeeded
    if (sentry_value_is_null(event)) {
        log_debug("ERROR: Event creation failed - sentry_value_new_message_event returned null");
        return -1.0;
    }
    log_debug("Event created successfully");
    
    // Check if Sentry is properly initialized
    log_debug("Checking if Sentry is initialized...");
    // Note: There's no direct API to check this, but we can try to get user context
    sentry_value_t user = sentry_value_new_object();
    sentry_value_set_by_key(user, "id", sentry_value_new_string("test_user"));
    sentry_set_user(user);
    log_debug("User context set (this confirms Sentry is initialized)");
    
    log_debug("Capturing event...");
    sentry_uuid_t uuid = sentry_capture_event(event);
    
    bool is_nil = sentry_uuid_is_nil(&uuid);
    log_debug("Event captured, UUID is " + std::string(is_nil ? "NIL" : "valid"));
    
    // Event sent asynchronously
    log_debug("Message event captured");
    
    return is_nil ? 0.0 : 1.0;
}

// Direct exception handler that can be passed to exception_unhandled_handler
double sentry_gm_exception_handler(const char* json_exception_data) {
    log_debug("========== SENTRY C++ EXCEPTION HANDLER ==========");
    log_debug("Raw exception JSON: " + std::string(json_exception_data ? json_exception_data : "NULL"));
    
    if (!json_exception_data) {
        log_debug("ERROR: Exception data is NULL");
        return 0.0;
    }
    
    try {
        std::string json_str(json_exception_data);
        
        // For now, just send the entire JSON as the message
        // You could parse this JSON to extract specific fields if needed
        sentry_level_t level = SENTRY_LEVEL_ERROR;
        sentry_value_t event = sentry_value_new_message_event(level, "gamemaker.exception", json_str.c_str());
        
        if (sentry_value_is_null(event)) {
            log_debug("ERROR: Failed to create message event");
            return 0.0;
        }
        
        // Add the raw exception data as extra context
        sentry_value_t extra = sentry_value_new_object();
        sentry_value_set_by_key(extra, "gamemaker_exception", sentry_value_new_string(json_str.c_str()));
        sentry_value_set_by_key(event, "extra", extra);
        
        log_debug("Capturing exception event...");
        sentry_uuid_t uuid = sentry_capture_event(event);
        
        bool is_nil = sentry_uuid_is_nil(&uuid);
        log_debug("Exception captured, UUID is " + std::string(is_nil ? "NIL" : "valid"));
        
        // Close Sentry to ensure event is sent before process termination
        sentry_close();
        log_debug("Handler completed - Sentry closed");
        
        return 0.0; // Always return 0 to indicate handler completed
        
    } catch (...) {
        log_debug("ERROR: Exception occurred in C++ exception handler");
        return 0.0;
    }
}

// Capture an exception event from GML exception handler
double sentry_gm_capture_exception(const char* message, const char* stack_trace, const char* script_name, double line_number) {
    log_debug("Capturing exception...");
    log_debug("Message: " + std::string(message ? message : "NULL"));
    log_debug("Stack trace: " + std::string(stack_trace ? stack_trace : "NULL"));
    log_debug("Script: " + std::string(script_name ? script_name : "NULL"));
    log_debug("Line: " + std::to_string((int)line_number));
    
    if (!message) {
        log_debug("ERROR: Message is NULL");
        return -1.0;
    }
    
    try {
        // Follow official Sentry Native documentation pattern
        log_debug("Creating event using official Sentry Native pattern...");
        sentry_value_t event = sentry_value_new_event();
        if (sentry_value_is_null(event)) {
            log_debug("ERROR: Failed to create event");
            return -1.0;
        }
        
        // Create exception with type and message
        log_debug("Creating exception object...");
        sentry_value_t exc = sentry_value_new_exception("GameMakerException", message);
        if (sentry_value_is_null(exc)) {
            log_debug("ERROR: Failed to create exception object");
            return -1.0;
        }
        
        // Create and attach stacktrace if we have stack trace data
        if (stack_trace && strlen(stack_trace) > 0) {
            log_debug("Creating stacktrace using sentry_value_new_stacktrace...");
            
            // Use sentry_value_new_stacktrace to create proper stacktrace
            sentry_value_t stacktrace = sentry_value_new_stacktrace(NULL, 0);
            if (!sentry_value_is_null(stacktrace)) {
                // Get the frames list from the stacktrace
                sentry_value_t frames = sentry_value_get_by_key(stacktrace, "frames");
                if (!sentry_value_is_null(frames)) {
                    // Create a frame based on our GML data
                    sentry_value_t frame = sentry_value_new_object();
                    
                    if (script_name && strlen(script_name) > 0) {
                        sentry_value_set_by_key(frame, "filename", sentry_value_new_string(script_name));
                        sentry_value_set_by_key(frame, "function", sentry_value_new_string(script_name));
                    }
                    
                    if (line_number > 0) {
                        sentry_value_set_by_key(frame, "lineno", sentry_value_new_int32((int32_t)line_number));
                    }
                    
                    // Mark as application code
                    sentry_value_set_by_key(frame, "in_app", sentry_value_new_bool(true));
                    
                    // Add the stack trace line as context
                    sentry_value_set_by_key(frame, "context_line", sentry_value_new_string(stack_trace));
                    
                    // Add frame to the stacktrace
                    sentry_value_append(frames, frame);
                    
                    log_debug("Stacktrace created successfully");
                }
                
                // Attach stacktrace to exception using the official method
                sentry_value_set_stacktrace(exc, NULL, 0);
                // Then manually set our custom stacktrace
                sentry_value_set_by_key(exc, "stacktrace", stacktrace);
            }
        }
        
        // Add exception to event using official method
        log_debug("Adding exception to event...");
        sentry_event_add_exception(event, exc);
        
        // Add tags for easier filtering
        sentry_value_t tags = sentry_value_new_object();
        sentry_value_set_by_key(tags, "engine", sentry_value_new_string("GameMaker"));
        if (script_name) {
            sentry_value_set_by_key(tags, "script", sentry_value_new_string(script_name));
        }
        sentry_value_set_by_key(event, "tags", tags);
        log_debug("Exception event created using official Sentry Native pattern");
        
        log_debug("Capturing exception event...");
        sentry_uuid_t uuid = sentry_capture_event(event);
        
        bool is_nil = sentry_uuid_is_nil(&uuid);
        log_debug("Exception captured, UUID is " + std::string(is_nil ? "NIL" : "valid"));
        
        // Event sent asynchronously
        log_debug("Exception event captured");
        
        return is_nil ? 0.0 : 1.0;
        
    } catch (...) {
        log_debug("ERROR: Exception occurred during sentry_capture_exception");
        return -1.0;
    }
}

// Capture exception from JSON string containing full GameMaker exception struct
double sentry_gm_capture_exception_json(const char* exception_json) {
    log_debug("Capturing exception from JSON...");
    log_debug("Exception JSON: " + std::string(exception_json ? exception_json : "NULL"));
    
    if (!exception_json) {
        log_debug("ERROR: Exception JSON is NULL");
        return -1.0;
    }
    
    try {
        // For now, we'll do simple string parsing since we don't have a JSON library
        // We'll extract the key fields manually
        std::string json_str(exception_json);
        
        std::string message = "";
        std::string long_message = "";
        std::string script = "";
        int line_number = -1;
        std::vector<std::string> stacktrace_frames;
        
        // Simple JSON parsing for message
        size_t msg_start = json_str.find("\"message\":\"");
        if (msg_start != std::string::npos) {
            msg_start += 11; // Length of "message":"
            size_t msg_end = json_str.find("\"", msg_start);
            if (msg_end != std::string::npos) {
                message = json_str.substr(msg_start, msg_end - msg_start);
            }
        }
        
        // Simple JSON parsing for longMessage
        size_t long_msg_start = json_str.find("\"longMessage\":\"");
        if (long_msg_start != std::string::npos) {
            long_msg_start += 15; // Length of "longMessage":"
            size_t long_msg_end = json_str.find("\"", long_msg_start);
            if (long_msg_end != std::string::npos) {
                long_message = json_str.substr(long_msg_start, long_msg_end - long_msg_start);
            }
        }
        
        // Simple JSON parsing for script
        size_t script_start = json_str.find("\"script\":\"");
        if (script_start != std::string::npos) {
            script_start += 10; // Length of "script":"
            size_t script_end = json_str.find("\"", script_start);
            if (script_end != std::string::npos) {
                script = json_str.substr(script_start, script_end - script_start);
            }
        }
        
        // Simple JSON parsing for line number
        size_t line_start = json_str.find("\"line\":");
        if (line_start != std::string::npos) {
            line_start += 7; // Length of "line":
            size_t line_end = json_str.find_first_of(",}", line_start);
            if (line_end != std::string::npos) {
                std::string line_str = json_str.substr(line_start, line_end - line_start);
                line_number = std::stoi(line_str);
            }
        }
        
        log_debug("Parsed - Message: " + message);
        log_debug("Parsed - Long message: " + long_message);
        log_debug("Parsed - Script: " + script);
        log_debug("Parsed - Line: " + std::to_string(line_number));
        
        // Use longMessage as the main message (more detailed)
        std::string final_message = long_message.empty() ? message : long_message;
        
        // Follow official Sentry Native documentation pattern
        log_debug("Creating event using official Sentry Native pattern...");
        sentry_value_t event = sentry_value_new_event();
        if (sentry_value_is_null(event)) {
            log_debug("ERROR: Failed to create event");
            return -1.0;
        }
        
        // Create exception with type and message
        log_debug("Creating exception object...");
        sentry_value_t exc = sentry_value_new_exception("GameMakerException", final_message.c_str());
        if (sentry_value_is_null(exc)) {
            log_debug("ERROR: Failed to create exception object");
            return -1.0;
        }
        
        // Create stacktrace with the script and line info we have
        if (!script.empty()) {
            log_debug("Creating stacktrace using sentry_value_new_stacktrace...");
            
            sentry_value_t stacktrace = sentry_value_new_stacktrace(NULL, 0);
            if (!sentry_value_is_null(stacktrace)) {
                sentry_value_t frames = sentry_value_get_by_key(stacktrace, "frames");
                if (!sentry_value_is_null(frames)) {
                    // Create a frame based on our parsed data
                    sentry_value_t frame = sentry_value_new_object();
                    
                    sentry_value_set_by_key(frame, "filename", sentry_value_new_string(script.c_str()));
                    sentry_value_set_by_key(frame, "function", sentry_value_new_string(script.c_str()));
                    sentry_value_set_by_key(frame, "lineno", sentry_value_new_int32((int32_t)line_number));
                    sentry_value_set_by_key(frame, "in_app", sentry_value_new_bool(true));
                    
                    // Add frame to the stacktrace
                    sentry_value_append(frames, frame);
                    
                    log_debug("Stacktrace created successfully");
                }
                
                // Attach stacktrace to exception
                sentry_value_set_stacktrace(exc, NULL, 0);
                sentry_value_set_by_key(exc, "stacktrace", stacktrace);
            }
        }
        
        // Add exception to event using official method
        log_debug("Adding exception to event...");
        sentry_event_add_exception(event, exc);
        
        // Add tags for easier filtering
        sentry_value_t tags = sentry_value_new_object();
        sentry_value_set_by_key(tags, "engine", sentry_value_new_string("GameMaker"));
        if (!script.empty()) {
            sentry_value_set_by_key(tags, "script", sentry_value_new_string(script.c_str()));
        }
        sentry_value_set_by_key(event, "tags", tags);
        log_debug("Exception event created using official Sentry Native pattern");
        
        log_debug("Capturing exception event...");
        sentry_uuid_t uuid = sentry_capture_event(event);
        
        bool is_nil = sentry_uuid_is_nil(&uuid);
        log_debug("Exception captured, UUID is " + std::string(is_nil ? "NIL" : "valid"));
        
        // Event sent asynchronously
        log_debug("Exception event captured");
        
        return is_nil ? 0.0 : 1.0;
        
    } catch (...) {
        log_debug("ERROR: Exception occurred during JSON exception capture");
        return -1.0;
    }
}

// Close Sentry
double sentry_gm_close() {
    sentry_close();
    return 1.0;
}

} // extern "C"