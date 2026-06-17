#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <iostream>

/**
 * @brief Вектор.
 * @details Реализовано: сложение, вычитание, домножение на число (float)
 * @todo Векторное и скалярное произведение
 * 
 * @param x, y, z Координаты.
 * @param length Длина. Требует cmath
 * @param normalize Нормализует вектор. Процедура без rvalue (см. notes)
 */
struct Vec3D {
    float x, y, z;
    Vec3D(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    Vec3D operator+(const Vec3D& v) const { return Vec3D(x + v.x, y + v.y, z + v.z); }
    Vec3D operator-(const Vec3D& v) const { return Vec3D(x - v.x, y - v.y, z - v.z); }
    Vec3D operator^(const Vec3D& v) const { return Vec3D(x * v.x, y * v.y, z * v.z); }
    Vec3D operator*(const Vec3D& v) const { return Vec3D(x - v.x, y - v.y, z - v.z); }
    Vec3D operator*(float s) const { return Vec3D(x * s, y * s, z * s); }

    float length() const { return std::sqrt(x*x + y*y + z*z); }
    /**
    * @brief Нормализует вектор.
    * @note Ничего не возвращает!
    */
    void normalize() { float l = length(); if (l > 0 && l != 1) { x /= l; y /= l; z /= l; } }
};

/**
 * @brief Грань.
 * @details Хранит индексы векторов хранящихся в Obj3D. Нумерация с 1; 0 - отсутствует вектор.
 * По умолчанию все поля 0.
 * 
 * @param v1, v2, v3 Индексы вершин.
 * @param vn Индекс вектора нормали.
 */
struct Face {
    int v1, v2, v3;
    int vn;
    Face() : v1(0), v2(0), v3(0), vn(0) {}
};

/**
 * @brief 3Д модель.
 * @details Загружает из файла и проверяет корректность.
 * Здесь хранится фигура в исходном виде, отрисовка делается отрисовщиком, поэтому поля открыты.
 * Перемещение делается через матрицу в основном цикле отрисовки.
 * 
 * @param vertices, normals, faces Для отрисовщика.
 * @param loadFromFile Не вызывать дважды!!!.
 */
class Obj3D {
private:
    /**
    * @brief Проверяет, что грани ссылаются на существующие вершины и нормали.
    * @return false если понадобится для строгой проверки.
    */
    bool checkCorrect() {
        bool result { true };
        for (int i = 0; i < faces.size(); i++) {
            if (
                faces[i].v1 > vertices.size() || faces[i].v2 > vertices.size() ||
                faces[i].v1 > vertices.size() || faces[i].vn > normals.size()
            ) {
                std::cerr << "Face " << i <<" contains a nonexistent vector" << std::endl;
                result = false;
            }
        }
        return result;
    }

    /**
    * @brief Вычисляет отсутствующие нормали для граней.
    * @todo Сделать оператор векторного умножения (см. Vec3D).
    */
    void calculateNormals() {
        for (auto& f : faces) {
            if (f.vn != 0) continue;

            Vec3D edge1 = vertices[f.v2 - 1] - vertices[f.v1 - 1];
            Vec3D edge2 = vertices[f.v3 - 1] - vertices[f.v1 - 1];
            Vec3D normal(
                edge1.y * edge2.z - edge1.z * edge2.y,
                edge1.z * edge2.x - edge1.x * edge2.z,
                edge1.x * edge2.y - edge1.y * edge2.x
            );
            normal.normalize();
            normals.push_back(normal);
            f.vn = normals.size();
        }
    }

public:
    std::vector<Vec3D> vertices;
    std::vector<Vec3D> normals;
    std::vector<Face> faces;

    /**
    * @brief Загружает 3D модель.
    * @details Читает только префиксы v, vn, f.
    * Для граней реализованы все варианты синтаксиса
    * 
    * @param file_path Путь.
    * @returns false если файл не открылся.
    * @note Автоматом проверяет и вычисляет нормали.
    * НЕ ОСТАНАВЛИВАЕТСЯ ПРИ ОШИБКАХ В ФАЙЛЕ (только предупреждает).
    */
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                Vec3D v;
                iss >> v.x >> v.y >> v.z;
                vertices.push_back(v);
            }
            else if (prefix == "vn") {
                Vec3D v;
                iss >> v.x >> v.y >> v.z;
                normals.push_back(v);
            }
            else if (prefix == "f") {
                Face f;
                std::string vertexData;
                for (int i = 0; i < 3; i++) {
                    iss >> vertexData;
                    std::istringstream viss(vertexData);
                    if (i == 0) {
                        std::string indexData;
                        std::getline(viss, indexData, '/');
                        f.v1 = std::stoi(indexData);
                        std::getline(viss, indexData, '/');
                        if (std::getline(viss, indexData, '/')) viss >> f.vn;
                    }
                    else if (i == 1) viss >> f.v2;
                    else if (i == 2) viss >> f.v3;
                }
                faces.push_back(f);
            }
        }

        file.close();
        checkCorrect();
        calculateNormals();
        checkCorrect();
        return !faces.empty();
    }
};

