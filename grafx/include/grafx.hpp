#include <vector>
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
    bool checkCorrect();
    /**
    * @brief Вычисляет отсутствующие нормали для граней.
    * @todo Сделать оператор векторного умножения (см. Vec3D).
    */
    void calculateNormals();
public:
    std::vector<Vec3D> vertices, normals;
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
    bool loadFromFile(const std::string& filename);
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
    
    Matrix4x4();
    
    /**
    * @brief Матрица вращения.
    * @details Альтернативный конструктор. Вращает только по оси Y
    * @param angle Угол в радианах.
    */
    static Matrix4x4 rotationY(float angle);
    /**
    * @brief Поворачивает вектор.
    * @param v Вектор.
    * @return Повёрнутый вектор.
    */
    Vec3D transform(const Vec3D& v) const;
};
