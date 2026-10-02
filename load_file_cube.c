/* ============================================================================
 * 🎮 3D-ДВИЖОК НА SDL3: РЕНДЕРИНГ КУБА ТРЕУГОЛЬНИКАМИ
 * ============================================================================
 * Этот файл объединяет все компоненты 3D-движка:
 * - Векторная математика (вращение, проекция)
 * - Mesh-данные (вершины и грани куба)
 * - Software-рендеринг через цветовой буфер
 * - Игровой цикл с контролем FPS
 * 
 * Принцип работы:
 * 1. Куб задан 8 вершинами и 12 треугольниками (2 на каждую из 6 граней)
 * 2. Каждый кадр вращаем все вершины вокруг осей X, Y, Z
 * 3. Проецируем 3D координаты на 2D экран (с перспективой /z)
 * 4. Рисуем треугольники как наборы пикселей в буфере
 * 5. Копируем буфер на экран через SDL_Texture
 * ============================================================================ */

/* ============================================================================
 * ЧАСТЬ 1: ПОДКЛЮЧЕНИЕ БИБЛИОТЕК
 * ============================================================================ */
#include <stdio.h>
#include <stdlib.h>     // malloc, free
#include <stdint.h>     // uint32_t, uint8_t
#include <stdbool.h>    // bool, true, false
#include <math.h>       // cos, sin
#include <locale.h>     // setlocale
#include <SDL3/SDL.h>   // SDL3 API
#include <SDL3/SDL_main.h> // Корректный main() для Windows
#include "array.c" // Для использования динамического массива
#include <math.h>

/* ============================================================================
 * ЧАСТЬ 2: КОНСТАНТЫ И МАКРОСЫ
 * ============================================================================ */
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)  // ~16.67 мс на кадр

// Количество вершин и граней куба
#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2)  // 6 граней × 2 треугольника на грань = 12

/* ============================================================================
 * ЧАСТЬ 3: ТИПЫ ДАННЫХ (ВЕКТОРЫ И ГЕОМЕТРИЯ)
 * ============================================================================ */

/**
 * @brief 2D вектор (точка на экране)
 */
typedef struct {
    float x;
    float y;
} vec2_t;

/**
 * @brief 3D вектор (точка в пространстве)
 */
typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

/**
 * @brief Грань (треугольник) из трёх индексов вершин
 * 
 * Каждая грань куба — это треугольник, заданный тремя индексами
 * вершин из массива mesh_vertices.
 */
typedef struct {
    int a;  ///< Индекс первой вершины (1-based)
    int b;  ///< Индекс второй вершины
    int c;  ///< Индекс третьей вершины
} face_t;

/**
 * @brief Треугольник для рендеринга (спроецированный)
 */
typedef struct {
    vec2_t points[3];  ///< Три 2D точки треугольника
} triangle_t;


typedef struct {
    vec3_t* vertices; // dynamic array of vertices
    face_t* faces;    // dynamic array of faces
    vec3_t rotation;  // rotation with x, y, and z values
} mesh_t;

/* ============================================================================
 * ЧАСТЬ 4: ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
 * ============================================================================ */

// --- Mesh-данные (геометрия куба) ---


mesh_t mesh ={
	.vertices = NULL,
	.faces = NULL,
	.rotation = {0,0,0}
};


float vec2_length(vec2_t v){
	return sqrt(v.x * v.x + v.y * v.y);
}


