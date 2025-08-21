if (keyboard_check(vk_left)) {
	image_angle += 5
}

if (keyboard_check(vk_right)) {
	image_angle -= 5
}

if (keyboard_check(vk_up)) {
	motion_add(image_angle, 0.1)
}

if (keyboard_check(vk_down)) {
	motion_add(image_angle, -0.1)
}

// Test exception handling - press space to trigger an error
if (keyboard_check_pressed(vk_space)) {
	// Add breadcrumb for player action
	sentry_add_breadcrumb(
	    "Player attempted to shoot",
	    "gameplay",
	    "user",
	    "{\"current_ammo\": " + string(ammo) + ", \"player_angle\": " + string(image_angle) + "}"
	);
	shoot();
}

move_wrap(true, true, sprite_width / 2)

function shoot() {
	if (check_ammo()) {
		ammo -= 1;
		show_debug_message("Shot! Ammo left: " + string(ammo));
		
		// Add breadcrumb for successful shot
		sentry_add_breadcrumb(
		    "Shot fired successfully",
		    "gameplay",
		    "default",
		    "{\"ammo_remaining\": " + string(ammo) + ", \"shot_angle\": " + string(image_angle) + "}"
		);
		
		// Add warning breadcrumb when ammo is low
		if (ammo <= 1) {
			sentry_add_breadcrumb(
			    "Low ammo warning",
			    "gameplay",
			    "warning",
			    "{\"ammo_remaining\": " + string(ammo) + "}"
			);
		}
	}
}

function check_ammo() {
	if (ammo > 0) {
		return true;
	}
	
	// Add breadcrumb before throwing exception
	sentry_add_breadcrumb(
	    "Ammo check failed - out of ammo",
	    "gameplay",
	    "error",
	    "{\"ammo_remaining\": " + string(ammo) + ", \"attempted_action\": \"shoot\"}"
	);
	
	throw ("No ammo left!");
}