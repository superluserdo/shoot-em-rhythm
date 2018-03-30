int declare_visual_structure();
struct style_struct *decascade_visual_structure(struct style_struct *style, struct style_struct *visual_structure_list);
SDL_Rect float_rect_to_pixels(struct float_rect *float_rect, struct xy_struct *screen_size);
SDL_Rect visual_structure_to_pixels(struct style_struct *relative_style, struct style_struct *visual_structure_list, struct xy_struct *screen_size);
