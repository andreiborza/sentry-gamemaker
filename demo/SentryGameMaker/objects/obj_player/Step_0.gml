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
	throw ("Hello World")
}

move_wrap(true, true, sprite_width / 2)