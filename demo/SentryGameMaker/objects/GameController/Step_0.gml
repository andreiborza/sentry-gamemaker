// Restart game when R key is pressed
if (keyboard_check_pressed(ord("R"))) {
	sentry_add_breadcrumb(
	    "Player restarted the game",
	    "gameplay",
	    "user",
	    "{\"room_name\": \"" + room_get_name(room) + "\"}"
	);
	room_restart();
}