#include "sopengl.h"
#include <math.h>



#define showsize 1
#define XR -70
#define YR 0
#define ZR 0




//支持off/obj/stl
int if_face=0, if_line=0, if_point=0;
//判断三种几何信息是否展示
static GLfloat my_set_material[] = { 1.0, 1.0, 0.1, 0.8};
int if_control_move = 0 ;



sopengl::sopengl(QWidget *parent)
{
    translate = showsize;
    m_ntype = 0;
    mesh = NULL;

}

void sopengl::initializeGL()
{
    /*设置清屏颜色*/
    //glClearColor(0.1,0.1, 0.1, 1);
    glClearColor(0, 0, 0, 1);
    /*设置着实技术，可以取值GL_FLAT和GL_SMOOTH。默认取值是GL_SMOOTH。*/
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0);
    /*GL_ALPHA_TEST	4864	跟据函数glAlphaFunc的条件要求来决定图形透明的层度是否显示。*/
    glEnable(GL_DEPTH_TEST);
}

void sopengl::resizeGL(int w, int h)
{
    double  m_nright =10, m_ntop = 10;
    double m_nleft =-10, m_nbottom = -10;

    glViewport(0, 0, GLint(w), GLint(h));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLfloat zNear = 0.1;
    GLfloat zFar = 100.0;
    GLfloat aspect = (GLfloat)w / (GLfloat)h;
    GLfloat fH = tan(GLfloat(90.0 / 360.0 * 3.14159)) * zNear;
    GLfloat fW = fH * aspect;
    //glFrustum(-fW, fW, -fH, fH, zNear, zFar);

    if (w <= h)
    {
        glOrtho(m_nleft, m_nright, m_ntop * (GLfloat)h / (GLfloat)w, m_nbottom * (GLfloat)h / (GLfloat)w, -1000.0, 1000.0);
    }
    else
    {
        glOrtho(m_nleft * (GLfloat)w / (GLfloat)h, m_nright * (GLfloat)w / (GLfloat)h, m_ntop, m_nbottom, -1000.0, 1000.0);
    }

    //gluPerspective(45.0, w / h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void sopengl::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    //glTranslatef(0.0, 0.0, translate);

    glScalef(translate, translate, translate);
    glRotatef(xRot, 1.0, 0.0, 0.0);
    glRotatef(yRot, 0.0, 1.0, 0.0);
    glRotatef(zRot, 0.0, 0.0, 1.0);

    if(m_ntype==0)
        draw_faces();
    if(m_ntype==1)
        draw_lines();
    if(m_ntype==2)
        draw_points();

    paintaxis(100);
}

void sopengl::keyPressEvent(QKeyEvent *)
{

}

void sopengl::wheelEvent(QWheelEvent *e)
{
    if (e->delta() > 0)
    {
        translate += GLfloat(0.05);
    }
    else
    {
        translate -= GLfloat(0.05);
    }
    qDebug() << translate;
    updateGL();
}

void sopengl::mousePressEvent(QMouseEvent *event)
{
    x = event->globalPos().x() - this->pos().x() - yRot;
    y = event->globalPos().y() - this->pos().y() - xRot;
}

void sopengl::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        int x1, y1;
        x1 = event->globalPos().x() - this->pos().x();
        y1 = event->globalPos().y() - this->pos().y();

        //qDebug() << x1 - x << " " << y1 - y;
        yRot = x1 - x;
        xRot = y1 - y;

        qDebug() << xRot << " " << yRot;
        updateGL();
    }
}

void sopengl::mouseReleaseEvent(QMouseEvent *event)
{
    temxy[0] = int(yRot);
    temxy[1] = int(xRot);
}

void sopengl::mouseDoubleClickEvent(QMouseEvent *event)
{
    translate = showsize;
    xRot = XR;
    yRot = YR;
    zRot = ZR;
    x = y = 0;
    updateGL();
}

void sopengl::paintaxis(double length)
{
    /*绘制坐标系*/
    glPointSize(5.0f);
    glBegin(GL_LINE_LOOP);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0, 0, 0);
    glVertex3f(length, 0, 0);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, length, 0);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, length);
    glEnd();
}

