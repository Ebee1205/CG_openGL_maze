#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> //여러 모델을 저장하기 위해 추가
#include <cstdlib>  //clear기능(system("cls"))

const double PI = std::acos(-1);

class xPoint3D {
public:
    float x, y, z, w;
    xPoint3D() : x(0), y(0), z(0), w(1) {}
};

std::vector<xPoint3D> mpoint;  // 저장할 점들의 벡터


std::vector<unsigned int> indexBuffer; // 인덱스 버퍼

size_t originalVertexs = 0;   // 전역으로 이동한 변수
size_t rotatedVertexs = 0;    // 전역으로 이동한 변수
bool showWireframe = true;     // 와이어프레임 표시 여부
bool showFilledTriangles = false;   // 삼각형 메쉬 표시 여부
int modelCount = 1;  // 저장된 모델의 수

void SaveModel() {
    std::string fileName;
    char userInput;

    std::cout << "모델의 파일 이름을 설정하시겠습니까? (y/n): ";
    std::cin >> userInput;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 개행 문자 처리

    if (userInput == 'y' || userInput == 'Y') {
        std::cout << "파일 이름을 입력하세요: ";
        std::getline(std::cin, fileName);
    }
    else {
        std::stringstream defaultName;
        defaultName << "myModel" << modelCount++;
        fileName = defaultName.str();
    }

    // dat파일로 저장
    fileName += ".dat";

    std::ofstream fout(fileName, std::ios::out | std::ios::binary);

    if (fout.is_open()) {
        // 벡터 크기 지정 후 저장
        size_t mpointSize = mpoint.size();
        fout.write(reinterpret_cast<const char*>(&mpointSize), sizeof(size_t));

        // 버텍스 버퍼 저장
        fout.write(reinterpret_cast<const char*>(mpoint.data()), mpointSize * sizeof(xPoint3D));

        // 인덱스 버퍼 벡터 크기 지정 후 저장
        size_t indexBufferSize = indexBuffer.size();
        fout.write(reinterpret_cast<const char*>(&indexBufferSize), sizeof(size_t));

        // 인덱스 버퍼 저장
        fout.write(reinterpret_cast<const char*>(indexBuffer.data()), indexBufferSize * sizeof(unsigned int));

        fout.close();
        std::cout << "다음에 정상적으로 저장되었습니다: " << fileName << std::endl;
    }
    else {
        std::cerr << "파일 열기 오류 발생" << std::endl;
    }
}

void createIndexBuffer() {
    indexBuffer.clear();
    size_t n = originalVertexs;
    size_t r = (rotatedVertexs > 0) ? (rotatedVertexs / n) : 0;

    for (size_t i = 0; i < r; ++i) {
        for (size_t j = 0; j < n; ++j) {
            // 아래로 연결 (제일 마지막 열의 경우 예외 처리)
            if (j != n - 1) {
                indexBuffer.push_back(i * n + j);
                indexBuffer.push_back(i * n + (j + 1) % n);
            }

            // 옆으로 연결
            indexBuffer.push_back(i * n + j);
            indexBuffer.push_back((i + 1) % r * n + j);

            // 대각선 연결 (한 방향으로만)
            if (j < n - 1) {
                indexBuffer.push_back(i * n + j);
                indexBuffer.push_back((i + 1) % r * n + (j + 1));
            }
        }
    }
}


void printIndexBuffer() {
    std::cout << "Index Buffer Contents:" << std::endl;
    for (size_t i = 0; i < indexBuffer.size(); i++) {
        std::cout << indexBuffer[i] << " ";
    }
    std::cout << std::endl;
}

void drawPoints() {
    glColor3f(1.0, 0.0, 0.0);
    glPointSize(5.0);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < mpoint.size(); i++) {
        glVertex3f(mpoint[i].x, mpoint[i].y, mpoint[i].z);
    }
    glEnd();
    glFlush();
}

void resetPoints() {
    mpoint.clear();
    originalVertexs = 0;
    rotatedVertexs = 0;
    createIndexBuffer();
    glutPostRedisplay();
}

