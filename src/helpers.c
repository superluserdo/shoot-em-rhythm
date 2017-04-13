#include <stdio.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include "main.h"
#include "level.h"
#include "helpers.h"



void int2array(int number, int (*array)[SCORE_DIGITS]) {
        int digits = 1;
        int numchecker = number;
        while (numchecker /= 10)
                digits++;

        for (int i = 0; i < SCORE_DIGITS; i++ )
                (*array)[i] = 0;

        int diff = SCORE_DIGITS - digits;
        for (int i = 0; i < SCORE_DIGITS; i++) {
                if ( i < diff ) {
                        (*array)[i] = 0;
                }
                else {
                        (*array)[SCORE_DIGITS - (i - diff) - 1] = number%10;
                        number /= 10;
                }
        }

}


/* Function to delete the entire linked list [geeksforgeeks.org]*/
void deleteList(struct node** head_ref) {
   /* deref head_ref to get the real head */
   struct node* current = *head_ref;
   struct node* next;

   while (current != NULL) 
   {
       next = current->next;
       free(current);
       current = next;
   }

   /* deref head_ref to affect the real head back
      in the caller. */
   *head_ref = NULL;
}



