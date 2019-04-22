struct float_rect rect_out_from_relative(struct visual_container_struct *container);
//struct float_rect decascade_visual_container(struct visual_container_struct *container);
struct float_rect decascade_visual_container(struct visual_container_struct *container, struct xy_struct screen_size, float *aspctr_inherit);
SDL_Rect float_rect_to_pixels(struct float_rect *float_rect, struct xy_struct screen_size);
SDL_Rect visual_container_to_pixels(struct visual_container_struct *relative_container, struct xy_struct screen_size);
int container_test_overlap(struct visual_container_struct *container_1, struct visual_container_struct *container_2, struct xy_struct screen_size);
int container_test_overlap_x(struct visual_container_struct *container_1, struct visual_container_struct *container_2, struct xy_struct screen_size);
int container_test_overlap_y(struct visual_container_struct *container_1, struct visual_container_struct *container_2, struct xy_struct screen_size);
struct size_ratio_struct pos_at_custom_anchor_hook(struct visual_container_struct *container, float x, float y, struct xy_struct screen_size);
