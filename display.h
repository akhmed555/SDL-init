#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // Обязательно для корректной работы main в SDL3
#include <stdint.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

bool is_running = false;
SDL_Window* window = NULL; 
SDL_Renderer* renderer = NULL;
uint32_t*  color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;


bool initialize_window(void);
void draw_grid(void);
void draw_rect(int x,int y,int width,int height,uint32_t color);
void setup(void);
void process_input(void);
void update(void);
void render_color_buffer(void);
Uint32 rand_color(void);
void render(void);
void cleanup(void);



#endif
