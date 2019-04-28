void *frametimer(void *); /* DEPRECATED - Used to be the only clock function running 
							 in a loop in its own thread - now split into multiple 
							 functions called by the main thread */

void timing_init(struct time_struct *timing);
void timing_zero(struct time_struct *timing);
int wait_to_present(struct time_struct *timing);
void update_time(struct time_struct *timing);
void pause_time(struct time_struct *timing);
void unpause_time(struct time_struct *timing);
