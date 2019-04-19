#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "structdef.h"
#include "newstruct.h"
#include "main.h"
#include "level.h"
#include "music.h"
#include "clock.h"
#include "helpers.h"
#include "animate.h"
#include "transform.h" // Can hopefully get rid of this soon
#include "spawn.h"
#include "deprecated_funcs.h"

#define SWORD_WIDTH 32
#define SWORD_HEIGHT 32 // Get rid of this, put it in animaions.cfg or something

void moveme(struct lane_struct *lanes, int *direction, struct animate_specific *anim) {

	if (*direction < 4) {
		if (*direction == 0 || *direction == 3) {
			if (lanes->currentlane > 0) {
				(lanes->currentlane)--;
			}
		}
		if (*direction == 2 || *direction == 1) {
			if (lanes->currentlane < lanes->total - 1) {
				(lanes->currentlane)++;
			}
		}
		*direction = 4;
	}
	/* set sprite position */
	anim->object->container->inherit = &lanes->containers[lanes->currentlane];

}

void movemap(struct level_struct *level, struct player_struct *player_ptr, struct xy_struct grid, SDL_Rect rcTile[grid.x * 3][grid.y], SDL_Rect rcTilemid[grid.x * 3][grid.y], SDL_Rect rcTSrc[grid.x][grid.y], SDL_Rect rcTSrcmid[grid.x][grid.y], int totallanes, int (*screenstrip [level->maxscreens]) [grid.x * 3][grid.y][2], int (*itemscreenstrip[level->maxscreens])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2], struct monster *(*bestiary)[10], struct item *(*itempokedex)[10]){
	if (player_ptr->flydir == 0){
		for (int i = 0; i < grid.x * 3; i++){
			for(int j = 0; j < grid.y; j++){
				rcTile[i][j].y += 4 * ZOOM_MULT * level->speedmult;
				rcTilemid[i][j].y += 4 * ZOOM_MULT * level->speedmult;
			}
		}
	}
	if (player_ptr->flydir == 1 ){
		if (rcTile[grid.x][0].x <= 0 ) {
			int frameoffset = rcTile[grid.x][0].x;
			(level->currentscreen)++;
			if (level->currentscreen >= level->maxscreens - 3){
				level->levelover = 1;
			}
			refreshtiles(totallanes, screenstrip, itemscreenstrip, level->currentscreen, grid, rcTile, rcTilemid, rcTSrc, rcTSrcmid, frameoffset, level->lanes.laneheight, *bestiary, *itempokedex);
		}

		if ( !(level->levelover) ) {
			for (int i = 0; i < grid.x * 3; i++){
				for(int j = 0; j < grid.y; j++){
					rcTile[i][j].x -= 4 * ZOOM_MULT * level->speedmult;
					rcTilemid[i][j].x -= 4 * ZOOM_MULT * level->speedmult;
					level->totalnativedist += 4 * level->speedmult;
				}
			}
			for ( int screen = 0; screen < 3; screen++ ) {
				for (int lane = 0; lane < level->lanes.total; lane++ ) {
					for (int i = 0; i < itemlanenum[screen][lane]; i++ ) {
						rcItem[screen][lane][i].x -= 4 * ZOOM_MULT * level->speedmult;
					}
				}
			}
		}
	}

	if (player_ptr->flydir == 2){
		for (int i = 0; i < grid.x * 3; i++){
			for(int j = 0; j < grid.y; j++){
				rcTile[i][j].y -= 4 * ZOOM_MULT * level->speedmult;
				rcTilemid[i][j].y -= 4 * ZOOM_MULT * level->speedmult;
			}
		}
	}

	if (player_ptr->flydir == 3){
		for (int i = 0; i < grid.x * 3; i++){
			for(int j = 0; j < grid.y; j++){
				rcTile[i][j].x += 4 * ZOOM_MULT * level->speedmult;
				rcTilemid[i][j].x += 4 * ZOOM_MULT * level->speedmult;
			}
		}
	}


}