float vec3_length(vec3_t v){
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec2_t vec2_add(vec2_t a,vec2_t b){
	
	vec2_t result ={
		.x = a.x + b.x,
		.y = a.y + b.y
	};
	
	return result;
}
vec2_t vec2_sub(vec2_t a,vec2_t b){
	
	vec2_t result ={
		.x = a.x - b.x,
		.y = a.y - b.y
	};
	
	return result;
	
	
}


vec3_t vec3_add(vec3_t a,vec3_t b){
	
	vec3_t result ={
		.x = a.x + b.x,
		.y = a.y + b.y,
		.z = a.z + b.z
	};
	
	return result;
	
}
vec3_t vec3_sub(vec3_t a,vec3_t b){
	
	vec3_t result ={
		.x = a.x - b.x,
		.y = a.y - b.y,
		.z = a.z - b.z
	};
	
	return result;
}




vec2_t vec2_mul(vec2_t v,float factor){
	vec2_t result = {
		.x = v.x * factor,
		.y = v.y * factor 
		};
		return result;
	}
	
vec2_t vec2_div(vec2_t v,float factor)	{
	vec2_t result = {
		.x = v.x / factor,
		.y = v.y / factor 
		};
		return result;
	}


vec3_t vec3_mul(vec3_t v,float factor){
	vec3_t result = {
		.x = v.x * factor,
		.y = v.y * factor,
		.z = v.z * factor 
		};
		return result;
	}
	
vec3_t vec3_div(vec3_t v,float factor)	{
	vec3_t result = {
		.x = v.x / factor,
		.y = v.y / factor,
		.z = v.z / factor 
		};
		return result;
	}



/**
 * @brief Вершины куба (8 углов)
 * 
 * Куб centered в (0,0,0) с размером 2 единицы.
 * Координаты от -1 до 1 по каждой оси.
 */
vec3_t cube_vertices[N_CUBE_VERTICES] = {
    { .x = -1, .y = -1, .z = -1 },  // 0: Задний-нижний-левый
    { .x = -1, .y =  1, .z = -1 },  // 1: Задний-верхний-левый
    { .x =  1, .y =  1, .z = -1 },  // 2: Задний-верхний-правый
    { .x =  1, .y = -1, .z = -1 },  // 3: Задний-нижний-правый
    { .x =  1, .y =  1, .z =  1 },  // 4: Передний-верхний-правый
    { .x =  1, .y = -1, .z =  1 },  // 5: Передний-нижний-правый
    { .x = -1, .y =  1, .z =  1 },  // 6: Передний-верхний-левый
    { .x = -1, .y = -1, .z =  1 }   // 7: Передний-нижний-левый
};



	
	
	mesh_t mesh;
	

/**
 * @brief Грани куба (12 треугольников)
 * 
 * Каждая грань куба разбита на 2 треугольника.
 * Индексы вершин 1-based (как в .obj файлах).
 * 
 * Порядок обхода: против часовой стрелки при взгляде снаружи
 * (важно для backface culling и нормалей).
 */
face_t cube_faces[N_CUBE_FACES] = {
    // Передняя грань (z = 1)
    { .a = 8, .b = 7, .c = 6 },  // Треугольник 1
    { .a = 8, .b = 6, .c = 5 },  // Треугольник 2
    
    // Задняя грань (z = -1)
    { .a = 1, .b = 2, .c = 3 },
    { .a = 1, .b = 3, .c = 4 },
    
    // Правая грань (x = 1)
    { .a = 4, .b = 3, .c = 5 },
    { .a = 4, .b = 5, .c = 6 },
    
    // Левая грань (x = -1)
    { .a = 8, .b = 7, .c = 2 },
    { .a = 8, .b = 2, .c = 1 },
    
    // Верхняя грань (y = 1)
    { .a = 2, .b = 7, .c = 5 },
    { .a = 2, .b = 5, .c = 3 },
    
    // Нижняя грань (y = -1)
    { .a = 6, .b = 8, .c = 1 },
    { .a = 6, .b = 1, .c = 4 }
};


void load_cube_mesh_data(void){
	
	for(int i = 0; i< N_CUBE_VERTICES;i++){
		vec3_t cube_vertex = cube_vertices[i];
		array_push(mesh.vertices,cube_vertex);
	}
	
	
	for(int i = 0; i < N_CUBE_FACES; i++){
		face_t cube_face = cube_faces[i];
		array_push(mesh.faces,cube_face);
	}
	
}
// --- Состояние рендерера ---

/**
 * @brief Массив треугольников для отрисовки в текущем кадре
 * 
 * После трансформации и проекции все 12 граней куба
 * сохраняются сюда для последующей растеризации.
 */
triangle_t* triangles_to_render = NULL;

/**
 * @brief Позиция камеры в 3D пространстве
 * 
 * Камера находится на оси Z перед кубом (z = -5).
 * Отрицательное значение = камера "перед" объектом.
 */
vec3_t camera_position = { .x = 0, .y = 0, .z = -5 };

/**
 * @brief Текущие углы вращения куба (в радианах)
 * 
 * Увеличиваются каждый кадр на 0.01 рад (~0.57 градуса).
 * При 60 FPS полный оборот = ~10 секунд.
 */
vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0 };