myMesh *sopengl::ReadASCII(const char *cfilename)
{
    /*点位容器*/
    std::vector<float> coorX;
    std::vector<float> coorY;
    std::vector<float> coorZ;
    /*法向量容器*/
    std::vector<float> vcoorX;
    std::vector<float> vcoorY;
    std::vector<float> vcoorZ;

    int i = 0, j = 0, cnt = 0, pCnt = 4;
    char a[100];
    char str[100];
    double x = 0, y = 0, z = 0;


    std::ifstream in(cfilename, std::ifstream::in);

    if (!in)
        return 0;
    do
    {
        i = 0;
        cnt = 0;
        in.getline(a, 100, '\n');
        while (a[i] != '\0')
        {
            if (!islower((int)a[i]) && !isupper((int)a[i]) && a[i] != ' ')
                break;
            cnt++;
            i++;
        }
        /*判断是不是向量*/
        bool bv = false;
        stringstream sstr(a);
        string bs;
        sstr >>bs;
        if(bs == "facet")
        {
            bv = true;
        }

        while (a[cnt] != '\0')
        {
            str[j] = a[cnt];
            cnt++;
            j++;
        }
        str[j] = '\0';
        j = 0;

        if(bv)
        {
            if (sscanf(str, "%lf%lf%lf", &x, &y, &z) == 3)
            {
                vcoorX.push_back(x);
                vcoorY.push_back(y);
                vcoorZ.push_back(z);
            }
        }
        else
        {
            if (sscanf(str, "%lf%lf%lf", &x, &y, &z) == 3)
            {
                coorX.push_back(x);
                coorY.push_back(y);
                coorZ.push_back(z);
            }
        }

        pCnt++;
    } while (!in.eof());

    //向结构体进行转换
    int vert_number = coorX.size();
    int face_number = vert_number/3;

    myMesh *meshs = new myMesh();
    meshs->verts = new Vertex [vert_number+1];
    assert(meshs->verts);
    //处理点的信息
    for(int i = 0 ; i  < vert_number;i++)
    {
        Vertex& vert = meshs->verts[meshs->vert_number++];
        vert.x =coorX[i];
        vert.y =coorY[i];
        vert.z =coorZ[i];
    }
    //处理面的信息
    meshs->faces = new Face [face_number];
    assert(meshs->faces);
    int index=0;
    for(int i  = 0; i < face_number;i++)
    {
        Face fa ;
        fa.vert_number = 3;                  //这里直接设置为定3即可！ STL三角片决定的！
        fa.verts =new Vertex* [fa.vert_number];
        for (int j  = 0 ;  j  < 3; j++) {
            fa.verts[j] = &meshs->verts[index++];
        }

        //get_normal(fa);
        fa.normal[0] = vcoorX[i];
        fa.normal[1] = vcoorY[i];
        fa.normal[2] = vcoorZ[i];

        meshs->faces[meshs->face_number++] = fa;
        //首先分配第一维数组
    }

    return meshs;
}