void movemon(int totallanes, float speedmultmon, struct time_struct timing, struct monster_node *linkptrs_start[totallanes], struct monster_node *linkptrs_end[totallanes], int monsterlanenum[totallanes], double remainder[totallanes], SDL_Rect player_out, SDL_Rect rcSword, int width) {

	struct monster_node *ptr2mon;
	float transspeed;
	for (int lane = 0; lane < totallanes; lane++) {
		if (linkptrs_start[lane] != NULL) {
			ptr2mon = linkptrs_start[lane];
			for (int i = 0; i < monsterlanenum[lane]; i++) {
				transspeed = timing.bps * linkptrs_start[lane]->speed * ZOOM_MULT * speedmultmon * timing.pxperbeat + ptr2mon->remainder;
				float translation = transspeed * timing.intervalglobal/1000.0;
				int transint = (int)translation;
				ptr2mon->remainder = translation - transint;
				ptr2mon->monster_rect.x -= transint;
				ptr2mon = ptr2mon->next;
			}
			while (linkptrs_start[lane]->monster_rect.x + linkptrs_start[lane]->monster_rect.w < 0) {
				struct monster_node *tmp = linkptrs_start[lane];
				linkptrs_start[lane] = linkptrs_start[lane]->next;
				monsterlanenum[lane]--;
				free(tmp);
				if (linkptrs_start[lane] == NULL)
					break;
			}
			float Dt;
			while (linkptrs_start[lane] != NULL && linkptrs_end[lane] != NULL) {
				transspeed = timing.bps * linkptrs_start[lane]->speed * ZOOM_MULT * speedmultmon * timing.pxperbeat + ptr2mon->remainder;
				float extra = (((float)(width - (player_out.x + player_out.w + rcSword.w/2)))/transspeed - 4*timing.intervalglobal/1000.0) * timing.bps;
				Dt = timing.currentbeat - linkptrs_end[lane]->entrybeat + extra;
				if (Dt >= 0) {
					float Dx = Dt / timing.bps * speedmultmon * linkptrs_end[lane]->speed;
					linkptrs_end[lane]->monster_rect.x = width - Dx;
					linkptrs_end[lane] = linkptrs_end[lane]->next;
					monsterlanenum[lane]++;
				}
				else
					break;
			}
		}
	}

}