void drawWireframe() {
    if (showWireframe) {
        glColor3f(0.0, 0.0, 0.0);
        glBegin(GL_LINES);
        for (size_t i = 0; i < indexBuffer.size(); i++) {
            size_t index = indexBuffer[i];
            glVertex3f(mpoint[index].x, mpoint[index].y, mpoint[index].z);
        }
        glEnd();
    }
    //printIndexBuffer();
}

void drawFilledTriangles() {
    if (showWireframe && showFilledTriangles) {
        glColor4f(0.5, 0.5, 0.5, 0.5);  // 반투명한 회색 설정
        glBegin(GL_TRIANGLES);

        for (size_t i = 0; i < indexBuffer.size(); i += 4) {
            size_t index1 = indexBuffer[i];
            size_t index2 = indexBuffer[i + 1];
            size_t index3 = indexBuffer[i + 2];

            glVertex3f(mpoint[index1].x, mpoint[index1].y, mpoint[index1].z);
            glVertex3f(mpoint[index2].x, mpoint[index2].y, mpoint[index2].z);
            glVertex3f(mpoint[index3].x, mpoint[index3].y, mpoint[index3].z);

            size_t index4 = indexBuffer[i + 3];

            glVertex3f(mpoint[index1].x, mpoint[index1].y, mpoint[index1].z);
            glVertex3f(mpoint[index3].x, mpoint[index3].y, mpoint[index3].z);
            glVertex3f(mpoint[index4].x, mpoint[index4].y, mpoint[index4].z);
        }

        glEnd();
    }
}


void drawAxes() {
    // 가로선
    glColor3f(0.0, 0.7, 1.0);
    glBegin(GL_LINES);
    glVertex2f(-1.0, 0.0);
    glVertex2f(1.0, 0.0);
    glEnd();

    // 세로선
    glBegin(GL_LINES);
    glVertex2f(0.0, -1.0);
    glVertex2f(0.0, 1.0);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glClearColor(0.7, 0.7, 0.7, 1.0);
    drawAxes();
    drawPoints();  // 저장된 점을 다시 그림
    drawWireframe();  // 수정된 부분: drawWireframe로 변경
    drawFilledTriangles();

    glutSwapBuffers();  // 추가된 부분: 이제 이중 버퍼를 사용하기 때문에 swap 필요
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void mouseCallback(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        xPoint3D newPoint;
        newPoint.x = static_cast<float>(x) / glutGet(GLUT_WINDOW_WIDTH) * 2 - 1;
        newPoint.y = -(static_cast<float>(y) / glutGet(GLUT_WINDOW_HEIGHT) * 2 - 1);
        mpoint.push_back(newPoint);

        glutPostRedisplay();  //화면 갱신!! 클릭때마다 점 반영
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        resetPoints();
        std::cout << "모든 정점이 초기화 되었습니다." << std::endl;
    }
}


void rotatePointsXAxis(float fRotAngle, std::vector<xPoint3D>& arPoints, std::vector<xPoint3D>& rotPoints) {
    float radian = fRotAngle * (PI / 180.0);
    for (int i = 0; i < arPoints.size(); i++) {
        xPoint3D newPt;
        newPt.x = arPoints[i].x;
        newPt.y = arPoints[i].y * cos(radian) - arPoints[i].z * sin(radian);
        newPt.z = arPoints[i].y * sin(radian) + arPoints[i].z * cos(radian);
        rotPoints.push_back(newPt);
    }
}

void rotatePointsYAxis(float fRotAngle, std::vector<xPoint3D>& arPoints, std::vector<xPoint3D>& rotPoints) {
    float radian = fRotAngle * (PI / 180.0);
    for (int i = 0; i < arPoints.size(); i++) {
        xPoint3D newPt;
        newPt.x = arPoints[i].x * cos(radian) + arPoints[i].z * sin(radian);
        newPt.y = arPoints[i].y;
        newPt.z = -arPoints[i].x * sin(radian) + arPoints[i].z * cos(radian);
        rotPoints.push_back(newPt);
    }
}