myMesh *sopengl::ReadBinary(const char *cfilename)
{
    std::vector<GLfloat> coorX;
    std::vector<GLfloat> coorY;
    std::vector<GLfloat> coorZ;
    char str[80];
    std::ifstream in(cfilename, std::ifstream::in | std::ifstream::binary);
    if (!in)
        return 0;
    //首先使用二进制读入文件
    myMesh *meshs = new myMesh();        //建立我们的mesh类
    in.read(str, 80);
    int triangles;                  //triangles 记录了三角面的数目
    in.read((char*)&triangles, sizeof(int));
    if (triangles == 0)
        return 0;
    for (int i = 0; i < triangles; i++)
    {
        //开始处理三角片  所以面的数量计数器++
        float coorXYZ[12];
        in.read((char*)coorXYZ, 12 * sizeof(float));
        for (int j = 1; j < 4; j++)
        {
            //三个四字节信息
            //分别处理每个坐标点
            coorX.push_back(coorXYZ[j * 3]);
            coorY.push_back(coorXYZ[j * 3 + 1]);
            coorZ.push_back(coorXYZ[j * 3 + 2]);
            //将点的信息压入暂存数组
            //由于stl文件特性只需要保持三个一组进行最后的划分即可！
        }
        in.read((char*)coorXYZ, 2);
    }
    in.close();
    //向结构体进行转换
    int vert_number = coorX.size();
    int face_number = triangles;
    meshs->verts = new Vertex [vert_number+1];
    assert(meshs->verts);
    //处理点的信息
    for(int i = 0 ; i  < vert_number;i++)
    {
        Vertex& vert = meshs->verts[meshs->vert_number++];
        vert.x =coorX[i];
        vert.y =coorY[i];
        vert.z =coorZ[i];
    }
    //处理面的信息
    meshs->faces = new Face [face_number];
    assert(meshs->faces);
    int index=0;
    for(int i  = 0; i < face_number;i++)
    {
        Face fa ;
        fa.vert_number = 3;                  //这里直接设置为定3即可！ STL三角片决定的！
        fa.verts =new Vertex* [fa.vert_number];
        for (int j  = 0 ;  j  < 3; j++) {
            fa.verts[j] = &meshs->verts[index++];
        }
        get_normal(fa);
        meshs->faces[meshs->face_number++] = fa;
        //首先分配第一维数组
    }
    return meshs;
}

myMesh *sopengl::ReadSTLFile(const char *cfilename)
{
    //只处理三角形足够了！
    if (cfilename == NULL)
        return 0;
    std::ifstream in(cfilename, std::ifstream::in);
    if (!in.is_open())
        return 0;
    std::string headStr;
    getline(in, headStr, ' ');
    in.close();
    if (headStr[0] == 's')
        return ReadASCII(cfilename);
    else
        return  ReadBinary(cfilename);


}

vector<string> split(const string& str, const string& delim) {
    vector<string> res;
    if("" == str) return res;
    //先将要切割的字符串从string类型转换为char*类型
    char * strs = new char[str.length() + 1] ; //不要忘了
    strcpy(strs, str.c_str());

    char * d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char *p = strtok(strs, d);
    while(p) {
        string s = p; //分割得到的字符串转换为string类型
        res.push_back(s); //存入结果数组
        p = strtok(NULL, d);
    }

    return res;
}

myMesh *sopengl::ReaderOBj(string fname2)
{
    string line;
    fstream f;
    f.open(fname2, ios::in);
    if (!f.is_open()) {
        cout << "文件打开出错" << endl;
    }
    vector<vector<GLfloat>> vset;
    vector<vector<GLint>> fset;
    int v_counter = 1;
    int f_counter  = 1;
    while (!f.eof()) {
        getline(f, line);//拿到obj文件中一行，作为一个字符串
        vector<string>parameters;
        string tailMark = " ";
        string ans = "";
        line = line.append(tailMark);
        if(line[0]!='v'&&line[0]!='f'){
            continue;
        }
        for (int i = 0; i < line.length(); i++) {
            char ch = line[i];
            if (ch != ' ') {
                ans += ch;
            }
            else {
                if(ans!=""){
                    parameters.push_back(ans); //取出字符串中的元素，以空格切分
                    ans = "";}
            }
        }
        cout << endl;
        if (parameters[0] == "v") {   //如果是顶点的话
            vector<GLfloat>Point;
            v_counter++;
            //            cout <<atof( parameters[1].c_str()) << "--" << atof( parameters[2].c_str()) <<" --  " << atof( parameters[3].c_str());
            Point.push_back(atof( parameters[1].c_str()));
            Point.push_back(atof( parameters[2].c_str()));
            Point.push_back(atof( parameters[3].c_str()));
            vset.push_back(Point);
        }
        else if (parameters[0] == "f") {   //如果是面的话，存放顶点的索引
            vector<GLint>vIndexSets;          //临时存放点的集合
            for (int i = 1; i < 4; i++) {
                string x = parameters[i];
                string ans = "";
                for (int j = 0; j < x.length(); j++) {   //跳过‘/’
                    char ch = x[j];
                    if (ch != '/') {
                        ans += ch;
                    }
                    else {
                        break;
                    }
                }
                vector<string > res;
                res = split(ans,"/");
                int index = atof(res[0].c_str());
                index--;//因为顶点索引在obj文件中是从1开始的，而我们存放的顶点vector是从0开始的，因此要减1
                vIndexSets.push_back(index);
            }
            fset.push_back(vIndexSets);
        }

    }
    f.close();
    myMesh *meshs = new myMesh();        //建立我们的mesh类
    //    cout << fset.size() << endl;
    //    cout << vset.size() << endl;
    //向结构体进行转换
    int vert_number = vset.size();
    int face_number = fset.size();
    meshs->verts = new Vertex [vert_number+1];
    assert(meshs->verts);
    //处理mesh的信息
    //----------------处理点的信息 --------------
    for(int i = 0 ; i  < vset.size();i++)
    {
        Vertex& vert = meshs->verts[meshs->vert_number++];
        vert.x = vset[i][0];
        vert.y = vset[i][1];
        vert.z = vset[i][2];
    }
    //----------------处理面信息 --------------
    meshs->faces = new Face [face_number];
    assert(meshs->faces);
    for(int i  = 0; i < fset.size();i++)
    {
        Face a ;
        a.vert_number = fset[i].size();
        a.verts =new Vertex* [fset[i].size()];
        for (int j  = 0 ;  j  < fset[i].size(); j++) {
            a.verts[j] = &meshs->verts[fset[i][j]];
        }
        get_normal(a);
        meshs->faces[meshs->face_number++] = a;
        //首先分配第一维数组
    }
    return meshs;
}