/**
 * @brief Коэффициент поля зрения (FOV)
 * 
 * Чем больше значение, тем "шире" камера и меньше перспективное
 * искажение. 640 подобрано для экрана 800×600.
 */
float fov_factor = 640;

/**
 * @brief Флаг работы приложения
 * 
 * false = выход из главного цикла
 */
bool is_running = false;

// --- SDL-ресурсы ---

SDL_Window* window = NULL;           ///< Окно SDL3
SDL_Renderer* renderer = NULL;       ///< Рендерер SDL3
uint32_t* color_buffer = NULL;       ///< Буфер кадра в RAM (800×600×4 байта)
SDL_Texture* color_buffer_texture = NULL;  ///< Текстура для передачи на GPU

int window_width = SCREEN_WIDTH;     ///< Ширина окна
int window_height = SCREEN_HEIGHT;   ///< Высота окна
int previous_frame_time = 0;         ///< Время предыдущего кадра (для FPS control)

/* ============================================================================
 * ЧАСТЬ 5: ВЕКТОРНАЯ МАТЕМАТИКА (ВРАЩЕНИЕ)
 * ============================================================================
 * Матрицы поворота вокруг осей X, Y, Z.
 * 
 * Формула вращения вокруг оси X:
 *   y' = y·cos(θ) - z·sin(θ)
 *   z' = y·sin(θ) + z·cos(θ)
 * 
 * Это стандартные формулы из линейной алгебры для поворота
 * точки в 3D пространстве.
 */

/**
 * @brief Вращение 3D вектора вокруг оси X
 * @param v Исходный вектор
 * @param angle Угол в радианах
 * @return vec3_t Повёрнутый вектор
 */
vec3_t vec3_rotate_x(vec3_t v, float angle) {
    vec3_t rotated_vector = {
        .x = v.x,  // X не меняется при вращении вокруг X
        .y = v.y * cos(angle) - v.z * sin(angle),
        .z = v.y * sin(angle) + v.z * cos(angle)
    };
    return rotated_vector;
}

/**
 * @brief Вращение 3D вектора вокруг оси Y
 * @param v Исходный вектор
 * @param angle Угол в радианах
 * @return vec3_t Повёрнутый вектор
 */
vec3_t vec3_rotate_y(vec3_t v, float angle) {
    vec3_t rotated_vector = {
        .x = v.x * cos(angle) - v.z * sin(angle),
        .y = v.y,  // Y не меняется при вращении вокруг Y
        .z = v.x * sin(angle) + v.z * cos(angle)
    };
    return rotated_vector;
}

/**
 * @brief Вращение 3D вектора вокруг оси Z
 * @param v Исходный вектор
 * @param angle Угол в радианах
 * @return vec3_t Повёрнутый вектор
 */
vec3_t vec3_rotate_z(vec3_t v, float angle) {
    vec3_t rotated_vector = {
        .x = v.x * cos(angle) - v.y * sin(angle),
        .y = v.x * sin(angle) + v.y * cos(angle),
        .z = v.z  // Z не меняется при вращении вокруг Z
    };
    return rotated_vector;
}

/* ============================================================================
 * ЧАСТЬ 6: ПРОЕКЦИЯ 3D → 2D (ПЕРСПЕКТИВА)
 * ============================================================================
 * Перспективная проекция создаёт эффект глубины:
 * - Ближние объекты (малый z) кажутся больше
 * - Дальние объекты (большой z) кажутся меньше
 * 
 * Формула: screen_x = (fov × world_x) / world_z
 * 
 * Деление на z — это и есть "перспективное деление",
 * основа всей 3D-графики!
 */

/**
 * @brief Проекция 3D точки на 2D экран с перспективой
 * @param point 3D точка в пространстве (после вращения)
 * @return vec2_t 2D точка на экране
 * 
 * Важно: point.z должен быть > 0 (точка перед камерой).
 * Если z <= 0, произойдёт деление на ноль или инверсия.
 */
vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z,
    };
    return projected_point;
}


