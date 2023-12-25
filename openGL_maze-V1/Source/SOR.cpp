#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> //���� ���� �����ϱ� ���� �߰�
#include <cstdlib>  //clear���(system("cls"))

const double PI = std::acos(-1);

class xPoint3D {
public:
    float x, y, z, w;
    xPoint3D() : x(0), y(0), z(0), w(1) {}
};

std::vector<xPoint3D> mpoint;  // ������ ������ ����


std::vector<unsigned int> indexBuffer; // �ε��� ����

size_t originalVertexs = 0;   // �������� �̵��� ����
size_t rotatedVertexs = 0;    // �������� �̵��� ����
bool showWireframe = true;     // ���̾������� ǥ�� ����
bool showFilledTriangles = false;   // �ﰢ�� �޽� ǥ�� ����
int modelCount = 1;  // ����� ���� ��

void SaveModel() {
    std::string fileName;
    char userInput;

    std::cout << "���� ���� �̸��� �����Ͻðڽ��ϱ�? (y/n): ";
    std::cin >> userInput;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // ���� ���� ó��

    if (userInput == 'y' || userInput == 'Y') {
        std::cout << "���� �̸��� �Է��ϼ���: ";
        std::getline(std::cin, fileName);
    }
    else {
        std::stringstream defaultName;
        defaultName << "myModel" << modelCount++;
        fileName = defaultName.str();
    }

    // dat���Ϸ� ����
    fileName += ".dat";

    std::ofstream fout(fileName, std::ios::out | std::ios::binary);

    if (fout.is_open()) {
        // ���� ũ�� ���� �� ����
        size_t mpointSize = mpoint.size();
        fout.write(reinterpret_cast<const char*>(&mpointSize), sizeof(size_t));

        // ���ؽ� ���� ����
        fout.write(reinterpret_cast<const char*>(mpoint.data()), mpointSize * sizeof(xPoint3D));

        // �ε��� ���� ���� ũ�� ���� �� ����
        size_t indexBufferSize = indexBuffer.size();
        fout.write(reinterpret_cast<const char*>(&indexBufferSize), sizeof(size_t));

        // �ε��� ���� ����
        fout.write(reinterpret_cast<const char*>(indexBuffer.data()), indexBufferSize * sizeof(unsigned int));

        fout.close();
        std::cout << "������ ���������� ����Ǿ����ϴ�: " << fileName << std::endl;
    }
    else {
        std::cerr << "���� ���� ���� �߻�" << std::endl;
    }
}

void createIndexBuffer() {
    indexBuffer.clear();
    size_t n = originalVertexs;
    size_t r = (rotatedVertexs > 0) ? (rotatedVertexs / n) : 0;

    for (size_t i = 0; i < r; ++i) {
        for (size_t j = 0; j < n; ++j) {
            // �Ʒ��� ���� (���� ������ ���� ��� ���� ó��)
            if (j != n - 1) {
                indexBuffer.push_back(i * n + j);
                indexBuffer.push_back(i * n + (j + 1) % n);
            }

            // ������ ����
            indexBuffer.push_back(i * n + j);
            indexBuffer.push_back((i + 1) % r * n + j);

            // �밢�� ���� (�� �������θ�)
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
        glColor4f(0.5, 0.5, 0.5, 0.5);  // �������� ȸ�� ����
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
    // ���μ�
    glColor3f(0.0, 0.7, 1.0);
    glBegin(GL_LINES);
    glVertex2f(-1.0, 0.0);
    glVertex2f(1.0, 0.0);
    glEnd();

    // ���μ�
    glBegin(GL_LINES);
    glVertex2f(0.0, -1.0);
    glVertex2f(0.0, 1.0);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glClearColor(0.7, 0.7, 0.7, 1.0);
    drawAxes();
    drawPoints();  // ����� ���� �ٽ� �׸�
    drawWireframe();  // ������ �κ�: drawWireframe�� ����
    drawFilledTriangles();

    glutSwapBuffers();  // �߰��� �κ�: ���� ���� ���۸� ����ϱ� ������ swap �ʿ�
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

        glutPostRedisplay();  //ȭ�� ����!! Ŭ�������� �� �ݿ�
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        resetPoints();
        std::cout << "��� ������ �ʱ�ȭ �Ǿ����ϴ�." << std::endl;
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
        //std::cout << "������ �ε��� ���۰� ����Ǿ����ϴ�." << std::endl;
    }
    else if (key == 'x' || key == 'X') {
        do {
            std::cout << "ȸ�� ���� �Է�(360�� ���): ";
            std::cin >> rotateAngle;

            if (std::cin.fail() || 360 % rotateAngle != 0) {
                std::cin.clear();  // �Է� ���� �ʱ�ȭ
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // ���� ����
                std::cout << "�ùٸ� ���� �Է��ϼ���.(���ڿ��̰ų� ����� �ƴմϴ�.)" << std::endl;
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

        std::cout << "���콺�� ���� ���� ����: " << originalVertexs << std::endl;
        std::cout << "ȸ��ü ���� ����: " << rotatedVertexs << std::endl;
        std::cout << "X�� ȸ���� �Ϸ�Ǿ����ϴ�." << std::endl;
        createIndexBuffer();
        glutPostRedisplay();
    }
    else if (key == 'y' || key == 'Y') {
        do {
            std::cout << "ȸ�� ���� �Է�(360�� ���): ";
            std::cin >> rotateAngle;

            if (std::cin.fail() || 360 % rotateAngle != 0) {
                std::cin.clear();  // �Է� ���� �ʱ�ȭ
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // ���� ����
                std::cout << "�ùٸ� ���� �Է��ϼ���.(���ڿ��̰ų� ����� �ƴմϴ�.)" << std::endl;
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

        std::cout << "���콺�� ���� ���� ����: " << originalVertexs << std::endl;
        std::cout << "ȸ��ü ���� ����: " << rotatedVertexs << std::endl;
        std::cout << "Y�� ȸ���� �Ϸ�Ǿ����ϴ�." << std::endl;
        createIndexBuffer();
        glutPostRedisplay();
    }
    else if (key == 'l' || key == 'L') {
        // 'L' Ű�� ���� �� ���̾������� ǥ�� ���θ� ���
        showWireframe = !showWireframe;
        if (showWireframe)
            std::cout << "���̾������� On" << std::endl;
        else
            std::cout << "���̾������� Off" << std::endl;
        glutPostRedisplay();
    }
    else if (key == 'f' || key == 'F') {
        // 'f' Ű�� ������ �ﰢ���� �������� ȸ������ ä���
        showFilledTriangles = !showFilledTriangles;
        if (showFilledTriangles)
            std::cout << "�� ä��� On" << std::endl;
        else
            std::cout << "�� ä��� Off" << std::endl;
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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  // ������ �κ�: GL_DOUBLE�� ����
    glutInitWindowSize(800, 600);  // ĵ���� ũ�⸦ 800x600���� ����
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