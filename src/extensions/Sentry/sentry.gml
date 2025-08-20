/// @description Sentry exception handler for GameMaker - call with exception_unhandled_handler(sentry_exception_handler)
/// @param {Struct} exception_struct The exception struct from GameMaker's exception_unhandled_handler
/// @returns {Real} Always returns 0 to indicate handler completion
// function sentry_exception_handler(exception_struct) {

#define sentry_exception_handler 
try {
    // Convert the entire exception struct to JSON
    var exception_json = json_stringify(argument0);
    
    // Send to Sentry using the JSON capture function
    var result = sentry_capture_exception_json(exception_json);
    
    // Optional: Log for debugging (can be removed in production)
    show_debug_message("Sentry: Exception captured, result: " + string(result));
    
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
    return 0;
}

#define sentry_auto_init
var options = extension_get_options("Sentry");
var dsn = options.DSN;
var debug = options.debug;

if (debug) {
    show_debug_message("[SENTRY] Auto-initializing Sentry with DSN: " + dsn);
}