/**
* @brief Матрица.
* @details Всё перемещение объекта в пространстве делать через неё.
* @param Matrix4x4 Единичная матрица.
* @param rotationY Матрица вращения (см. описание).
* @param transform Возвращает новый вектор с новыми координатами.
*/
struct Matrix4x4 {
    float m[4][4];
    
    Matrix4x4() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }
    
    /**
    * @brief Матрица вращения.
    * @details Альтернативный конструктор. Вращает только по оси Y
    * @param angle Угол в радианах.
    */
    static Matrix4x4 rotationY(float angle) {
        Matrix4x4 mat;
        float c = std::cos(angle);
        float s = std::sin(angle);
        mat.m[0][0] = c;  mat.m[0][2] = s;
        mat.m[2][0] = -s; mat.m[2][2] = c;
        return mat;
    }
    
    Vec3D transform(const Vec3D& v) const {
        float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
        float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
        float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
        return Vec3D(x, y, z);
    }
};

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
    float rotationAngle;
    float speed;
    
    Vec3D cameraPos;
    float distance;

    /**
    * @brief Меняет текущий поворот модели.
    * @param angle Поворот в радианах.
    */
    void update(float angle) {
        rotationAngle += angle;
        if (rotationAngle > 2 * M_PI) {
            rotationAngle -= 2 * M_PI;
        }
    }

    /**
    * @brief Обработка ввода.
    * Реализовано:
    * Выход на ESC и крестик
    * Контроль вращения на +{=}, -, пробел
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
            }
            else if (event.type == SDL_EVENT_MOUSE_MOTION) {

                
            }
        }
    }

    /**
     * @brief Проекция точки на плоскость.
     * @note Для передачи в SDL_RenderLine
     * 
     * @param point3D Трёхмерная точка.
     * @return Двумерные координаты.
     */
    SDL_FPoint projectPoint(const Vec3D& point3D) {
        float fov = 1.0f;
        float aspect = (float)windowWidth / windowHeight;
        
        Vec3D viewSpace = point3D - cameraPos;
        float z = viewSpace.z + distance;
        
        if (z <= 0.1f) return {-1000, -1000};
        
        float x = viewSpace.x * fov / z * 200.0f;
        float y = -viewSpace.y * fov / z * 200.0f;
        
        x = x + windowWidth / 2.0f;
        y = y + windowHeight / 2.0f;
        
        return {x, y};
    }

    /**
    * @brief Отрисовка фигуры в SDL_Renderer.
    * @details Не меняет Obj3D, все преобразования через матрицу.
    */
    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        if (model.faces.empty()) return;
        
        Matrix4x4 rotationMatrix = Matrix4x4::rotationY(rotationAngle);
        
        for (const auto& f : model.faces) {
            if (f.v1 <= 0 || f.v2 <= 0 || f.v3 <= 0) continue;
            if (f.v1 > (int)model.vertices.size() || 
                f.v2 > (int)model.vertices.size() || 
                f.v3 > (int)model.vertices.size()) continue;
            
            Vec3D v1 = model.vertices[f.v1 - 1];
            Vec3D v2 = model.vertices[f.v2 - 1];
            Vec3D v3 = model.vertices[f.v3 - 1];
            
            Vec3D rv1 = rotationMatrix.transform(v1);
            Vec3D rv2 = rotationMatrix.transform(v2);
            Vec3D rv3 = rotationMatrix.transform(v3);
            
            SDL_FPoint p1 = projectPoint(rv1);
            SDL_FPoint p2 = projectPoint(rv2);
            SDL_FPoint p3 = projectPoint(rv3);
            
            if (p1.x == -1000 && p1.y == -1000) continue;
            if (p2.x == -1000 && p2.y == -1000) continue;
            if (p3.x == -1000 && p3.y == -1000) continue;

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderLine(renderer, p1.x, p1.y, p2.x, p2.y);
            SDL_RenderLine(renderer, p2.x, p2.y, p3.x, p3.y);
            SDL_RenderLine(renderer, p3.x, p3.y, p1.x, p1.y);
        }
        
        SDL_RenderPresent(renderer);
    }

public:
    SDLApp() : window(nullptr), renderer(nullptr), windowWidth(800), windowHeight(600),
               running(true),
               rotationAngle(0), speed(0.01f), cameraPos(0, 0, -5), distance(5.0f) {}
    
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
    * @note Окно должно быть инициализированно заранее. Выполняется в пока running = true. Привязан к частоте в 60FPS.
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
    
    std::string file_path {"assets/cube.obj"};
    if (argc > 1) file_path = argv[1];
    if (!app.loadModel(file_path)) {
        return 1;
    }

    app.run();
    return 0;
}