void load_obj_file_data(char * filename){
	
	FILE* file; 
	file = fopen(filename,"r");
	char line[1024];
	
	while(fgets(line,1024,file)){
		//printf("LINE= %s",line);
		if(strncmp(line,"v ",2)==0){
			vec3_t vertex;
			sscanf(line,"v %f %f %f",&vertex.x,&vertex.y,&vertex.z);
			array_push(mesh.vertices,vertex);
			
			
			
			int result = sscanf(
    line,
    "v %f %f %f",
    &vertex.x,
    &vertex.y,
    &vertex.z
);

printf(
    "result=%d | x=%f y=%f z=%f\n",
    result,
    vertex.x,
    vertex.y,
    vertex.z
);
			
			
		}
			
			if(strncmp(line,"f ",2)==0){
				
				int vertex_indices[3];
				int texture_indices[3];
				int normal_indices[3];
				sscanf(line,"f %d/%d/%d %d/%d/%d %d/%d/%d",
				&vertex_indices[0],&texture_indices[0],&normal_indices[0],
				&vertex_indices[1],&texture_indices[1],&normal_indices[1],
				&vertex_indices[2],&texture_indices[2],&normal_indices[2]
				
				);
				
				face_t face = {
					.a = vertex_indices[0],
					.b = vertex_indices[1],
					.c = vertex_indices[2]
					
					};
					
					array_push(mesh.faces,face);
				}
		
		}
	}

/* ============================================================================
 * ЧАСТЬ 7: ИНИЦИАЛИЗАЦИЯ SDL3
 * ============================================================================
 * Создаём окно и рендерер SDL3.
 * 
 * SDL3 изменения по сравнению с SDL2:
 * - SDL_Init(SDL_INIT_EVERYTHING) → SDL_Init(SDL_INIT_VIDEO)
 * - SDL_GetCurrentDisplayMode(0, &mode) → SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay())
 * - SDL_CreateRenderer(window, -1, 0) → SDL_CreateRenderer(window, NULL)
 */

/**
 * @brief Инициализация SDL3 и создание окна
 * @return true если успешно
 */
bool initialize_window(void) {
    /* Шаг 1: Инициализация видеоподсистемы SDL3 */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "❌ Ошибка SDL_Init: %s\n", SDL_GetError());
        return false;
    }
    printf("✅ SDL3 инициализирован\n");

    /* Шаг 2: Получение информации о дисплее (SDL3 API) */
    SDL_DisplayID primary = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(primary);
    
    SDL_Log("=== Информация о дисплее ===");
    SDL_Log("Разрешение: %dx%d px", mode->w, mode->h);
    SDL_Log("Частота: %.2f Hz", mode->refresh_rate);
    SDL_Log("Плотность: %.2f", mode->pixel_density);

    /* Шаг 3: Создание окна (SDL3 упрощённый синтаксис) */
    window = SDL_CreateWindow(
        "SDL3 3D Cube Engine",  // Заголовок
        SCREEN_WIDTH,           // Ширина
        SCREEN_HEIGHT,          // Высота
        SDL_WINDOW_BORDERLESS   // Флаги (без рамок)
    );
    
    if (!window) {
        fprintf(stderr, "❌ Ошибка создания окна: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }
    printf("✅ Окно создано: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);

    /* Шаг 4: Центрирование окна */
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    /* Шаг 5: Создание рендерера (SDL3) */
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "❌ Ошибка создания рендерера: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    printf("✅ Рендерер создан\n");

    return true;
}

/* ============================================================================
 * ЧАСТЬ 8: ВЫДЕЛЕНИЕ РЕСУРСОВ (SETUP)
 * ============================================================================ */

/**
 * @brief Выделение памяти под цветовой буфер и создание SDL-текстуры
 */
void setup(void) {
    /* Шаг 1: Выделение памяти под буфер кадра
     * 800 × 600 × 4 байта = 1,920,000 байт (~1.8 MB)
     */
    color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
    if (!color_buffer) {
        fprintf(stderr, "❌ Не удалось выделить память\n");
        exit(1);
    }
    printf("✅ Цветовой буфер выделен: %d байт\n", window_width * window_height * 4);

    /* Шаг 2: Создание SDL-текстуры для передачи буфера на GPU
     * SDL_PIXELFORMAT_ARGB8888 = 4 байта на пиксель (Alpha, Red, Green, Blue)
     * SDL_TEXTUREACCESS_STREAMING = частое обновление (каждый кадр)
     */
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );
	
	//load_cube_mesh_data();
	load_obj_file_data("assets/cube.obj");
	printf("%d yyyyyyyyyyyyyyyy",array_length(mesh.faces));
    
    if (!color_buffer_texture) {
        fprintf(stderr, "❌ Ошибка создания текстуры: %s\n", SDL_GetError());
        exit(1);
    }
    printf("✅ SDL_Texture создана\n");
}

/* ============================================================================
 * ЧАСТЬ 9: ОБРАБОТКА ВВОДА (SDL3 EVENTS)
 * ============================================================================
 * SDL3 изменения:
 * - SDL_QUIT → SDL_EVENT_QUIT
 * - SDL_KEYDOWN → SDL_EVENT_KEY_DOWN
 * - event.key.keysym.sym → event.key.key
 */

/**
 * @brief Обработка событий ввода
 * 
 * Проверяет:
 * - Закрытие окна (крестик)
 * - Нажатие ESC
 */
void process_input(void) {
    SDL_Event event;
    
    /* Обрабатываем ВСЕ события в очереди (while, а не if!) */
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            is_running = false;
            printf("👋 Закрытие окна...\n");
        }
        else if (event.type == SDL_EVENT_KEY_DOWN) {
            /* SDL3: event.key.key вместо event.key.keysym.sym */
            if (event.key.key == SDLK_ESCAPE) {
                is_running = false;
                printf("👋 Нажат ESC\n");
            }
        }
    }
}

