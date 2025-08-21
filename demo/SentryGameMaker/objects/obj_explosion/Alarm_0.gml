// Clean up particle system
part_emitter_destroy(explosion_system, explosion_emitter);
part_type_destroy(explosion_orange);
part_type_destroy(explosion_yellow);
part_system_destroy(explosion_system);

instance_destroy();