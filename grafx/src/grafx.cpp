#include <grafx.hpp>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

int Obj3D::checkCorrect() const {
    for (int i = 0; i < faces.size(); i++) {
        if (
            faces[i].v1 > vertices.size() || faces[i].v2 > vertices.size() ||
            faces[i].v1 > vertices.size() || faces[i].vn > normals.size()
        ) {
            std::cerr << "Face " << i <<" contains a nonexistent vector" << std::endl;
            return i;
        }
    }
    return -1;
}

void Obj3D::calculateNormals() {
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


bool Obj3D::loadFromFile(const std::string& filename) {
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
    if (checkCorrect() != -1) {
        return false;
    }
    calculateNormals();
    return !faces.empty();
}

Matrix4x4::Matrix4x4() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m[i][j] = (i == j) ? 1.0f : 0.0f;
}

Matrix4x4 Matrix4x4::rotationY(float angle) {
    Matrix4x4 mat;
    float c = std::cos(angle);
    float s = std::sin(angle);
    mat.m[0][0] = c;  mat.m[0][2] = s;
    mat.m[2][0] = -s; mat.m[2][2] = c;
    return mat;
}

Vec3D Matrix4x4::transform(const Vec3D& v) const {
    float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
    float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
    float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
    return Vec3D(x, y, z);
}
