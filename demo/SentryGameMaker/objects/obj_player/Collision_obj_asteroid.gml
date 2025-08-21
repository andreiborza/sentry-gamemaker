// Create explosion particle system
var explosion_system = part_system_create();
part_system_automatic_update(explosion_system, true);
part_system_automatic_draw(explosion_system, true);

var explosion_emitter = part_emitter_create(explosion_system);

// Create orange explosion particles (copied from bomb)
var explosion_orange = part_type_create();
part_type_shape(explosion_orange, pt_shape_disk);
part_type_size(explosion_orange, 0.1, 0.3, 0, 0);
part_type_scale(explosion_orange, 1, 1);
part_type_color1(explosion_orange, c_orange);
part_type_alpha3(explosion_orange, 1, 0.5, 0);
part_type_speed(explosion_orange, 1, 4, -0.05, 0);
part_type_direction(explosion_orange, 0, 360, 0, 0);
part_type_life(explosion_orange, 30, 60);

// Create yellow fire core particles (copied from bomb)
var explosion_yellow = part_type_create();
part_type_shape(explosion_yellow, pt_shape_disk);
part_type_size(explosion_yellow, 0.05, 0.15, 0, 0);
part_type_scale(explosion_yellow, 1, 1);
part_type_color1(explosion_yellow, c_yellow);
part_type_alpha3(explosion_yellow, 1, 0.7, 0);
part_type_speed(explosion_yellow, 0.5, 3, -0.08, 0);
part_type_direction(explosion_yellow, 0, 360, 0, 0);
part_type_life(explosion_yellow, 20, 40);

// Emit explosion particles
part_emitter_region(explosion_system, explosion_emitter, x-3, x+3, y-3, y+3, ps_shape_ellipse, ps_distr_gaussian);
part_emitter_burst(explosion_system, explosion_emitter, explosion_orange, 50);
part_emitter_burst(explosion_system, explosion_emitter, explosion_yellow, 30);

instance_destroy();