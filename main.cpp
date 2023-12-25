#include "main.h"

//-----------------------[카메라 초기화]-----------------------//

static float angle = 0.0, ratio;
static float x = maze.startX, y = 0.5f * maze.mapScale, z = maze.startZ; // 1인칭 미로로 카메라 시작 위치 설정
static float lx = 0.0f, ly = 0.0f, lz = -2.0f; // 1인칭 미로로 방향 설정

float playerSpeed = 0.1;
float playerRotSpeed = 4.0;

void rotatePlayer(float ang) {
    lx = sin(playerRotSpeed * ang); // 회전속도 설정
    lz = -cos(playerRotSpeed * ang);
    glLoadIdentity();
    gluLookAt(x, y, z,
              x + lx, y + ly, z + lz,
              0.0f, 1.0f, 0.0f);
}

//-----------------------[미로 정보 읽어오기]-----------------------//

Maze readMazeFromFile(const string& filePath) {
    ifstream inFile(filePath);

    if (!inFile.is_open()) {
        cerr << "Error: Unable to open the file." << endl;
        exit(1);
    }

    // 미로 크기, 시작 포인트 X좌표, Y좌표를 파일에서 읽어옴
    inFile >> maze.size;
    inFile.ignore();

    inFile >> maze.startX >> maze.startZ;

    inFile.ignore();

    maze.data.resize(maze.size, vector<int>(maze.size, 0));

    for (int i = 0; i < maze.size; ++i) {
        string line;
        getline(inFile, line);

        if (line.size() != maze.size) {
            cerr << "Error: Invalid maze data at line " << i + 1 << ". Expected length: " << maze.size << ", Actual length: " << line.size() << endl;
            exit(1);
        }

        for (int j = 0; j < maze.size; ++j) {
            if (j < line.size()) {
                maze.data[i][j] = (line[j] - '0');
            }
            else {
                cerr << "Error: Subscript out of range at line " << i + 1 << ", column " << j + 1 << endl;
                exit(1);
            }
        }
    }

    inFile.close();

    // 미로의 카메라 시작좌표 읽어오기
    x = maze.startX, y = 0.5f * maze.mapScale, z = -maze.startZ;

    return maze;
}


void printTextMaze(const Maze& maze) {
    cout << "Maze Size: " << maze.size << endl;
    cout << "Start Point - X Coordinate: " << maze.startX << ", Z Coordinate: " << maze.startZ << endl;

    cout << "Maze Content:" << endl;
    for (int i = 0; i < maze.size; ++i) {
        for (int j = 0; j < maze.size; ++j) {
            cout << maze.data[i][j] << " ";
        }
        cout << endl;
    }
}

//-----------------------[충돌처리 및 아이템 상호작용]-----------------------//

bool checkCollison(AABB a, AABB b) {
    if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
    if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
    return true;
}

void updatePlayerAABB() {
    // 플레이어의 AABB 정보 갱신
    playerAABB.min.x = x - 0.1;
    playerAABB.min.z = z - 0.1;
    playerAABB.max.x = x + 0.1;
    playerAABB.max.z = z + 0.1;
}

// 아이템 변수 초기화
bool collisonMode = true;
bool holdItem = false;
int selectedItem = 0;  // 아이템 종류 (1: obj1, 2: obj2, 3: obj3)

static const double itemHoldDuration = 7.0; // 7초로 설정

void applyItemEffect() {
    switch (selectedItem - 1) { // index상 아이템은 2번부터 9까지 설정
    case 1:
        collisonMode = false;
        cout << "1번 아이템 적용됨!" << endl;
        // obj1 아이템 효과 적용
        break;
    case 2:
        playerSpeed *= 2;
        playerRotSpeed *= 1.6;
        cout << "2번 아이템 적용됨!" << endl;
        // obj2 아이템 효과 적용
        break;
    case 3:
        cout << "3번 아이템 적용됨!" << endl;
        playerSpeed *= 0.5;
        break;
        // 추가 아이템이 있다면 여기에 계속 추가 가능
    }
}

