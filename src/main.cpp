#include <iostream>
#include <stdexcept>
#include <sentry.h>

int main() {
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, "https://36b59e45df048abbe20b7af6cdf76844@o447951.ingest.us.sentry.io/4509864179466240");
    // This is also the default-path. For further information and recommendations:
    // https://docs.sentry.io/platforms/native/configuration/options/#database-path
    sentry_options_set_database_path(options, ".sentry-native");
    sentry_options_set_release(options, "my-project-name@2.3.12");
    sentry_options_set_debug(options, 1);

    // Initialize Sentry
    int init_result = sentry_init(options);
    if (init_result != 0) {
        std::cerr << "Failed to initialize Sentry" << std::endl;
        return 1;
    }
    
    std::cout << "Sentry initialized successfully!" << std::endl;
    
    // Send a test message
    sentry_capture_event(sentry_value_new_message_event(
        SENTRY_LEVEL_INFO,
        "test",
        "Hello from sentry-native sample!"
    ));
    
    std::cout << "Test message sent to Sentry" << std::endl;
    
    sentry_close();
    std::cout << "Sentry closed successfully" << std::endl;
    
    return 0;
}