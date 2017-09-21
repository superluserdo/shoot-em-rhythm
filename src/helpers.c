#include <stdio.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include "main.h"
#include "level.h"
#include "helpers.h"

void int2array(int number, int *array, int array_digits) {
        int digits = 1;
        int numchecker = number;
        while (numchecker /= 10)
                digits++;

        for (int i = 0; i < array_digits; i++ )
                array[i] = 0;

        int diff = array_digits - digits;
        for (int i = 0; i < array_digits; i++) {
                if ( i < diff ) {
                        array[i] = 0;
                }
                else {
                        array[array_digits - (i - diff) - 1] = number%10;
                        number /= 10;
                }
        }

}

void deleteList(struct monster_node** head_ref) {
   struct monster_node* current = *head_ref;
   struct monster_node* next;

   while (current != NULL) 
   {
       next = current->next;
       free(current);
       current = next;
   }
   *head_ref = NULL;
}



