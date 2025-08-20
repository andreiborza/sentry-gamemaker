/// @description Sentry exception handler for GameMaker - call with exception_unhandled_handler(sentry_exception_handler)
/// @param {Struct} exception_struct The exception struct from GameMaker's exception_unhandled_handler
/// @returns {Real} Always returns 0 to indicate handler completion
// function sentry_exception_handler(exception_struct) {

#define sentry_exception_handler 
try {
    // Convert the entire exception struct to JSON
    var exception_json = json_stringify(argument0);

    show_debug_message("Capturing exception json: " + exception_json);
    
    // Send to Sentry using the JSON capture function
    var result = sentry_capture_exception_json(exception_json);
    
    // Optional: Log for debugging (can be removed in production)
    show_debug_message("Sentry: Exception captured, result: " + string(result));

    sentry_close();
    
    return 0;
} catch (e) {
    // Fallback: if JSON processing fails, try to send basic message
    try {
        var fallback_message = "GameMaker Exception: " + string(argument0.message || "Unknown error");
        sentry_capture_message(3, fallback_message); // Level 3 = error
        show_debug_message("Sentry: Used fallback exception capture");
    } catch (e2) {
        show_debug_message("Sentry: Failed to capture exception - " + string(e2));
    }

    sentry_close();
    return 0;
}

#define sentry_auto_init
var options = extension_get_options("Sentry");
var dsn = options.DSN;
var debug = options.debug;
var setup_exception_handler = options.setup_exception_handler;

if (debug) {
    show_debug_message("[SENTRY] Auto-initializing Sentry with DSN: " + dsn);
}

var save_dir = game_save_id;
var sentry_db_path = save_dir + ".sentry-native";
  
// Initialize Sentry
var result = sentry_init(dsn, sentry_db_path, debug ? 1.0 : 0.0);

show_debug_message("called sentry init: " + string(result))

if (setup_exception_handler) {
    show_debug_message("[SENTRY] Set up Sentry exception handler");

    exception_unhandled_handler(sentry_exception_handler);
}