void refreshtiles(int totallanes, int (*screenstrip[]) [grid.x][grid.y][2], int (*itemscreenstrip[])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int currentscreen, struct xy_struct grid, SDL_Rect rcTile[grid.x][grid.y], SDL_Rect rcTilemid[grid.x][grid.y], SDL_Rect rcTSrc[grid.x][grid.y], SDL_Rect rcTSrcmid[grid.x][grid.y], int frameoffset, float laneheight[totallanes], struct monster *bestiary[10], struct item *itempokedex[10]) {

	for (int k = 0; k <= 2; k++) {
		for (int i = 0; i < grid.x; i++){
			for (int j = 0; j < grid.y; j++){

				rcTile[i + grid.x * k][j].x = (i + grid.x * k) * ZOOM_MULT * TILE_SIZE + frameoffset;
				rcTile[i + grid.x * k][j].y = (j) * ZOOM_MULT * TILE_SIZE;
				rcTile[i + grid.x * k][j].w = TILE_SIZE*ZOOM_MULT * 2;
				rcTile[i + grid.x * k][j].h = TILE_SIZE*ZOOM_MULT * 2;
				rcTSrc[i + grid.x * k][j].x = TILE_SIZE*(*screenstrip[currentscreen + k])[i][j][1]/2;
				rcTSrc[i + grid.x * k][j].y = TILE_SIZE*(*screenstrip[currentscreen + k])[i][j][0]/2;
				rcTSrc[i + grid.x * k][j].w = TILE_SIZE;
				rcTSrc[i + grid.x * k][j].h = TILE_SIZE;
			}
		}
	}

	//}

if ( currentscreen != 0 ) {
	void *ptrtemp = iteminfoptrs[0];
	iteminfoptrs[0] = iteminfoptrs[1];
	iteminfoptrs[1] = iteminfoptrs[2];
	iteminfoptrs[2] = ptrtemp;
}


int itemcounter;
int itemcounterperscreen;
for (int lane = 0; lane < totallanes; lane++){
	itemcounter = 0;
	for (int screen = 0; screen <= 2; screen++) {
		itemcounterperscreen = 0;
		for (int j = 0; j < MAX_ITEMS_PER_LANE_PER_SCREEN; j++){
			if ((*itemscreenstrip[currentscreen + screen])[lane][j][0] != -1 ) {
				rcItemSrc[screen][lane][itemcounterperscreen].x = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).Src[0];// * ZOOM_MULT;
				rcItemSrc[screen][lane][itemcounterperscreen].y = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).Src[1];// * ZOOM_MULT;
				rcItemSrc[screen][lane][itemcounterperscreen].w = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).wh[0];// * ZOOM_MULT;
				rcItemSrc[screen][lane][itemcounterperscreen].h = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).wh[1];// * ZOOM_MULT;

				rcItem[screen][lane][itemcounterperscreen].w = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).wh[0] * ZOOM_MULT * 2;
				rcItem[screen][lane][itemcounterperscreen].h = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).wh[1] * ZOOM_MULT * 2;
				rcItem[screen][lane][itemcounterperscreen].x = ( (*itemscreenstrip[currentscreen + screen])[lane][j][1] + grid.x * screen * TILE_SIZE) * ZOOM_MULT + frameoffset;
				rcItem[screen][lane][itemcounterperscreen].y = laneheight[lane] - rcItem[screen][lane][itemcounterperscreen].h / 2;

				if ( currentscreen == 0 || screen == 2 ) {
					(*iteminfoptrs[screen])[lane][itemcounterperscreen] = 0;
				}

				itemcounter++;
				itemcounterperscreen++;
			}
		}
		itemlanenum[screen][lane] = itemcounterperscreen;
	}
}
}

void laserfire(struct level_struct *level, int totallanes, struct laser_struct *laser, struct player_struct *player, SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect player_out, float laneheight[totallanes], int currentlane, int framecount, int currentscreen, int hue, int *soundchecklist) {
	if (laser->turnon) {
		(laser->count)++;

		if (laser->count >= 4) {
			laser->turnon = 0;
		}
	}

	else if (laser->turnoff) {
		(laser->count)--;

		if (laser->count <= 0) {
			laser->on = 0;
			laser->turnoff = 0;
			return;
		}
	}



	int lasersrcpos;
	lasersrcpos = (laser->count);
	if (laser->count >= 4 && framecount%4 == 0)
		lasersrcpos --;
	rcLaserSrc[0].x = LASER_SEPARATOR_X * 0 + 120 * hue;
	rcLaserSrc[0].y = LASER_SEPARATOR_Y * lasersrcpos;
	rcLaserSrc[0].w = LASER_SEPARATOR_X - 1;
	rcLaserSrc[0].h = LASER_HEIGHT;

	rcLaserSrc[1].x = LASER_SEPARATOR_X * 1 + 17 + 120 * hue;
	rcLaserSrc[1].y = LASER_SEPARATOR_Y * lasersrcpos;
	rcLaserSrc[1].w = 1;
	rcLaserSrc[1].h = LASER_HEIGHT;

	rcLaserSrc[2].x = LASER_SEPARATOR_X * 2 + 120 * hue;
	rcLaserSrc[2].y = LASER_SEPARATOR_Y * lasersrcpos;
	rcLaserSrc[2].w = LASER_SEPARATOR_X;
	rcLaserSrc[2].h = LASER_HEIGHT;

	int laserlength = NATIVE_RES_X * ZOOM_MULT - player_out.x - player_out.w + rcLaser[2].w;

	int done = 0;
	struct monster_node *ptr2mon = linkptrs_start[currentlane];
	for ( int i = 0; i < monsterlanenum[currentlane]; i++ ) {
		if ( ptr2mon->monster_rect.x > ( player_out.x + player_out.w ) && (ptr2mon->status != -1)){
			if ( ptr2mon->monster_rect.x <= NATIVE_RES_X * ZOOM_MULT ) {
				laserlength = - player_out.x - player_out.w + ptr2mon->monster_rect.x;
				damage(currentlane, ptr2mon, laser->power, soundchecklist, level);
			}
			break;
		}
		ptr2mon = ptr2mon->next;
	}

	rcLaser[0].x = 0.24 * NATIVE_RES_X * ZOOM_MULT;
	rcLaser[0].y = laneheight[currentlane] - LASER_HEIGHT * ZOOM_MULT/2;
	rcLaser[0].w = LASER_SEPARATOR_X * ZOOM_MULT;
	rcLaser[0].h = LASER_HEIGHT * ZOOM_MULT;

	rcLaser[1].x = rcLaser[0].x + rcLaser[0].w;
	rcLaser[1].y = laneheight[currentlane] - LASER_HEIGHT * ZOOM_MULT/2;
	rcLaser[1].w = laserlength - rcLaser[2].w;
	rcLaser[1].h = LASER_HEIGHT * ZOOM_MULT;

	rcLaser[2].x = rcLaser[1].x + rcLaser[1].w;
	rcLaser[2].y = laneheight[currentlane] - LASER_HEIGHT * ZOOM_MULT/2;
	rcLaser[2].w = LASER_SEPARATOR_X * ZOOM_MULT;
	rcLaser[2].h = LASER_HEIGHT * ZOOM_MULT;

	player->living.power--;
}

