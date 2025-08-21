draw_set_color(c_white);
draw_set_font(-1);
gpu_set_tex_filter(false);
draw_text_transformed(20, display_get_gui_height() - 40, "Ammo: " + string(ammo), 2, 2, 0);
gpu_set_tex_filter(true);