// F 눌러야 호출됨
void eatItem() {
    // 플레이어와 아이템 충돌 체크 (AABB 충돌체크)
    for (int i = 0; i < maze.size; ++i) {
        for (int j = 0; j < maze.size; ++j) {
            if (maze.data[i][j] >= 2) {
                // 아이템의 AABB 생성
                itemAABB.min.x = j - 0.5;
                itemAABB.min.z = -i - 0.5;
                itemAABB.max.x = j + 0.5;
                itemAABB.max.z = -i + 0.5;

                // 충돌 체크
                if (checkCollison(playerAABB, itemAABB)) {
                    if (!holdItem) {
                        holdItem = true;
                        itemHoldStartTime = time(nullptr);
                        
                        selectedItem = maze.data[i][j];
                        applyItemEffect();

                        cout << "아이템을 먹었습니다!" << endl;
                        maze.data[i][j] = 0;

                        return;
                    }
                }
            }
        }
    }
}

void resetItemEffect() {
    if (holdItem && difftime(time(nullptr), itemHoldStartTime) >= itemHoldDuration) {
        holdItem = false;
        collisonMode = true;
        playerSpeed = 0.1;
        playerRotSpeed = 4.0;
        cout << "7초 지남!!!!" << endl;
    }
}

//-----------------------[플레이어 이동 및 키보드 설정]-----------------------//

void movePlayer(int direction) {
    x = x + direction * lx * playerSpeed;
    z = z + direction * lz * playerSpeed;

    updatePlayerAABB();
    // 벽 각각 큐브 사각형에 대한 최소 최대 설정 후 충돌하는지 체크 
    for (int i = 0; i < maze.size; ++i)
    {
        for (int j = 0; j < maze.size; ++j)
        {
            // 1인 부분에 대한 AABB 생성
            if (maze.data[i][j] == 1)
            {
                AABB wallAABB;
                wallAABB.min.x = j - 0.5;
                wallAABB.min.z = -i - 0.5;
                wallAABB.max.x = j + 0.5;
                wallAABB.max.z = -i + 0.5;

                // 충돌 체크
                if (collisonMode == true && checkCollison(playerAABB, wallAABB))
                {
                    cout << "충돌!!!" << endl;
                    // 충돌 시 이동 취소
                    x = x - direction * lx * playerSpeed;
                    z = z - direction * lz * playerSpeed;
                    updatePlayerAABB();  // 갱신된 위치로 AABB 다시 설정
                    return;
                }
            }
        }
    }
    glLoadIdentity();
    gluLookAt(x, y, z, x + lx, y + ly, z + lz, 0.0f, 1.0f, 0.0f);
}

// 키보드 wasd 입력 & f 아이템
void inputKey(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
    case 'A':
        angle -= 0.01f;
        rotatePlayer(angle);
        cout << "좌" << endl;
        break;
    case 'd':
    case 'D':
        angle += 0.01f;
        rotatePlayer(angle);
        cout << "우" << endl;
        break;
    case 'w':
    case 'W':
        movePlayer(1);
        cout << "앞" << endl;
        break;
    case 's':
    case 'S':
        movePlayer(-1);
        cout << "뒤" << endl;
        break;
    case 'f':
    case 'F':
        eatItem();
        break;
    }
}

//-----------------------[SOR데이터 읽어오기]-----------------------//

SOR::SOR(const std::string& filePath) {
    LoadModel(filePath);
}

void SOR::LoadModel(const std::string& filePath) {
    std::ifstream fin(filePath, std::ios::in | std::ios::binary);

    if (fin.is_open()) {
        mpoint.clear();
        indexBuffer.clear();

        size_t mpointSize;
        fin.read(reinterpret_cast<char*>(&mpointSize), sizeof(size_t));
        mpoint.resize(mpointSize);
        fin.read(reinterpret_cast<char*>(mpoint.data()), mpointSize * sizeof(xPoint3D));

        size_t indexBufferSize;
        fin.read(reinterpret_cast<char*>(&indexBufferSize), sizeof(size_t));
        indexBuffer.resize(indexBufferSize);
        fin.read(reinterpret_cast<char*>(indexBuffer.data()), indexBufferSize * sizeof(unsigned int));

        fin.close();
        std::cout << "다음에 정상적으로 불러왔습니다: " << filePath << std::endl;
    }
    else {
        std::cerr << "파일 열기 오류 발생" << std::endl;
    }
}

