#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h> // 카메라 기능
#include <cmath>
#include <ctime> // 아이템 타이머 기능

#include <GL/glut.h>

#define STB_IMAGE_IMPLEMENTATION
#include <GL/stb_image.h>

using namespace std;


// 미로
struct Maze {
    int size;       // Maze size
    float startX;   // Starting X coordinate
    float startZ;   // Starting Z coordinate
    float mapScale = 1.0;
    std::vector<std::vector<int>> data;  // Maze data
};

static Maze maze;

Maze readMazeFromFile(const string& filePath);
void printTextMaze(const Maze& maze);


// 카메라
extern float angle, ratio;
extern float x, y, z;
extern float lx, ly, lz;

extern float playerSpeed;
extern float playerRotSpeed;

void rotatePlayer(float ang);


// 충돌처리
struct Vec2 {
    double x;
    double z;
};

struct AABB {
    Vec2 min;
    Vec2 max;
};

AABB playerAABB;
AABB itemAABB;
extern bool collisonMode;
extern bool holdItem;
extern int selectedItem;

static time_t itemHoldStartTime; // 시간저장
extern const double itemHoldDuration;

bool checkCollison(AABB a, AABB b);
void updatePlayerAABB();
void applyItemEffect();
void eatItem();
void resetItemEffect();


// 키보드 설정
void movePlayer(int direction);
void inputKey(unsigned char key, int x, int y);


// SOR
class SOR {
private:
    struct xPoint3D {
        float x, y, z, w;
        xPoint3D() : x(0), y(0), z(0), w(1) {}
    };

    std::vector<xPoint3D> mpoint;
    std::vector<unsigned int> indexBuffer;

public:
    SOR(const std::string& filePath);

    void LoadModel(const std::string& filePath);

    void drawWireframe(float scale, const std::vector<float>& color);

    void draw(float scale, const std::vector<float>& color);
};

// 텍스처 로드
GLfloat MyVertices[8][3] = { {-0.5, -0.5, 0.5}, {-0.5, 0.5, 0.5}, {0.5, 0.5, 0.5}, {0.5, -0.5, 0.5},
        {-0.5, -0.5, -0.5}, {-0.5, 0.5, -0.5}, {0.5, 0.5, -0.5}, {0.5, -0.5, -0.5} };
GLubyte MyVertexList[24] = { 0,3,2,1,2,3,7,6,0,4,7,3,1,2,6,5,4,5,6,7,0,1,5,4 };
GLfloat MyColors[8][3] = { {0.2,0.2,0.2},{1.0,0.0,0.0},{1.0,1.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0},{1.0,0.0,1.0},{1.0,1.0,1.0},{0.0,1.0,1.0} };

GLfloat SkyVertices[8][3] = {
    {-0.5f, 0.5f, -0.5f},   // left top front
    {0.5f, 0.5f, -0.5f},    // right top front
    {0.5f, -0.5f, -0.5f},   // right bottom front
    {-0.5f, -0.5f, -0.5f},  // left bottom front
    {-0.5f, 0.5f, 0.5f},    // left top back
    {0.5f, 0.5f, 0.5f},     // right top back
    {0.5f, -0.5f, 0.5f},    // right bottom back
    {-0.5f, -0.5f, 0.5f}    // left bottom back
};
GLubyte SkyVertexList[24] = { 0,3,2,1,2,3,7,6,0,4,7,3,1,2,6,5,4,5,6,7,0,1,5,4 };


class TextureLoader {
public:
    TextureLoader() {}
    ~TextureLoader() {}

    void LoadTexture(const char* filePath, GLuint& textureID);

private:
    unsigned char* Read_PngImage(const char name[], int* width, int* height, int* components);
};

class TexturedPlane {
public:
    TexturedPlane(GLuint textureID);

    void setBlendMode(GLenum srcBlend, GLenum destBlend);

    void draw();

private:
    GLuint textureID;
    GLfloat planeVertices[4][3] = { {-0.5, 0.0, -0.5}, {-0.5, 0.0, 0.5}, {0.5, 0.0, 0.5}, {0.5, 0.0, -0.5} };
    GLfloat planeTexCoords[4][2] = { {0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0} };

    std::pair<GLenum, GLenum> blendMode;
};

GLuint mazeTexture;
GLuint groundTexture;

GLuint grassLOD1Texture;
GLuint grassLOD0Texture;

GLuint overlay1Texture;
GLuint overlay2Texture;
GLuint overlay3Texture;

GLuint skybox1Texture;
GLuint skybox2Texture;
GLuint skybox3Texture;
GLuint skybox4Texture;
GLuint skybox5Texture;
GLuint skybox6Texture;

void initTexture();

// draw부분
void drawWall();
void drawSkybox();
void drawGround();
void drawOnlyGrassLOD0();
void drawOnlyGrassLOD1();
void drawOverlayview();
void GenerateGrass();

// 미로 그리기 부분
void drawMaze(const Maze& maze);

// 디스플레이 리스트
GLuint wallDisplayList;
GLuint groundDisplayList;

GLuint grassLOD1DisplayList;
GLuint grassLOD0DisplayList;

GLuint obj1DisplayList;
GLuint obj2DisplayList;
GLuint obj3DisplayList;
GLuint objLOD0DisplayList;

void createDisplayLists();

// 렌더링
void changeSize(int w, int h);
void drawOverlay();
void changeSize(int w, int h);
void display();
void init();