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
#include <regex>
#include <nlohmann/json.hpp>


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
double sentry_gm_init(const char* dsn, const char* database_path, double sample_rate, double debug) {

    log_debug("Initializing Sentry...");
    log_debug("DSN: " + std::string(dsn ? dsn : "NULL"));
    log_debug("Database path: " + std::string(database_path ? database_path : "NULL"));
    log_debug("Sample rate: " + std::to_string(sample_rate));
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
    
    // Set sample rate
    sentry_options_set_sample_rate(options, sample_rate);
    log_debug("Sample rate set to: " + std::to_string(sample_rate));
    
    // Set debug mode and custom logger
    sentry_options_set_debug(options, (int)debug);
    
    // Set SDK name
    sentry_options_set_sdk_name(options, "sentry.native.gamemaker");
    log_debug("SDK name set to: sentry.native.gamemaker");
    
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


// Capture exception from GameMaker exception struct (JSON)
double sentry_gm_capture_exception(const char* exception_json, double handled) {
    log_debug("Capturing exception from GameMaker exception struct...");
    log_debug("Exception JSON: " + std::string(exception_json ? exception_json : "NULL"));
    log_debug("Raw handled parameter: " + std::to_string(handled));
    log_debug("Handled flag (boolean): " + std::to_string(handled != 0 ? 1 : 0));
    
    if (!exception_json) {
        log_debug("ERROR: Exception JSON is NULL");
        return -1.0;
    }
    
    using json = nlohmann::json;
    using json_parse_error = nlohmann::json::parse_error;
    
    try {
        
        // Parse the JSON exception struct
        json exception_data = json::parse(exception_json);
        
        // Extract fields from GameMaker exception struct
        std::string message = exception_data.value("message", "");
        std::string long_message = exception_data.value("longMessage", "");
        std::string script = exception_data.value("script", "");
        int line_number = exception_data.value("line", -1);
        
        // Extract stacktrace array if present
        std::vector<std::string> stacktrace_frames;
        if (exception_data.contains("stacktrace") && exception_data["stacktrace"].is_array()) {
            for (const auto& frame : exception_data["stacktrace"]) {
                if (frame.is_string()) {
                    stacktrace_frames.push_back(frame.get<std::string>());
                }
            }
        }
        
        log_debug("Parsed - Message: " + message);
        log_debug("Parsed - Long message: " + long_message);
        log_debug("Parsed - Script: " + script);
        log_debug("Parsed - Line: " + std::to_string(line_number));
        log_debug("Parsed - Stacktrace frames: " + std::to_string(stacktrace_frames.size()));
        
        // Use message as exception type, longMessage as exception value
        std::string exception_type = message.empty() ? "GameMakerException" : message;
        std::string exception_value = long_message.empty() ? message : long_message;
        
        if (exception_value.empty()) {
            log_debug("ERROR: No exception message available in exception data");
            return -1.0;
        }
        
        // Clean up exception_value after stacktrace parsing - remove redundant info
        // Remove the stacktrace line that starts with " at gml_" since we have proper stacktrace now
        size_t at_pos = exception_value.find("\n at gml_");
        if (at_pos != std::string::npos) {
            // Find the end of this line and remove it
            size_t line_end = exception_value.find('\n', at_pos + 1);
            if (line_end != std::string::npos) {
                exception_value.erase(at_pos, line_end - at_pos);
            } else {
                // Line goes to end of string
                exception_value.erase(at_pos);
            }
        }
        
        // Replace multiple newlines with single spaces to make it more readable
        std::regex newline_regex(R"(\s*\n\s*)");
        exception_value = std::regex_replace(exception_value, newline_regex, " ");
        
        // Clean up any extra whitespace
        std::regex whitespace_regex(R"(\s+)");
        exception_value = std::regex_replace(exception_value, whitespace_regex, " ");
        
        // Trim leading/trailing whitespace
        size_t start = exception_value.find_first_not_of(" \t");
        if (start != std::string::npos) {
            size_t end = exception_value.find_last_not_of(" \t");
            exception_value = exception_value.substr(start, end - start + 1);
        } else {
            exception_value = ""; // String was all whitespace
        }
        
        log_debug("Cleaned exception value: " + exception_value);
        
        // Create Sentry event following official documentation pattern
        log_debug("Creating event using official Sentry Native pattern...");
        sentry_value_t event = sentry_value_new_event();
        if (sentry_value_is_null(event)) {
            log_debug("ERROR: Failed to create event");
            return -1.0;
        }
        
        // Create exception with type from message and value from longMessage
        log_debug("Creating exception object...");
        log_debug("Exception type: " + exception_type);
        log_debug("Exception value: " + exception_value);
        sentry_value_t exc = sentry_value_new_exception(exception_type.c_str(), exception_value.c_str());
        if (sentry_value_is_null(exc)) {
            log_debug("ERROR: Failed to create exception object");
            return -1.0;
        }
        
        // Set mechanism following sentry-native pattern
        sentry_value_t mechanism = sentry_value_new_object();
        sentry_value_set_by_key(mechanism, "type", sentry_value_new_string("generic"));
        sentry_value_set_by_key(mechanism, "handled", sentry_value_new_bool(handled != 0.0));
        sentry_value_set_by_key(mechanism, "synthetic", sentry_value_new_bool(false));
        sentry_value_set_by_key(exc, "mechanism", mechanism);
        
        log_debug("Mechanism created - type: gamemaker, handled: " + std::string(handled != 0.0 ? "true" : "false") + ", synthetic: false");
        
        // Create stacktrace if we have frame data - only use GML frames, not native C++ frames
        if (!stacktrace_frames.empty() || (!script.empty() && line_number > 0)) {
            log_debug("Creating GML-only stacktrace...");
            
            // Create stacktrace object manually to avoid capturing native frames
            sentry_value_t stacktrace = sentry_value_new_object();
            sentry_value_t frames = sentry_value_new_list();
            
            if (!sentry_value_is_null(stacktrace) && !sentry_value_is_null(frames)) {
                // Helper function to parse GameMaker frame string following gmlogging-suite approach
                auto parseGMLFrame = [&](const std::string& frame_str, bool is_top_frame) -> sentry_value_t {
                    sentry_value_t frame = sentry_value_new_object();
                    
                    std::string function_name = "";
                    std::string filename = "";
                    int lineno = -1;
                    std::string context_line = "";
                    
                    // Parse format: "function_name (line X) - \tcontext_code"
                    // Based on gmlogging-suite parsing logic
                    
                    size_t line_pos = frame_str.find(" (line ");
                    if (line_pos != std::string::npos) {
                        // Extract function name before " (line "
                        std::string location_part = frame_str.substr(0, line_pos);
                        
                        // Find closing parenthesis after line number
                        size_t line_end = frame_str.find(")", line_pos);
                        if (line_end != std::string::npos) {
                            // Extract line number
                            std::string line_str = frame_str.substr(line_pos + 7, line_end - line_pos - 7);
                            try {
                                lineno = std::stoi(line_str);
                            } catch (...) {}
                            
                            // Extract context line after closing parenthesis and optional " - "
                            size_t context_start = line_end + 1;
                            if (context_start < frame_str.length()) {
                                std::string remaining = frame_str.substr(context_start);
                                
                                // Skip " - " if present
                                if (remaining.find(" - ") == 0) {
                                    remaining = remaining.substr(3);
                                }
                                
                                // Clean up leading/trailing whitespace and tabs
                                size_t first_char = remaining.find_first_not_of(" \t");
                                if (first_char != std::string::npos) {
                                    context_line = remaining.substr(first_char);
                                    // Remove trailing whitespace
                                    size_t last_char = context_line.find_last_not_of(" \t\r\n");
                                    if (last_char != std::string::npos) {
                                        context_line = context_line.substr(0, last_char + 1);
                                    }
                                }
                            }
                        }
                        
                        // Parse the location part to extract clean function and filename
                        // Handle format: "gml_Script_function@gml_Object_object" or "gml_Object_object"
                        try {
                            std::regex gml_regex(R"(^gml_(Script|Object)_(.+?)(?:@gml_(Script|Object)_(.+?))?$)");
                            std::smatch gml_matches;
                            
                            if (std::regex_match(location_part, gml_matches, gml_regex)) {
                                std::string first_type = gml_matches[1].str();
                                std::string first_name = gml_matches[2].str();
                                
                                if (gml_matches[3].matched && gml_matches[4].matched) {
                                    // Format: gml_Script_function@gml_Object_object
                                    std::string second_name = gml_matches[4].str();
                                    function_name = first_name;
                                    filename = second_name;
                                } else {
                                    // Format: gml_Object_object or gml_Script_function
                                    if (first_type == "Script") {
                                        function_name = first_name;
                                        filename = first_name;
                                    } else { // Object
                                        filename = first_name;
                                        function_name = "Event";
                                    }
                                }
                            } else {
                                // Fallback: use raw location
                                function_name = location_part;
                                filename = location_part;
                            }
                        } catch (const std::regex_error& e) {
                            log_debug("Regex error parsing location: " + std::string(e.what()));
                            function_name = location_part;
                            filename = location_part;
                        }
                    } else {
                        // No line number found, try colon-separated format as fallback
                        size_t colon_pos = frame_str.rfind(":");
                        if (colon_pos != std::string::npos && colon_pos < frame_str.length() - 1) {
                            std::string line_str = frame_str.substr(colon_pos + 1);
                            // Check if it's all digits
                            if (!line_str.empty() && std::all_of(line_str.begin(), line_str.end(), ::isdigit)) {
                                function_name = frame_str.substr(0, colon_pos);
                                try {
                                    lineno = std::stoi(line_str);
                                } catch (...) {}
                            } else {
                                function_name = frame_str;
                            }
                        } else {
                            function_name = frame_str;
                        }
                        filename = function_name;
                    }
                    
                    // Special handling for first frame: extract context from longMessage if not found in frame
                    if (is_top_frame && context_line.empty() && !long_message.empty()) {
                        log_debug("Attempting to extract context for top frame from longMessage");
                        log_debug("Frame string: " + frame_str);
                        log_debug("LongMessage: " + long_message);
                        
                        // Look for the current frame string in longMessage to find the context
                        // The longMessage typically contains the full error with context for the top frame
                        size_t frame_pos = long_message.find(frame_str);
                        log_debug("Frame position in longMessage: " + (frame_pos != std::string::npos ? std::to_string(frame_pos) : "NOT_FOUND"));
                        
                        if (frame_pos != std::string::npos) {
                            // Look for " - " after the frame reference on the same line
                            size_t after_frame = frame_pos + frame_str.length();
                            size_t dash_pos = long_message.find(" - ", after_frame);
                            if (dash_pos != std::string::npos) {
                                // Extract everything after " - " until the next newline
                                size_t context_start = dash_pos + 3;
                                size_t context_end = long_message.find('\n', context_start);
                                if (context_end == std::string::npos) context_end = long_message.length();
                                
                                std::string potential_context = long_message.substr(context_start, context_end - context_start);
                                
                                // Clean up the context line
                                size_t first_char = potential_context.find_first_not_of(" \t");
                                if (first_char != std::string::npos) {
                                    context_line = potential_context.substr(first_char);
                                    size_t last_char = context_line.find_last_not_of(" \t\r\n");
                                    if (last_char != std::string::npos) {
                                        context_line = context_line.substr(0, last_char + 1);
                                    }
                                }
                            }
                        } else {
                            // Alternative: look for this specific script+line combination in longMessage
                            // Build a search pattern like "gml_Script_check_ammo@gml_Object_obj_player_Step_0 (line 36)"
                            std::string search_pattern;
                            if (!function_name.empty() && !filename.empty() && lineno > 0) {
                                search_pattern = "gml_Script_" + function_name + "@gml_Object_" + filename + " (line " + std::to_string(lineno) + ")";
                                log_debug("Searching for pattern: " + search_pattern);
                                
                                size_t pattern_pos = long_message.find(search_pattern);
                                if (pattern_pos != std::string::npos) {
                                    log_debug("Found pattern at position: " + std::to_string(pattern_pos));
                                    // Look for " - " after the pattern
                                    size_t after_pattern = pattern_pos + search_pattern.length();
                                    size_t dash_pos = long_message.find(" - ", after_pattern);
                                    if (dash_pos != std::string::npos) {
                                        // Extract everything after " - " until the next newline
                                        size_t context_start = dash_pos + 3;
                                        size_t context_end = long_message.find('\n', context_start);
                                        if (context_end == std::string::npos) context_end = long_message.length();
                                        
                                        std::string potential_context = long_message.substr(context_start, context_end - context_start);
                                        
                                        // Clean up the context line
                                        size_t first_char = potential_context.find_first_not_of(" \t");
                                        if (first_char != std::string::npos) {
                                            context_line = potential_context.substr(first_char);
                                            size_t last_char = context_line.find_last_not_of(" \t\r\n");
                                            if (last_char != std::string::npos) {
                                                context_line = context_line.substr(0, last_char + 1);
                                            }
                                            log_debug("Extracted context: " + context_line);
                                        }
                                    }
                                } else {
                                    log_debug("Pattern not found in longMessage");
                                }
                            }
                        }
                    }
                    
                    // Set required fields for proper Sentry display
                    if (!function_name.empty()) {
                        sentry_value_set_by_key(frame, "function", sentry_value_new_string(function_name.c_str()));
                    }
                    if (!filename.empty()) {
                        sentry_value_set_by_key(frame, "filename", sentry_value_new_string(filename.c_str()));
                    }
                    if (lineno > 0) {
                        sentry_value_set_by_key(frame, "lineno", sentry_value_new_int32((int32_t)lineno));
                    }
                    if (!context_line.empty()) {
                        sentry_value_set_by_key(frame, "context_line", sentry_value_new_string(context_line.c_str()));
                    }
                    
                    // Mark as application code and set platform
                    sentry_value_set_by_key(frame, "in_app", sentry_value_new_bool(true));
                    sentry_value_set_by_key(frame, "platform", sentry_value_new_string("gml"));
                    
                    return frame;
                };
                
                // Process frames in REVERSE order (Sentry wants oldest to newest, GM gives newest to oldest)
                if (!stacktrace_frames.empty()) {
                    log_debug("Processing " + std::to_string(stacktrace_frames.size()) + " GML stacktrace frames in reverse order");
                    for (int i = stacktrace_frames.size() - 1; i >= 0; --i) {
                        const std::string& frame_str = stacktrace_frames[i];
                        sentry_value_t frame = parseGMLFrame(frame_str, i == 0); // First in array is top frame
                        sentry_value_append(frames, frame);
                    }
                } else {
                    // Create a single frame from script/line data
                    log_debug("Creating single GML frame from script/line data");
                    sentry_value_t frame = sentry_value_new_object();
                    
                    sentry_value_set_by_key(frame, "filename", sentry_value_new_string(script.c_str()));
                    sentry_value_set_by_key(frame, "function", sentry_value_new_string(script.c_str()));
                    if (line_number > 0) {
                        sentry_value_set_by_key(frame, "lineno", sentry_value_new_int32((int32_t)line_number));
                    }
                    sentry_value_set_by_key(frame, "in_app", sentry_value_new_bool(true));
                    
                    sentry_value_append(frames, frame);
                }
                
                // Set frames array on stacktrace object
                sentry_value_set_by_key(stacktrace, "frames", frames);
                
                log_debug("GML-only stacktrace created successfully with " + std::to_string(sentry_value_get_length(frames)) + " frames");
                
                // Attach stacktrace to exception
                sentry_value_set_by_key(exc, "stacktrace", stacktrace);
            }
        }
        
        // Add exception to event
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
        
        // Capture the event
        log_debug("Capturing exception event...");
        sentry_uuid_t uuid = sentry_capture_event(event);
        
        bool is_nil = sentry_uuid_is_nil(&uuid);
        log_debug("Exception captured, UUID is " + std::string(is_nil ? "NIL" : "valid"));
        
        return is_nil ? 0.0 : 1.0;
        
    } catch (const json_parse_error& e) {
        log_debug("ERROR: JSON parse error: " + std::string(e.what()));
        return -1.0;
    } catch (...) {
        log_debug("ERROR: Exception occurred during exception capture");
        return -1.0;
    }
}

// Close Sentry
double sentry_gm_close() {
    sentry_close();
    return 1.0;
}

} // extern "C"