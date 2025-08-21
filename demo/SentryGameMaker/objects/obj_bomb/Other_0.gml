// Clean up particle system
part_emitter_destroy(trail_system, trail_emitter);
part_type_destroy(trail_particle);
part_type_destroy(fire_particle);
part_system_destroy(trail_system);

instance_destroy();
