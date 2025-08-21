speed = 6;

// Create particle system for trail
trail_system = part_system_create();
trail_emitter = part_emitter_create(trail_system);

// Create particle type for orange trail
trail_particle = part_type_create();
part_type_shape(trail_particle, pt_shape_disk);
part_type_size(trail_particle, 0.1, 0.3, 0, 0);
part_type_scale(trail_particle, 1, 1);
part_type_color1(trail_particle, c_orange);
part_type_alpha3(trail_particle, 1, 0.5, 0);
part_type_speed(trail_particle, 0.5, 1.5, -0.01, 0);
part_type_direction(trail_particle, direction - 45, direction + 45, 0, 0);
part_type_life(trail_particle, 15, 30);

// Create particle type for yellow fire core
fire_particle = part_type_create();
part_type_shape(fire_particle, pt_shape_disk);
part_type_size(fire_particle, 0.05, 0.15, 0, 0);
part_type_scale(fire_particle, 1, 1);
part_type_color1(fire_particle, c_yellow);
part_type_alpha3(fire_particle, 1, 0.7, 0);
part_type_speed(fire_particle, 0.3, 1, -0.02, 0);
part_type_direction(fire_particle, direction - 20, direction + 20, 0, 0);
part_type_life(fire_particle, 10, 20);