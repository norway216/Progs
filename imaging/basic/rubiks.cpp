// rubiks3x3_full.cpp
#include <GL/glut.h>
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>

// ------------------- 魔方数据结构 -------------------
enum Color { WHITE, YELLOW, RED, ORANGE, BLUE, GREEN };
const float colorRGB[6][3] = {
    {1,1,1}, {1,1,0}, {1,0,0}, {1,0.5,0}, {0,0,1}, {0,1,0}
};

struct CubeFace {
    Color grid[3][3];
};

struct Cube {
    CubeFace faces[6]; // 0:U,1:D,2:F,3:B,4:L,5:R
};

Cube cube;

// ------------------- OpenGL 渲染 -------------------
float rotationAngle = 0.0f;
int currentStep = 0;
std::vector<std::string> moveSeq = {"R","U","R'","U'"}; // 示例动作序列

void drawCube() {
    float size = 0.9f;
    for(int f=0; f<6; f++){
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                Color c = cube.faces[f].grid[i][j];
                glColor3f(colorRGB[c][0], colorRGB[c][1], colorRGB[c][2]);
                glPushMatrix();
                // 简单布局六面立方块
                glTranslatef(j-1, i-1, f-2.5f);
                glutSolidCube(0.9);
                glPopMatrix();
            }
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    glRotatef(rotationAngle, 0,1,0);
    drawCube();
    glPopMatrix();
    glutSwapBuffers();
}

void timer(int v){
    rotationAngle += 1.0f;
    if(rotationAngle > 360) rotationAngle -= 360;
    glutPostRedisplay();
    glutTimerFunc(16,timer,0);
}

// ------------------- 自动还原演示 -------------------
void autoSolve() {
    for(size_t i=0;i<moveSeq.size();i++){
        std::cout << "Move: " << moveSeq[i] << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        currentStep++;
    }
}

// ------------------- 主程序 -------------------
int main(int argc, char** argv) {
    // 初始化魔方颜色（演示用，每面单色）
    for(int f=0;f<6;f++)
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++)
                cube.faces[f].grid[i][j] = static_cast<Color>(f);

    // 初始化 OpenGL
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("3x3 Rubik Cube Solver Demo");
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2,0.2,0.2,1.0);

    // --------------------- 调整相机视角 ---------------------
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0/600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(5.0, 5.0, 10.0,  // 相机位置
              0.0, 0.0, 0.0,   // 看向魔方中心
              0.0, 1.0, 0.0);  // 上方向

    glutDisplayFunc(display);
    glutTimerFunc(16,timer,0);

    // 后台线程执行自动还原动作
    std::thread solver(autoSolve);
    solver.detach();

    glutMainLoop();
    return 0;
}