//var save_dir = game_save_id;
//var sentry_db_path = save_dir + ".sentry-native";
  
//// Initialize Sentry
//var result = sentry_init("https://36b59e45df048abbe20b7af6cdf76844@o447951.ingest.us.sentry.io/4509864179466240", sentry_db_path, "my-game@1.0.0", 1.0);

//show_debug_message("called sentry init: " + string(result))

//// Set up Sentry exception handler - now users just need this one line!
//exception_unhandled_handler(sentry_exception_handler);

// Send test messages
//sentry_capture_message(2, "Warning: Low health detected");