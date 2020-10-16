#ifndef SOPENGL_H
#define SOPENGL_H

#include <QWidget>
#include <QtOpenGL/qgl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <QGLWidget>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <vector>
#include <QDebug>

#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
using namespace std;



typedef struct Vertex {
    //定义三维图形的
    //用于face结构体中
    float x, y, z;
} Vertex;

typedef struct Face {
    //多边形（三角形）面的结构体
    Face(void) : vert_number(0), verts(0) {};
    int vert_number;        //记录顶点的个数
    Vertex **verts;          //这是一个面的所有 顶点数组（含有坐标）
    float normal[3];         //记录点的法向量，分别是x，y，z三个方向
    //注意点的法向量通过顶点的信息计算得到！
    //对于obj模型如果我们已经得到了法线的信息
    //那么就直接拿来用就好！
} Face;

typedef struct myMesh {
    //自定义mesh的结构体
    myMesh(void) : vert_number(0), verts(0), face_number(0), faces(0) {};
    //自定义构造器
    int vert_number;        //总的顶点个数
    Vertex *verts;             //定点数组
    int face_number;                 //面的数目
    Face *faces;
    vector<Vertex>point;
} myMesh;



class sopengl :public QGLWidget
{
public:
    sopengl(QWidget *parent = NULL);

    void setstl(QString str)
    {
        mesh = ReadSTLFile(str.toUtf8().data());
        updateGL();
    }
    void settypes(int num)
    {
        m_ntype = num;
        updateGL();
    }


protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void keyPressEvent(QKeyEvent*);
    void wheelEvent(QWheelEvent* e);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);


private:
    GLfloat translate, xRot, yRot, zRot;

    int x;
    int y;
    bool buttonbl;

    int temxy[2];

    void paintaxis(double length);

    /*STL文件解析*/
    myMesh *ReadASCII(const char *cfilename);
    myMesh *ReadBinary(const char *cfilename);
    myMesh *ReadSTLFile(const char *cfilename);

    /*obj文件解析*/
    myMesh * ReaderOBj(string fname2);
    /*off文件解析*/
    myMesh * ReadOffFile(const char *filename);
    void  get_normal(Face& face);
    void draw_faces();
    void draw_points();
    void draw_lines();
    myMesh *mesh;
    int m_ntype;

};

#endif // SOPENGL_H