/* ============================================================================
 * ЧАСТЬ 10: ОБНОВЛЕНИЕ ЛОГИКИ (UPDATE)
 * ============================================================================
 * Каждый кадр:
 * 1. Контроль FPS (ожидание до следующего кадра)
 * 2. Вращение куба (увеличение углов)
 * 3. Трансформация всех вершин
 * 4. Проекция 3D → 2D
 * 5. Сохранение треугольников для рендеринга
 */

/**
 * @brief Обновление состояния игры
 * 
 * Включает:
 * - Контроль FPS (60 FPS)
 * - Вращение куба
 * - Трансформацию и проекцию всех вершин
 */
void update(void) {
    /* Шаг 1: Контроль FPS
     * Вычисляем, сколько времени прошло с прошлого кадра
     * и ждём, если нужно, чтобы достичь 60 FPS
     */
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    
    previous_frame_time = SDL_GetTicks();
    
    triangles_to_render = NULL;

    /* Шаг 2: Вращение куба
     * Увеличиваем углы на 0.01 радиан за кадр
     */
    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;
	
	int num_faces = array_length(mesh.faces);

    /* Шаг 3: Обработка всех 12 граней куба */
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];
        vec3_t face_vertices[3];
        
        /* Получаем 3 вершины текущей грани
         * Индексы в mesh_faces 1-based, поэтому -1
         */
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];
        
        triangle_t projected_triangle;
        
        /* Шаг 4: Трансформация и проекция каждой вершины */
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];
            
            /* a. Вращение вокруг всех трёх осей
             * Порядок важен: X → Y → Z
             */
            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);
            
            /* b. Сдвиг относительно камеры
             * Камера на z = -5, поэтому вычитаем (-5) = прибавляем 5
             * Это "отодвигает" куб от камеры
             */
            transformed_vertex.z -= camera_position.z;
            transformed_vertices[j]=transformed_vertex;
            
            /* c. Перспективная проекция
             * Деление на z создаёт эффект глубины
             */
		 }
            for(int j = 0;j < 3;j++){ 
            vec2_t projected_point = project(transformed_vertices[j]);
            
            /* d. Смещение в центр экрана
             * (0,0) → центр, а не левый верхний угол
             */
            projected_point.x += (window_width / 2);
            projected_point.y += (window_height / 2);
            
            projected_triangle.points[j] = projected_point;
        }
        
        /* Шаг 5: Сохраняем готовый треугольник */
       // triangles_to_render[i] = projected_triangle;
       array_push(triangles_to_render,projected_triangle);
    }
}

/* ============================================================================
 * ЧАСТЬ 11: РАСТЕРИЗАЦИЯ (РИСОВАНИЕ)
 * ============================================================================
 * Рисуем пиксели в цветовой буфер (RAM), затем копируем на экран.
 */

/**
 * @brief Рисование одного пикселя
 * @param x X-координата
 * @param y Y-координата
 * @param color Цвет в формате 0xAARRGGBB
 */
void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < window_width && y >= 0 && y < window_height) {
        color_buffer[(window_width * y) + x] = color;
    }
}

