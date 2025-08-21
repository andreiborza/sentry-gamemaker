instance_destroy();

with (other) {
	health -= 4;
	show_debug_message("Asteroid health: " + string(health));
	
	if (health <= 0) {
		instance_destroy();
		if (sprite_index == spr_rock_big) {
			repeat(2) {
				var new_asteroid = instance_create_layer(x, y, "Instances", obj_asteroid);
				new_asteroid.health = 2;
				new_asteroid.sprite_index = spr_rock_small;
	        }
			
			repeat(60) {
				instance_create_layer(x, y, "Instances", obj_debris);
			}
	    } else if (sprite_index == spr_rock_small) {
			repeat(2) {
				var new_asteroid = instance_create_layer(x, y, "Instances", obj_asteroid);
				new_asteroid.health = 1;
				new_asteroid.sprite_index = spr_rock_tiny;
	        }
			
			repeat(30) {
				instance_create_layer(x, y, "Instances", obj_debris);
			}
		} else {
			repeat(15) {
				instance_create_layer(x, y, "Instances", obj_debris);
			}
		}
	}
}