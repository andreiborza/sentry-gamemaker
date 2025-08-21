// Add game controller initialization breadcrumb
sentry_add_breadcrumb(
    "Game controller initialized",
    "lifecycle",
    "default",
    "{\"room_name\": \"" + room_get_name(room) + "\", \"instance_count\": " + string(instance_count) + "}"
);

// Add breadcrumb for test event
sentry_add_breadcrumb(
    "Sending test health warning",
    "testing",
    "warning",
    "{\"health_threshold\": \"low\", \"test_type\": \"health_warning\"}"
);

// Send test messages
sentry_capture_message(2, "Warning: Low health detected");