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
	shoot();
}

move_wrap(true, true, sprite_width / 2)

function shoot() {
	if (check_ammo()) {
		ammo -= 1;
		show_debug_message("Shot! Ammo left: " + string(ammo));
	}
}

function check_ammo() {
	if (ammo > 0) {
		return true;
	}
	
	throw ("No ammo left!");
}