void SOR::drawWireframe(float scale, const std::vector<float>& color) {
    glColor3f(color[0], color[1], color[2]);
    glBegin(GL_LINES);
    for (size_t i = 0; i < indexBuffer.size(); i++) {
        size_t index = indexBuffer[i];
        glVertex3f(scale * mpoint[index].x, scale * mpoint[index].y, scale * mpoint[index].z);
    }
    glEnd();
}

void SOR::draw(float scale, const std::vector<float>& color) {
    // 혹시나 추가할게 있다면 여기에 추가
    drawWireframe(scale, color);
}


//-----------------------[텍스처 파일 읽어오기 및 텍스처 선언부]-----------------------//

void TextureLoader::LoadTexture(const char* filePath, GLuint& textureID) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, components;
    unsigned char* image = Read_PngImage(filePath, &width, &height, &components);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    stbi_image_free(image);
}

unsigned char* TextureLoader::Read_PngImage(const char name[], int* width, int* height, int* components) {
    int iwidth, iheight, idepth;
    unsigned char* image = stbi_load(name, &iwidth, &iheight, &idepth, STBI_rgb_alpha);

    if (!image) {
        std::cerr << "Error: Unable to open PNG file: " << name << std::endl;
        exit(EXIT_FAILURE);
    }

    *width = iwidth;
    *height = iheight;
    *components = idepth;

    return image;
}

TexturedPlane::TexturedPlane(GLuint textureID) : textureID(textureID), blendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) {}

void TexturedPlane::setBlendMode(GLenum srcBlend, GLenum destBlend) {
    blendMode = std::make_pair(srcBlend, destBlend);
}

void TexturedPlane::draw() {
    glPushMatrix();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glEnable(GL_BLEND);
    glBlendFunc(blendMode.first, blendMode.second);

    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, 1.0);

    for (int i = 0; i < 4; ++i) {
        glTexCoord2fv(planeTexCoords[i]);
        glVertex3fv(planeVertices[i]);
    }

    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

GLuint textureID;

// 벽 큐브 아랫면이랑 동일한 설정
GLfloat planeVertices[4][3] = { {-0.5, 0.0, -0.5}, {-0.5, 0.0, 0.5}, {0.5, 0.0, 0.5}, {0.5, 0.0, -0.5} };
GLfloat planeTexCoords[4][2] = { {0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0} };

std::pair<GLenum, GLenum> blendMode; // 블렌드 모드 페어로 선택할 수 있도록 

// 텍스처 선언
TextureLoader textureLoad;

void initTexture() {
    textureLoad.LoadTexture("resource/wall.png", mazeTexture);
    textureLoad.LoadTexture("resource/ground.png", groundTexture);
    textureLoad.LoadTexture("resource/grassLOD1.png", grassLOD1Texture);
    textureLoad.LoadTexture("resource/grassLOD0.png", grassLOD0Texture);

    textureLoad.LoadTexture("resource/overlay1.png", overlay1Texture);
    textureLoad.LoadTexture("resource/overlay2.png", overlay2Texture);
    textureLoad.LoadTexture("resource/overlay3.png", overlay3Texture);

    textureLoad.LoadTexture("resource/skybox1.png", skybox1Texture);
    textureLoad.LoadTexture("resource/skybox2.png", skybox2Texture);
    textureLoad.LoadTexture("resource/skybox3.png", skybox3Texture);
    textureLoad.LoadTexture("resource/skybox4.png", skybox4Texture);
    textureLoad.LoadTexture("resource/skybox5.png", skybox5Texture);
    textureLoad.LoadTexture("resource/skybox6.png", skybox6Texture);
}

//-----------------------[각종 draw함수 모음]-----------------------//