void swordfunc(struct level_struct *level, struct sword_struct *sword, SDL_Rect player_out, int framecount, struct monster_node *linkptrs_start[TOTAL_LANES], struct audio_struct audio) {

	struct lane_struct *lanes = &level->lanes;
	
	sword->power = 30000;

	sword->rect_out.x = player_out.x + player_out.w - 2 * ZOOM_MULT;
	sword->rect_out.y = lanes->laneheight[lanes->currentlane] - ( SWORD_HEIGHT + 18 ) * ZOOM_MULT;
	sword->rect_out.w = sword->rect_in.w * ZOOM_MULT * 2;
	sword->rect_out.h = sword->rect_in.h * ZOOM_MULT * 2;

	if ( sword->swing ) {

		if ( sword->down ) {
			(sword->count)--;
			if ( sword->count <= 0 ) {
				sword->down = 0;
			}
		}

		else if ( !(sword->down) ) {

			if ( sword->count == 0 ) {
				audio.soundchecklist[8] = 1;
			}

			(sword->count)++;

			if ( sword->count >= 2 ) {
				sword->down = 1;

				struct monster_node *ptr2mon = linkptrs_start[lanes->currentlane];
				for ( int i = 0; i < monsterlanenum[lanes->currentlane]; i++ ) {
					if ( ptr2mon->monster_rect.x > ( player_out.x + player_out.w ) && (ptr2mon->status != -1)){
						if ( ptr2mon->monster_rect.x <= sword->rect_out.x + sword->rect_out.w ) {
							damage(lanes->currentlane, ptr2mon, sword->power, audio.soundchecklist, level);
						}
						else {
							break;
						}
					}
					ptr2mon = ptr2mon->next;
				}

			}
		}
	}



	sword->rect_in.y = SWORD_HEIGHT * sword->count;
}

void damage(int currentlane, struct monster_node *ptr2mon, int power, int *soundchecklist, struct level_struct *level) {

	ptr2mon->health -= power;
	if (ptr2mon->health <= 0){
		ptr2mon->status = -1;
		soundchecklist[7] = 1;
		level->score += 10;
	}
}