myMesh *sopengl::ReadOffFile(const char *filename)
{
    /*
       函数说明： 对文件进行读取,读取的是off文件
       */
    int i;
    FILE *fp;                                         //开始读入文件
    if (!(fp = fopen(filename, "r"))) {
        cout << "无法打开文件" << endl;
        return 0;
    }
    myMesh *mesh = new myMesh();        //建立我们的mesh类
    //进行读入文件的操作
    int vert_number = 0;                            //记录顶点个数
    int face_number = 0;                                    //记录面的个数
    int line_number = 0;                                   //记录边的个数
    int line_count = 0;                               //这个是我读入了几行
    char buffer[1024];
    while (fgets(buffer, 1023, fp)) {
        line_count++;
        char *bufferp = buffer;
        while (isspace(*bufferp)) bufferp++;
        if (*bufferp == '#') continue;
        if (*bufferp == '\0') continue;
        if (vert_number == 0) {
            if (!strstr(bufferp, "OFF")) {
                if ((sscanf(bufferp, "%d%d%d", &vert_number, &face_number, &line_number) != 3) || (vert_number == 0)) {
                    cout << "存在语法错误！"  << endl;
                    fclose(fp);
                    return NULL;
                }
                //存储顶点的信息
                cout << "aa" << vert_number << "--" << face_number << endl;
                mesh->verts = new Vertex [vert_number];
                assert(mesh->verts);
                //存储面的信息
                mesh->faces = new Face [face_number];
                assert(mesh->faces);
                //如果头文件信息合适那么开辟内存空间
            }
        }
        else if (mesh->vert_number < vert_number) {
            Vertex& vert = mesh->verts[mesh->vert_number++];
            if (sscanf(bufferp, "%f%f%f", &(vert.x), &(vert.y), &(vert.z)) != 3) {
                cout << "点的信息中，数据量不足（3个）" << endl;
                fclose(fp);
                return NULL;
            }
        }
        else if (mesh->face_number < face_number) {
            Face& face = mesh->faces[mesh->face_number++];          //新建一个face对象
            bufferp = strtok(bufferp, " \t");
            if (bufferp) face.vert_number = atoi(bufferp);
            else {
                fclose(fp);
                return NULL;
            }
            face.verts = new Vertex *[face.vert_number];
            //建立面的数组
            assert(face.verts);
            for (i = 0; i < face.vert_number; i++) {
                bufferp = strtok(NULL, " \t");
                if (bufferp) face.verts[i] = &(mesh->verts[atoi(bufferp)]);
                else {
                    fprintf(stderr, "Syntax error with face on line %d in file %s\n", line_count, filename);
                    fclose(fp);
                    return NULL;
                }
            }
            get_normal(face);
        }
        else {
            cout << "格式存在错误！"  << endl;
            break;
        }
    }
    //判断实际的 面 的数目是否和要求数目一样！
    if (face_number != mesh->face_number) {
        cout << "面的数目与实际不符"<<endl;
    }
    fclose(fp);
    return mesh;
}