void drawWall() {
    glPushMatrix();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mazeTexture);

    glEnable(GL_BLEND);

    //glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glColorPointer(3, GL_FLOAT, 0, MyColors);
    glVertexPointer(3, GL_FLOAT, 0, MyVertices);

    // UV맵 좌표
    GLfloat MyUV[24][2] = {
    {0.0, 0.5}, {0.0, 1.0}, {0.25, 1.0}, {0.25, 0.5}, // +x
    {0.75, 0.5}, {0.75, 1.0}, {1.0, 1.0}, {1.0, 0.5}, // -x
    {0.25, 0.5}, {0.25, 0.0}, {0.5, 0.0}, {0.5, 0.5}, // +y
    {0.25, 1.0}, {0.25, 0.5}, {0.5, 0.5}, {0.5, 1.0}, // -y
    {0.5, 0.5}, {0.5, 0.0}, {0.75, 0.0}, {0.75, 0.5}, // +z
    {0.25, 0.0}, {0.25, 0.5}, {0.5, 0.5}, {0.5, 0.0}  // -z
    };


    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, MyUV);

    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, MyVertexList);


    //glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void drawSkybox() {
    glPushMatrix();

    glScalef(100.0f, 100.0f, 100.0f);
    glTranslatef(0.0f, -0.001f, 0.0f);

    glColor3f(1.0f, 1.0f, 1.0f); // 색 초기화 안해주면 스카이박스 그릴때도 거리에 따라 블렌드 적용됨
    // Face 3 아래
    TexturedPlane skybox3(skybox3Texture);
    skybox3.setBlendMode(GL_ONE, GL_ZERO);
    skybox3.draw();

    // Face 1 위
    TexturedPlane skybox1(skybox1Texture);
    glTranslatef(0.0f, 1.0f, 0.0f);
    skybox1.setBlendMode(GL_ONE, GL_ZERO);
    skybox1.draw();

    // Face 2 앞면
    TexturedPlane skybox2(skybox2Texture);
    glTranslatef(0.0f, -0.5f, -0.5f);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    skybox2.setBlendMode(GL_ONE, GL_ZERO);
    skybox2.draw();

    // Face 6 뒷면
    TexturedPlane skybox6(skybox6Texture);
    glTranslatef(0.0f, +1.0f, 0.0f);
    skybox6.setBlendMode(GL_ONE, GL_ZERO);
    skybox6.draw();

    // Face 4 왼쪽
    TexturedPlane skybox4(skybox4Texture);
    glTranslatef(-0.5f, -0.5f, 0.0f);
    glRotatef(90.0, 0.0, 0.0, 1.0);
    skybox4.setBlendMode(GL_ONE, GL_ZERO);
    skybox4.draw();

    // Face 5 오른쪽
    TexturedPlane skybox5(skybox5Texture);
    glRotatef(180.0, 0.0, 0.0, 1.0);
    glTranslatef(0.0f, 1.0f, 0.0f);
    skybox5.setBlendMode(GL_ONE, GL_ZERO);
    skybox5.draw();

    glPopMatrix();
}

void drawGround() {
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 0.0f);
    TexturedPlane ground(groundTexture);
    ground.setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ground.draw();
    glPopMatrix();
}

void drawOnlyGrassLOD1() {
    glPushMatrix();

    glRotatef(90.0, 1.0, 0.0, 0.0);
    glScalef(0.3, 0.3, 0.3);

    TexturedPlane grassLOD1(grassLOD1Texture);

    grassLOD1.setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    grassLOD1.draw();
    glRotatef(90.0, 0.0, 0.0, 1.0);
    grassLOD1.draw();

    glPopMatrix();
}

void drawOnlyGrassLOD0() {
    glPushMatrix();

    TexturedPlane grassLOD0(grassLOD0Texture);

    grassLOD0.setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    grassLOD0.draw();

    glPopMatrix();
}

