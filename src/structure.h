struct float_rect decascade_visual_container(struct visual_container_struct *container);
SDL_Rect float_rect_to_pixels(struct float_rect *float_rect, struct xy_struct screen_size);
SDL_Rect visual_container_to_pixels(struct visual_container_struct *relative_container, struct xy_struct screen_size);