/**
 * @brief Рисование заполненного прямоугольника
 */
void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            draw_pixel(x + i, y + j, color);
        }
    }
}

/**
 * @brief Рисование линии (Line) для визуализации
 */
 
 void draw_line(int x0,int y0,int x1,int y1,uint32_t color)
 {
	int delta_x = (x1 - x0);
	int delta_y = (y1 - y0);
	
	int longest_side_length = (abs(delta_x) >=abs( delta_y)) ? abs(delta_x) : abs(delta_y);
	
	float x_inc = delta_x/(float)longest_side_length;
	float y_inc = delta_y/(float)longest_side_length;
	
	float current_x = x0;
	float current_y = y0;
	
	for(int i = 0; i <= longest_side_length;i++){
		draw_pixel(round(current_x),round(current_y),color);
		current_x += x_inc;
		current_y += y_inc;
		}
	 
	 }
 
/**
 * @brief Рисование треугольник (triangle) для визуализации
 */
 
 
 void draw_triangle(int x0,int y0,int x1,int y1,int x2,int y2,uint32_t color){
	 
	 draw_line(x0,y0,x1,y1,color);
	 draw_line(x1,y1,x2,y2,color);
	 draw_line(x2,y2,x0,y0,color);
	 }
 
 
 
/**
 * @brief Рисование сетки (grid) для визуализации
 */
void draw_grid(void) {
    for (int y = 0; y < window_height; y += 10) {
        for (int x = 0; x < window_width; x += 10) {
            color_buffer[(window_width * y) + x] = 0xFF444444;
        }
    }
}

/**
 * @brief Очистка буфера цветом
 * @param color Цвет фона
 */
void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            color_buffer[(window_width * y) + x] = color;
        }
    }
}


void free_resources(void){
	array_free(mesh.faces);
	array_free(mesh.vertices);
}
/**
 * @brief Копирование буфера на экран (через SDL_Texture)
 */
void render_color_buffer(void) {
    /* Шаг 1: Копируем RAM-буфер в SDL-текстуру */
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,  // Вся текстура
        color_buffer,
        (int)(window_width * sizeof(uint32_t))  // Pitch (байт на строку)
    );
    
    /* Шаг 2: Рисуем текстуру на экране (SDL3: SDL_RenderTexture) */
    SDL_RenderTexture(renderer, color_buffer_texture, NULL, NULL);
}

/* ============================================================================
 * ЧАСТЬ 12: ОСНОВНАЯ ФУНКЦИЯ ОТРИСОВКИ (RENDER)
 * ============================================================================ */

/**
 * @brief Отрисовка кадра
 * 
 * Последовательность:
 * 1. Очистка буфера (чёрный цвет)
 * 2. Рисование сетки (опционально)
 * 3. Рисование всех треугольников куба
 * 4. Копирование на экран
 * 5. SDL_RenderPresent (swap buffers)
 */
void render(void) {
    /* Шаг 1: Очистка буфера чёрным цветом */
    clear_color_buffer(0xFF000000);
    
    /* Шаг 2: Рисование сетки (фон) */
    draw_grid();
    int num_triangles = array_length(triangles_to_render);
    
    /* Шаг 3: Отрисовка всех 12 треугольников
     * Каждый треугольник = 3 точки (рисуем как маленькие квадраты 3×3)
     */
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];
        
        // Рисуем каждую из 3 вершин треугольника
        draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);
        
        draw_triangle(
        triangle.points[0].x,
        triangle.points[0].y,
        triangle.points[1].x,
        triangle.points[1].y,
        triangle.points[2].x,
        triangle.points[2].y,
        0XFF00FF00
        );
        
        
    }
    draw_line(100,200,300,50,0xff00ff00);
    
    array_free(triangles_to_render);
    /* Шаг 4: Копирование буфера на экран */
    render_color_buffer();
    
    /* Шаг 5: Показ кадра (swap buffers) */
    SDL_RenderPresent(renderer);
}

/* ============================================================================
 * ЧАСТЬ 13: ОЧИСТКА РЕСУРСОВ
 * ============================================================================ */

/**
 * @brief Освобождение всех ресурсов перед выходом
 */
void destroy_window(void) {
    printf(" Очистка ресурсов...\n");
    
    free(color_buffer);
    
    if (color_buffer_texture) {
        SDL_DestroyTexture(color_buffer_texture);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);free_resources();
    }
    
    SDL_Quit();
    printf("✅ SDL3 завершён\n");
}