void sopengl::get_normal(Face &face)
{

    //计算面法线！
    //计算面的法线
    //通过公式计算：
    face.normal[0] = face.normal[1] = face.normal[2] = 0;
    Vertex *v1 = face.verts[face.vert_number-1];
    for (int i = 0; i < face.vert_number; i++) {
        Vertex *v2 = face.verts[i];
        //新建所有的点
        face.normal[0] += (v1->y - v2->y) * (v1->z + v2->z);
        face.normal[1] += (v1->z - v2->z) * (v1->x + v2->x);
        face.normal[2] += (v1->x - v2->x) * (v1->y + v2->y);
        //首先完成叉乘的工作
        v1 = v2;
    }
    //计算归一化法线
    float squared_normal_length = 0.0;
    squared_normal_length += face.normal[0]*face.normal[0];
    squared_normal_length += face.normal[1]*face.normal[1];
    squared_normal_length += face.normal[2]*face.normal[2];
    float normal_length = sqrt(squared_normal_length);
    //得到归一化长度
    if (normal_length > 1.0E-6) {
        face.normal[0] /= normal_length;
        face.normal[1] /= normal_length;
        face.normal[2] /= normal_length;
    }
    //然后完成归一化任务
}

void sopengl::draw_faces()
{
    if(mesh == NULL)
    {
        return;
    }
    for (int i = 0; i < mesh->face_number; i++) {
        //注意我们的操作都是
        Face& face = mesh->faces[i];
        glBegin(GL_POLYGON);              //绘制多边形即可！
        //在绘制面的过程中载入我们已经计算好的法线量信息
        glNormal3fv(face.normal);           //在绘制面的时候同时载入法向量信息
        for (int j = 0; j < face.vert_number; j++) {
            Vertex *vert = face.verts[j];
            glVertex3f(vert->x, vert->y, vert->z);
        }
        glEnd();
    }
}

void sopengl::draw_points()
{
    if(mesh == NULL)
    {
        return;
    }
    //下面绘制点的信息
    //依次将面的信息点进行遍历
    glColor3f(0.0, 1.0, 0.0);
    glPointSize(2);
    glBegin(GL_POINTS);
    for (int j  = 0 ;  j < mesh->vert_number; j++) {
        glVertex3f(mesh->verts[j].x, mesh->verts[j].y,mesh->verts[j].z);
    }
    glEnd();
    //    for (int i = 0; i < mesh->face_number; i++) {
    //        Face& face = mesh->faces[i];
    //        glColor3f(0.0, 1.0, 0.0);
    //        glPointSize(1);
    //        glBegin(GL_POINTS);
    //
    //                for (int j = 0; j < face.vert_number; j++) {
    //            Vertex *vert = face.verts[j];
    // cout << vert->x << " " << vert->y <<"  "<<vert ->z << endl;
    //            glVertex3f(vert->x, vert->y, vert->z);
    //        }
    //        glEnd();
    //    }
}

void sopengl::draw_lines()
{
    if(mesh == NULL)
    {
        return;
    }
    double temp_x,temp_y,temp_z;
    for (int i = 0; i < mesh->face_number; i++) {
        Face& face = mesh->faces[i];
        glColor3f(0, 0, 1);
        glBegin(GL_LINES);
        for (int j = 0; j < face.vert_number; j++) {
            Vertex *vert = face.verts[j];
            if(j==0){
                temp_x  = vert->x;
                temp_y  = vert->y;
                temp_z  = vert->z;
                continue;
            }
            glVertex3f(temp_x, temp_y, temp_z);
            glVertex3f(vert->x, vert->y, vert->z);
            temp_x  = vert->x;
            temp_y  = vert->y;
            temp_z  = vert->z;
        }
        glEnd();
    }
}
