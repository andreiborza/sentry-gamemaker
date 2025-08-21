ammo = 20;

// Set user information for tracking
sentry_set_user("player_" + string(random(10000)), "test_player", "player@gamemaker.io", "");

// Add breadcrumb for player creation
sentry_add_breadcrumb(
    "Player object created",
    "lifecycle",
    "default",
    "{\"initial_ammo\": " + string(ammo) + ", \"position_x\": " + string(x) + ", \"position_y\": " + string(y) + "}"
);