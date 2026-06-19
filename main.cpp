#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "grafx/include/grafx.hpp"
#include <string>
#include <cmath>
#include <iostream>

/**
* @brief Основной класс.
* @details Инициализировать через init(), загрузить модель через loadModel(), запустить через run().
* Реализация отрисовки в render(), обработка ввода в handleEvents(), основной цикл в run().
* @param windowWidth, windowHeight Размер окна.
* @param running Статус.
* @param distance Расстояние до модели (любые значения).
* @param speed Скорость вращения.
*/
class SDLApp {
private:
    SDL_Window* window;
    int windowWidth, windowHeight;
    SDL_Renderer* renderer;
    bool running;

    Obj3D model;
    float rotationAngle, speed;

    Vec3D light;
    bool opaque;
    
    Vec3D cameraPos;
    float distance;

    /**
    * @brief Меняет текущий поворот модели.
    * @param angle Поворот в радианах.
    */
    void update(const float angle) {
        rotationAngle += angle;
        if (rotationAngle > 2 * M_PI) {
            rotationAngle -= 2 * M_PI;
        }
    }

    /**
    * @brief Обработка ввода.
    * Реализовано:
    * Выход на ESC и крестик
    * Вращение мышкой
    * Автовращение на +(=), -, пробел
    * Приближение на стрелки
    * 
    */
    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (
                event.type == SDL_EVENT_QUIT || 
                (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
            ) {
                running = false;
                return;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_EQUALS) speed += 0.01f;
                else if (event.key.key == SDLK_MINUS) speed -= 0.01f;
                else if (event.key.key == SDLK_SPACE) speed = 0;
                else if (event.key.key == SDLK_DOWN) distance += 1.0f;
                else if (event.key.key == SDLK_UP) distance -= 1.0f;
                else if (event.key.key == SDLK_C) opaque = !opaque;
                else if (event.key.key == SDLK_LEFT) moveLight(0.5f);
                else if (event.key.key == SDLK_RIGHT) moveLight(-0.5f);
            }
            else if (event.type == SDL_EVENT_MOUSE_MOTION && event.motion.state > 0) {
                speed = 0;
                update(-event.motion.xrel / 100);
            }
        }
    }
    
    /**
    * @brief Поворачивает освещение
    * @param angle Поворот в радианах.
    */
    void moveLight(const float angle) {
        auto rotationMatrix = Matrix4x4::rotationY(angle);
        light = rotationMatrix.transform(light);
    }

    /**
     * @brief Проекция точки на плоскость.
     * @note Для передачи в SDL_RenderLine
     * 
     * @param point3D Трёхмерная точка.
     * @return Двумерные координаты.
     */
    SDL_FPoint projectPoint(const Vec3D& point3D) {
        float fov{ 1.0f };
        
        Vec3D viewSpace{ point3D - cameraPos };
        float z{ viewSpace.z + distance};
        
        if (z <= 0.1f) return { -1000, -1000 };
        
        float x{ viewSpace.x * fov / z * 200.0f };
        float y{ -viewSpace.y * fov / z * 200.0f };
        
        x += windowWidth / 2.0f;
        y += windowHeight / 2.0f;
        
        return { x, y };
    }

    /**
    * @brief Отрисовка фигуры в SDL_Renderer.
    * @details Не меняет Obj3D, все преобразования через матрицу.
    */
    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        if (model.faces.empty()) return;
        
        auto rotationMatrix = Matrix4x4::rotationY(rotationAngle);
        
        for (const auto& f : model.faces) {
            if (f.v1 <= 0 || f.v2 <= 0 || f.v3 <= 0) continue;
                        
            auto p1{ projectPoint(rotationMatrix.transform(model.vertices[f.v1 - 1])) };
            auto p2{ projectPoint(rotationMatrix.transform(model.vertices[f.v2 - 1])) };
            auto p3{ projectPoint(rotationMatrix.transform(model.vertices[f.v3 - 1])) };
            
            if (
                p1.x == -1000 && p1.y == -1000 ||
                p2.x == -1000 && p2.y == -1000 ||
                p3.x == -1000 && p3.y == -1000
            ) continue;

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderLine(renderer, p1.x, p1.y, p2.x, p2.y);
            SDL_RenderLine(renderer, p2.x, p2.y, p3.x, p3.y);
            SDL_RenderLine(renderer, p3.x, p3.y, p1.x, p1.y);

            if (opaque) continue;
            if ((
                rotationMatrix.transform(model.normals[f.vn - 1]) -
                cameraPos*0.2f).length() > 1.5f
            ) continue;

            SDL_Vertex polygon[3];

            polygon[0].position = p1;
            polygon[1].position = p2;
            polygon[2].position = p3;

            auto shade{ (rotationMatrix.transform(model.normals[f.vn - 1]) - light).length() };

            polygon[0].color = { shade, shade, shade, 255 };
            polygon[1].color = { shade, shade, shade, 255 };
            polygon[2].color = { shade, shade, shade, 255 };

            SDL_RenderGeometry(renderer, nullptr, polygon, 3, nullptr, 0);
        }
        
        SDL_RenderPresent(renderer);
    }

public:
    SDLApp() : window(nullptr), windowWidth(800), windowHeight(600),
               renderer(nullptr), running(true),
               rotationAngle(0), speed(0.01f), opaque(false),
               cameraPos(0, 0, -5), distance(5.0f),
               light(-1, 0, -1) { light.normalize(); }
    
    ~SDLApp() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }

    /**
    * @brief Инициализация окна.
    */
    bool init() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
            return false;
        }
        
        window = SDL_CreateWindow("3D Viewer", windowWidth, windowHeight, 0);
        if (!window) {
            std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
            return false;
        }
        
        renderer = SDL_CreateRenderer(window, NULL);
        if (!renderer) {
            std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
            return false;
        }

        SDL_RaiseWindow(window);

        return true;
    }

    /**
    * @brief Загружает 3D модель.
    * 
    * @param file_path Путь.
    */
    bool loadModel(const auto& file_path) {
        if (!model.loadFromFile(file_path)) {
            std::cerr << "Failed to load " << file_path << std::endl;
            return false;
        }

        return true;
    }
    
    /**
    * @brief Основной цикл.
    * @note Окно должно быть инициализированно заранее. Выполняется пока running = true. Привязан к частоте в 60FPS.
    */
    void run() {
        while (running) {
            handleEvents();
            update(speed);
            render();
            SDL_Delay(16);
        }
    }
};

int main(int argc, char* argv[]) {
    SDLApp app;

    if (!app.init()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return 1;
    }

    std::string file_path{ "assets/cube.obj" };
    if (argc > 1) file_path = argv[1];
    if (!app.loadModel(file_path)) {
        return 1;
    }

    app.run();
    return 0;
}