void drawOverlayview() {
    glPushMatrix();

    glEnable(GL_TEXTURE_2D);

    switch (selectedItem - 1) {
    case 1:
        glBindTexture(GL_TEXTURE_2D, overlay1Texture);
        break;
    case 2:
        glBindTexture(GL_TEXTURE_2D, overlay2Texture);
        break;
    case 3:
        glBindTexture(GL_TEXTURE_2D, overlay3Texture);
        break;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, 1.0);

    glTexCoord2d(0, 0); glVertex2f(-2, -1);
    glTexCoord2d(1, 0); glVertex2f(+2, -1);
    glTexCoord2d(1, 1); glVertex2f(+2, +1);
    glTexCoord2d(0, 1); glVertex2f(-2, +1);

    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void GenerateGrass() {
    glPushMatrix();

    for (int i = 0; i < maze.size; ++i) {
        for (int j = 0; j < maze.size; ++j) {
            glPushMatrix();
            glTranslatef(j * 1.0f, 0.1, -i * 1.0f); // 1칸씩 이동

            float wallDistance = sqrt((x - j) * (x - j) + (z + i) * (z + i));

            if (wallDistance >= 0.0 && wallDistance <= 3.0) {
                glCallList(grassLOD1DisplayList); // 잔디
            }
            else if (wallDistance > 3.0 && wallDistance <= 8.0) {
                glCallList(grassLOD0DisplayList);
            }
            glPopMatrix();
        }
    }
    glPopMatrix();
}

//-----------------------[디스플레이 리스트 선언부]-----------------------//

void createDisplayLists() {
    wallDisplayList = glGenLists(1);
    glNewList(wallDisplayList, GL_COMPILE);
    drawWall();
    glEndList();

    groundDisplayList = glGenLists(1);
    glNewList(groundDisplayList, GL_COMPILE);
    drawGround();
    glEndList();

    grassLOD1DisplayList = glGenLists(1);
    glNewList(grassLOD1DisplayList, GL_COMPILE);
    glTranslatef(-0.26, 0.0, -0.3);
    drawOnlyGrassLOD1();
    glTranslatef(+0.44, 0.0, +0.65);
    drawOnlyGrassLOD1();
    glTranslatef(+0.8, 0.0, +0.3);
    drawOnlyGrassLOD1();
    glEndList();

    grassLOD0DisplayList = glGenLists(1);
    glNewList(grassLOD0DisplayList, GL_COMPILE);
    glTranslatef(0.0, -0.095, 0.0);
    drawOnlyGrassLOD0();
    glEndList();


    obj1DisplayList = glGenLists(1);
    glNewList(obj1DisplayList, GL_COMPILE);
    SOR obj1("SORobj/obj1.dat");
    obj1.draw(0.5, { 1.0, 0.0, 1.0 });
    glEndList();

    obj2DisplayList = glGenLists(1);
    glNewList(obj2DisplayList, GL_COMPILE);
    SOR obj2("SORobj/obj2.dat");
    obj2.draw(0.4, { 0.0, 1.0, 1.0 });
    glEndList();

    obj3DisplayList = glGenLists(1);
    glNewList(obj3DisplayList, GL_COMPILE);
    SOR obj3("SORobj/obj3.dat");
    obj3.draw(0.4, { 1.0, 1.0, 0.0 });
    glEndList();

    objLOD0DisplayList = glGenLists(1);
    glNewList(objLOD0DisplayList, GL_COMPILE);
    glutWireCube(0.3);
    glEndList();
}

//-----------------------[미로 그리기]-----------------------//

void drawMaze(const Maze& maze) {
    for (int i = 0; i < maze.size; ++i) {
        for (int j = 0; j < maze.size; ++j) {
            glPushMatrix();
            glTranslatef(j * 1.0f, 0.5f, -i * 1.0f); // 1칸씩 이동

            // 벽까지의 거리 계산
            float wallDistance = sqrt((x - j) * (x - j) + (z + i) * (z + i));

            // 거리를 이용해 명도 계산
            float darkness = 1.0f / (wallDistance * 0.5f + 1.0f); // 월디스턴스 옆의 숫자 조정해서 간격 조정가능

            // 거리가 멀어질수록 어둡게 보임
            glColor3f(1.0f, 1.0f, 1.0f);
            glColor3f(darkness, darkness, darkness);
            glCallList(groundDisplayList);

            if (maze.data[i][j] == 1) {
                // 거리가 멀어질수록 어둡게 보임
                glColor3f(1.0f, 1.0f, 1.0f);
                glColor3f(darkness, darkness, darkness);
                glCallList(wallDisplayList);
            }

            // obj그리기 - f키로 줍기 토글할 수 있게 분리해둠
            if (maze.data[i][j] == 2) {
                if (wallDistance >= 0.0 && wallDistance <= 3.0) {
                    glCallList(obj1DisplayList); // obj1
                }
                else if (wallDistance > 3.0 && wallDistance <= 8.0) {
                    glColor3f(1.0f, 0.0f, 1.0f);
                    glCallList(objLOD0DisplayList);
                }
            }
            else if (maze.data[i][j] == 3) {
                if (wallDistance >= 0.0 && wallDistance <= 3.0) {
                    glCallList(obj2DisplayList); // obj2
                }
                else if (wallDistance > 3.0 && wallDistance <= 8.0) {
                    glColor3f(0.0f, 1.0f, 1.0f);
                    glCallList(objLOD0DisplayList);
                }
            }
            else if (maze.data[i][j] == 4) {
                if (wallDistance >= 0.0 && wallDistance <= 3.0) {
                    glCallList(obj3DisplayList); // obj3
                }
                else if (wallDistance > 3.0 && wallDistance <= 8.0) {
                    glColor3f(1.0f, 1.0f, 0.0f);
                    glCallList(objLOD0DisplayList);
                }
            }
            glPopMatrix();
        }
    }
}

//-----------------------[렌더링 관련 모음]-----------------------//

// 오버레이 그리기
void drawOverlay() {
    glDisable(GL_LIGHTING); // 오버레이 조명 비활성화
    // 현재 변환상태 스택에 저장
    glPushAttrib(GL_TRANSFORM_BIT | GL_ENABLE_BIT);
    // 현재 모드 투영행렬뷰로 설정
    glMatrixMode(GL_PROJECTION);

    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1); //오소뷰로 변환
    // 현재 뷰 다시 스택에 저장
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    glLoadIdentity();

    if (holdItem) {
        glColor3f(1, 1, 1);
        drawOverlayview();
    }
    // 이전상태(투영변환)으로 복원
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    // 변환 및 활성화 상태 이전 상태로 복원
    glPopAttrib();

    // 모델뷰 이전 상태로 복원
    glPopMatrix();
}