void myHelp() {
    std::cout << "------------------- Help -------------------" << std::endl;
    std::cout << "Mouse Left Button : Add Vertex" << std::endl;
    std::cout << "Mouse Right Button : Reset Vertex" << std::endl;
    std::cout << "X ==> X Axis Rotation" << std::endl;
    std::cout << "Y ==> Y Axis Rotation" << std::endl;
    std::cout << "L ==> Wireframe On/Off" << std::endl;
    std::cout << "F ==> Filled Triangle On/Off" << std::endl;
    std::cout << "S ==> Save Vertices & Index Buffer" << std::endl;
    std::cout << "C ==> Clear Terminal" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
}

void keyboardCallback(unsigned char key, int x, int y) {
    std::vector<xPoint3D> rotatedPoints;
    int rotateAngle;
    if (key == 's' || key == 'S') {
        SaveModel();
        //std::cout << "정점과 인덱스 버퍼가 저장되었습니다." << std::endl;
    }
    else if (key == 'x' || key == 'X') {
        do {
            std::cout << "회전 각도 입력(360의 약수): ";
            std::cin >> rotateAngle;

            if (std::cin.fail() || 360 % rotateAngle != 0) {
                std::cin.clear();  // 입력 상태 초기화
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 버퍼 비우기
                std::cout << "올바른 값을 입력하세요.(문자열이거나 약수가 아닙니다.)" << std::endl;
            }
            else {
                break;
            }
        } while (true);

        originalVertexs = mpoint.size();
        for (int angle = 0; angle < 360; angle += rotateAngle) {
            rotatePointsXAxis(angle, mpoint, rotatedPoints);
        }
        mpoint = rotatedPoints;
        rotatedVertexs = mpoint.size();

        std::cout << "마우스로 찍은 정점 개수: " << originalVertexs << std::endl;
        std::cout << "회전체 정점 개수: " << rotatedVertexs << std::endl;
        std::cout << "X축 회전이 완료되었습니다." << std::endl;
        createIndexBuffer();
        glutPostRedisplay();
    }
    else if (key == 'y' || key == 'Y') {
        do {
            std::cout << "회전 각도 입력(360의 약수): ";
            std::cin >> rotateAngle;

            if (std::cin.fail() || 360 % rotateAngle != 0) {
                std::cin.clear();  // 입력 상태 초기화
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 버퍼 비우기
                std::cout << "올바른 값을 입력하세요.(문자열이거나 약수가 아닙니다.)" << std::endl;
            }
            else {
                break;
            }
        } while (true);

        originalVertexs = mpoint.size();
        for (int angle = 0; angle < 360; angle += rotateAngle) {
            rotatePointsYAxis(angle, mpoint, rotatedPoints);
        }
        mpoint = rotatedPoints;
        rotatedVertexs = mpoint.size();

        std::cout << "마우스로 찍은 정점 개수: " << originalVertexs << std::endl;
        std::cout << "회전체 정점 개수: " << rotatedVertexs << std::endl;
        std::cout << "Y축 회전이 완료되었습니다." << std::endl;
        createIndexBuffer();
        glutPostRedisplay();
    }
    else if (key == 'l' || key == 'L') {
        // 'L' 키를 누를 때 와이어프레임 표시 여부를 토글
        showWireframe = !showWireframe;
        if (showWireframe)
            std::cout << "와이어프레임 On" << std::endl;
        else
            std::cout << "와이어프레임 Off" << std::endl;
        glutPostRedisplay();
    }
    else if (key == 'f' || key == 'F') {
        // 'f' 키를 누르면 삼각형을 반투명한 회색으로 채우기
        showFilledTriangles = !showFilledTriangles;
        if (showFilledTriangles)
            std::cout << "색 채우기 On" << std::endl;
        else
            std::cout << "색 채우기 Off" << std::endl;
        glutPostRedisplay();
    }
    else if (key == 'c' || key == 'C') {
        //system("clear");
        system("cls");
        myHelp();
    }
}

int main(int argc, char** argv) {
    myHelp();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  // 수정된 부분: GL_DOUBLE로 변경
    glutInitWindowSize(800, 600);  // 캔버스 크기를 800x600으로 설정
    glutCreateWindow("SOR Modeler");

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(-1, 1, -1, 1);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseCallback);
    glutKeyboardFunc(keyboardCallback);

    glClearColor(1.0, 1.0, 1.0, 1.0);

    glutMainLoop();

    return 0;
}