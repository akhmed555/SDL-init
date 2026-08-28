#include "display.h"

bool initialize_window(void) {
    // 1. Инициализируем только видео (SDL_INIT_EVERYTHING в SDL3 удален)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Ошибка SDL_Init: %s\n", SDL_GetError());
        return false;
    }
    SDL_DisplayID primary = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode;
    mode = SDL_GetCurrentDisplayMode(primary);

		SDL_Log("=== SDL_DisplayMode ===");
		SDL_Log("displayID    : %u", mode->displayID);
		SDL_Log("format       : %s", SDL_GetPixelFormatName(mode->format));
		SDL_Log("w            : %d px", mode->w);
		SDL_Log("h            : %d px", mode->h);
		SDL_Log("pixel_density: %.2f", mode->pixel_density);
		SDL_Log("refresh_rate : %.3f Hz", mode->refresh_rate);
		SDL_Log("driverdata   : [internal pointer, ignore]");

// Физическое разрешение (если нужно)
		SDL_Log("Physical res : %dx%d px", 
        (int)(mode->w * mode->pixel_density), 
        (int)(mode->h * mode->pixel_density));

    // 2. Создаем окно (в SDL3 координаты задаются отдельно или флагами)
    window = SDL_CreateWindow("SDL3 Boilerplate", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);
    if (!window) {
        fprintf(stderr, "Ошибка создания окна: %s\n", SDL_GetError());
        return false;
    }
    
    // Центрируем окно (новый способ в SDL3)
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    
    // 3. Создаем рендерер (NULL означает автоматический выбор лучшего драйвера)
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "Ошибка создания рендерера: %s\n", SDL_GetError());
        return false;
    }

    // Включаем вертикальную синхронизацию для плавности (убирает разрывы кадров)
    SDL_SetRenderVSync(renderer, 1);

	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    return true;
}


void draw_grid(void){
	for(int y = 0;y < SCREEN_HEIGHT ;y++){
		for(int x = 0; x < SCREEN_WIDTH;x++){
			
			if(x % 3 == 0 || y % 3 == 0){
				color_buffer[(SCREEN_WIDTH * y)+x] = 0xff333333;
				}
			
			}
		
		
		}
	
	}
	
	void draw_rect(int x,int y,int width,int height,uint32_t color){
		
		for(int i = 0; i<width;i++){
			for(int j = 0; j<height; j++){
				int current_x = x + i;
				int current_y =  y + j;
				color_buffer[(SCREEN_WIDTH*current_y)+current_x] = color;
				}
			}
		
		}
		
void setup(void) {
    // Здесь инициализируем игровые объекты, загружаем текстуры и т.д.
    
    color_buffer = (uint32_t*)malloc(sizeof(uint32_t)*SCREEN_WIDTH*SCREEN_HEIGHT);
    color_buffer_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    SCREEN_WIDTH,
    SCREEN_HEIGHT
    
    );
    
    
}

void process_input(void) {
    SDL_Event event;
    
    // ВАЖНО: используем while, а не if, чтобы обработать ВСЕ события за кадр 
    // и предотвратить лаги ввода при быстром нажатии клавиш.
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            is_running = false;
        } 
        else if (event.type == SDL_EVENT_KEY_DOWN) {
            // В SDL3 event.key.keysym.sym заменен на event.key.key
            if (event.key.key == SDLK_ESCAPE) {
                is_running = false;
            }
        }
    }
}

void update(void) {
    // Здесь обновляем логику игры (движение, физика, ИИ)
}

 void clear_color_buffer(uint32_t color){
	 for(int y = 0; y<SCREEN_HEIGHT;y++)
	 for(int x = 0;x<SCREEN_WIDTH ;x++){
		 color_buffer[(SCREEN_WIDTH*y)+x] = color;
		 }
	 
	 }

	 
void render_color_buffer(void){
	
	SDL_UpdateTexture(
	color_buffer_texture,
	NULL,
	color_buffer,
	(int)(SCREEN_WIDTH*sizeof(uint32_t))
	);
	
	SDL_RenderTexture(renderer,color_buffer_texture,NULL,NULL);
	
	}

	Uint32 rand_color(void) {
    Uint8 r = rand() % 256;  // 0-255
    Uint8 g = rand() % 256;
    Uint8 b = rand() % 256;
    
    // Сдвигаем байты на нужные позиции и объединяем
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

void render(void) {
	
    // Очищаем экран красным цветом
    SDL_SetRenderDrawColor(renderer, 255, 19, 0,0 );
    SDL_RenderClear(renderer);
    draw_grid();
    draw_rect(300,200,300,150,0xffff00ff);
    
    //for(int i=0;i<2000;i++){
	draw_rect(rand()%100,rand()%60,rand()%400,rand()%400,rand_color());
	//}
    //
    render_color_buffer();
    clear_color_buffer(0x6c8cd5); 

    // ... здесь будет отрисовка ваших объектов ...

    // Показываем отрисованное на экране
    SDL_RenderPresent(renderer);
}	

void cleanup(void) {
    // Правильная очистка ресурсов перед выходом
    free(color_buffer);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}
		 