// 창 유지
void changeSize(int w, int h) {
    // 창이 너무 작아졌을때 0 으로 나뉘는 것 방지
    if (h == 0)
        h = 1;
    ratio = 1.0f * w / h;

    // 좌표계 수정 전 초기화
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // 뷰포트 창 전체크기로 설정 
    glViewport(0, 0, w, h);

    // 절단 공간 설정
    gluPerspective(45, ratio, 0.1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(x, y, z,
        x + lx, y + ly, z + lz,
        0.0f, 1.0f, 0.0f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawSkybox();
    drawMaze(maze);
    GenerateGrass();

    resetItemEffect(); // 아이템 효과 초기화

    drawOverlay();

    glutSwapBuffers();
}

void init() {
    glEnable(GL_DEPTH_TEST);

    //텍스처 초기화
    initTexture();

    // 벽과 오브젝트 디스플레이 리스트 생성
    createDisplayLists();

    // 멀티샘플링 활성화
    glEnable(GLUT_MULTISAMPLE);

    // 안티에일리어싱 설정
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
}

//-----------------------[메인 출력부]-----------------------//

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("maze drawing");

    // 파일에서 미로 읽어오기
    const string filePath = "resource/maze.txt";
    maze = readMazeFromFile(filePath);
    printTextMaze(maze);


    // 초기 카메라 위치와 방향 설정
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(x, y, z,
              x + lx, y + ly, z + lz,
              0.0f, 1.0f, 0.0f);

    glutKeyboardFunc(inputKey);
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(changeSize);

    init();

    glutMainLoop();

    return 0;
}