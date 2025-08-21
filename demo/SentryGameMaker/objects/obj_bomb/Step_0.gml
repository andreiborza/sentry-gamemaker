// Update and emit particles for trail
part_system_update(trail_system);

// Emit orange trail particles
part_emitter_region(trail_system, trail_emitter, x-2, x+2, y-2, y+2, ps_shape_ellipse, ps_distr_gaussian);
part_emitter_burst(trail_system, trail_emitter, trail_particle, 6);

// Emit yellow fire particles closer to bomb center (more concentrated)
part_emitter_region(trail_system, trail_emitter, x-1, x+1, y-1, y+1, ps_shape_ellipse, ps_distr_gaussian);
part_emitter_burst(trail_system, trail_emitter, fire_particle, 3);