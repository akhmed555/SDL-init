#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> 
#include <stdint.h>
#include <locale.h>
#include "display.c"




int main(int argc, char* argv[]) {
	 setlocale(LC_ALL, ".utf8");
	 
	// uint32_t* buffer = NULL;
	 //buffer = (uint32_t*)malloc(sizeof(uint32_t)*100*5);
	 
    is_running = initialize_window();
    int sizeVar = sizeof(int);
    printf("SizeVar %d\n", sizeVar);

    if (is_running) {
        setup();

        while (is_running) {
            process_input();
            update();
            render();
        }

        cleanup();
    }

    return 0;
}