/* ============================================================================
 * ЧАСТЬ 14: ГЛАВНАЯ ФУНКЦИЯ (MAIN)
 * ============================================================================ */

/**
 * @brief Точка входа в программу
 * 
 * Структура:
 * 1. Инициализация локали (кириллица)
 * 2. Создание окна SDL3
 * 3. Выделение ресурсов
 * 4. Главный цикл (input → update → render)
 * 5. Очистка
 */
int main(int argc, char *argv[]) {
    /* Шаг 1: Настройка локали для русского вывода */
    setlocale(LC_ALL, "C");
    printf("🚀 Запуск SDL3 3D Cube Engine...\n");
    
    /* Шаг 2: Инициализация окна */
    is_running = initialize_window();
    if (!is_running) {
        fprintf(stderr, "❌ Не удалось инициализировать SDL3\n");
        return 1;
    }
    
    /* Шаг 3: Выделение ресурсов */
    setup();
    printf("▶️  Запуск главного цикла (60 FPS)...\n");
    
    /* Шаг 4: ГЛАВНЫЙ ЦИКЛ
     * Работает 60 раз в секунду благодаря контролю FPS
     */
    while (is_running) {
        process_input();  // Обработка ESC и закрытия окна
        update();         // Вращение, трансформация, проекция
        render();         // Отрисовка треугольников
    }
    
    /* Шаг 5: Очистка */
    destroy_window();
    printf("✅ Программа завершена\n");
    
    return 0;
}

/* ============================================================================
 * 📚 ИТОГ: ЧТО ПРОИСХОДИТ В ЭТОМ КОДЕ
 * ============================================================================
 * 
 * 1. ГЕОМЕТРИЯ:
 *    - Куб задан 8 вершинами (углами) и 12 треугольниками (гранями)
 *    - Каждая грань = 2 треугольника
 * 
 * 2. КАЖДЫЙ КАДР (60 раз в секунду):
 *    
 *    🔄 UPDATE:
 *       - Ждём до следующего кадра (контроль 60 FPS)
 *       - Увеличиваем углы вращения (cube_rotation += 0.01)
 *       - Для каждой из 12 граней:
 *         · Берём 3 вершины
 *         · Вращаем вокруг X, Y, Z
 *         · Сдвигаем относительно камеры (z -= (-5))
 *         · Проецируем на экран (деление на z!)
 *         · Сохраняем в triangles_to_render[]
 *    
 *     RENDER:
 *       - Очищаем буфер (чёрный цвет)
 *       - Рисуем сетку (серые точки каждые 10 пикселей)
 *       - Для каждого треугольника:
 *         · Рисуем 3 маленькие квадрата 3×3 (вершины)
 *       - Копируем буфер на экран
 *       - SDL_RenderPresent (показываем кадр)
 * 
 * 3. МАТЕМАТИКА:
 *    - Вращение: матрицы поворота (cos/sin)
 *    - Проекция: perspective divide (x' = x/z, y' = y/z)
 *    - Это основа всей 3D-графики!
 * 
 * ============================================================================
 * 🚀 КОМПИЛЯЦИЯ:
 * ============================================================================
 * gcc -Wall -Wextra -std=c11 -O2 -o cube_3d main.c \
 *     -ID:/SDL/x86_64-w64-mingw32/include \
 *     -LD:/SDL/x86_64-w64-mingw32/lib \
 *     -lSDL3 -lm
 * 
 * Флаги:
 * - -Wall -Wextra: все предупреждения
 * - -std=c11: стандарт C11
 * - -O2: оптимизация
 * - -lSDL3: библиотека SDL3
 * - -lm: математическая библиотека (для cos/sin)
 * 
 * ============================================================================
 * 🎯 СЛЕДУЮЩИЕ УЛУЧШЕНИЯ:
 * ============================================================================
 * 1. Заполнение треугольников (fill triangle algorithm)
 * 2. Z-buffer (правильное перекрытие граней)
 * 3. Backface culling (не рисовать задние грани)
 * 4. Текстуры (UV-координаты)
 * 5. Освещение (нормали, dot product)
 * 6. Загрузка .obj моделей
 * 
 * Удачи с 3D-графикой! 🎮
 * ============================================================================
 */