void amihurt(int totallanes, struct status_struct status, struct monster_node *linkptrs_start[totallanes], SDL_Rect player_out, struct monster *bestiary[10]) {

	/* Check screen 0, current lane for monster sprites encroaching on the player sprite */

	struct monster_node *ptr2mon = linkptrs_start[status.level->lanes.currentlane];

	for ( int i = 0; i < monsterlanenum[status.level->lanes.currentlane]; i++ ) {

		if ( ( ptr2mon->monster_rect.x > player_out.x ) && ( ptr2mon->monster_rect.x < ( player_out.x + player_out.w ) ) ) {

			/* Is the monster dead? */
			if ( ptr2mon->status != -1) {

				int monstertype = ptr2mon->montype;
				gethurt(status, (*bestiary[monstertype]).attack);
			}
			break;
		}
		ptr2mon = ptr2mon->next;
	}

}


void touchitem(struct lane_struct *lanes, int currentlane, int currentscreen, SDL_Rect player_out, struct item *itempokedex[10], int (*itemscreenstrip[])[lanes->total][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int *levelover, int *soundchecklist) {
	/* Check screen 0, current lane for monster sprites encroaching on the player sprite */

	int done = 0;
	for ( int screen = 0; screen < 3; screen++ ) {

		for ( int i = 0; i < itemlanenum[screen][currentlane]; i++ ) {

			if ( ( rcItem[screen][currentlane][i].x > player_out.x ) && ( rcItem[screen][currentlane][i].x < ( player_out.x + player_out.w ) ) ) {

				/* Is the item "dead"? */
				if ( (*iteminfoptrs[screen])[currentlane][i] != -1) {

					int order = i;
					int itemtype = (*itemscreenstrip[currentscreen + screen])[currentlane][i][0];
					struct item thisitem = *itempokedex[itemtype];
					void (*functionptr)(int *int1, int int2, void *otherdata) = thisitem.functionptr;
					(*functionptr)(thisitem.int1, thisitem.int2, thisitem.otherdata);
					(*iteminfoptrs[screen])[currentlane][i] = -1;
					soundchecklist[6] = 1;
				}
				done = 1;
				break;
			}
		}
		if ( done )
			break;
	}
}


int invinciblefunc(struct player_struct *player) {

	if ( player->living.invincibility != 0 ) {
		player->invinciblecounter[player->living.invincibility]--;
		if ( player->invinciblecounter[player->living.invincibility] <= 0 ) {
			player->living.invincibility = 0;
			player->living.invincibility_toggle = 1;
		}
	}

	return player->living.invincibility;
}

void gethurt(struct status_struct status, int attack) {

	if (status.player->living.invincibility == 0 ) {
		int damage = attack;
		status.player->living.HP -= damage;
		if ( status.player->living.HP <= 0 ) {
			status.audio->soundchecklist[5] = 1;
			status.level->levelover = 1;
		}
		else {
			status.audio->soundchecklist[4] = 1;
			status.player->living.invincibility = 1;
			status.player->living.invincibility_toggle = 1;
			status.player->invinciblecounter[1] = 40;
		}
	}
}

void restorehealth(int *HPptr, int restoreHPpts, void *otherdata) {
	struct player_struct *player = (struct player_struct *) otherdata;
	if ( player->living.max_HP - *HPptr <= restoreHPpts ) {
		*HPptr = player->living.max_HP;
	}
	else
		*HPptr += restoreHPpts;

}

void restorepower(int *powerptr, int restorepowerpts, void *otherdata) {

	struct player_struct *player = (struct player_struct *)otherdata;
	if ( player->living.max_PP - *powerptr <= restorepowerpts ) {
		*powerptr = player->living.max_PP;
	}
	else
		*powerptr += restorepowerpts;

}


void healthup(int *max_HPptr, int HPuppts, void *nullptr) {

	*max_HPptr += HPuppts;
}

void PPup(int *max_PPptr, int PPuppts, void *nullptr) {

	*max_PPptr += PPuppts;
}
