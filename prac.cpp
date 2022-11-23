#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector> 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning(disable:4996)
using namespace std;

void make_vertexShaders(GLuint& shader);
void make_fragmentShaders(GLuint& shader);
GLuint make_shaderProgram(GLuint& shader1, GLuint& shader2);

GLuint shaderID;
GLuint vertexShader;
GLuint fragmentShader;

float terrainPosition[] = {
   -50.0f, 0.0f, -50.0f,
   -50.0f, 0.0f, 50.0f,
   50.0f, 0.0f, 50.0f,
   50.0f, 0.0f, -50.0f,
};

const GLchar* vertexShaderSource =
"#version 330 core \n"
"layout (location = 0) in vec3 vPos;\n"
"uniform mat4 projectionTransform;"
"uniform mat4 viewTransform;"
"uniform mat4 transform;"
"layout (location = 1) in vec2 vTexCoord;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"gl_Position = projectionTransform * viewTransform * transform * vec4(vPos, 1.0f);\n"
"TexCoord = vTexCoord;\n"
"}\0";

const GLchar* fragmentShaderSource =
"#version 330 core\n"
"out vec4 fragmentColor; \n"
"in vec2 TexCoord;\n"
"uniform sampler2D passTexture1;\n"
"uniform sampler2D passTexture2;\n"
"uniform bool twoTextures = false;\n"
"void main()\n"
"{\n"
"if(twoTextures)"
"fragmentColor = texture(passTexture1, TexCoord) * texture(passTexture2, TexCoord);\n"
"else if(!twoTextures)"
"fragmentColor = texture(passTexture1, TexCoord);\n"
"}\0";

void make_vertexShaders(GLuint& vertexShader)
{
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR : vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}
void make_fragmentShaders(GLuint& fragmentShader)
{
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR : fragment shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}
GLuint make_shaderProgram(GLuint& vertexShader, GLuint& fragmentShader)
{
    GLuint ShaderProgramID;

    ShaderProgramID = glCreateProgram();
    glAttachShader(ShaderProgramID, vertexShader);
    glAttachShader(ShaderProgramID, fragmentShader);
    glLinkProgram(ShaderProgramID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];

    glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
        exit(-1);
    }

    glUseProgram(ShaderProgramID);

    return ShaderProgramID;
}

int loadObj(const char* filename);
void InitTexture(const char* filename);

struct Objectload {

    vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    vector< glm::vec3 > temp_vertices;
    vector< glm::vec2 > temp_uvs;
    vector< glm::vec3 > temp_normals;
    vector< glm::vec3 > outvertex, outnormal;
    vector< glm::vec2 > outuv;

    float sumX = 0.0, sumY = 0.0, sumZ = 0.0;
    float aveX, aveY, aveZ;
    float scaleX, scaleY, scaleZ;
    float minX = 0.0, minY = 0.0, minZ = 0.0;
    float maxX = 0.0, maxY = 0.0, maxZ = 0.0;
    float scaleAll;

    float sizeX, sizeY, sizeZ;
    unsigned int texture;

    int loadObj(const char* filename);
    void InitTexture(const char* filename);
};

int Objectload::loadObj(const char* filename)
{
    FILE* objFile;

    fopen_s(&objFile, filename, "rb");

    if (objFile == NULL) {
        printf("Impossible to open the file !\n");
        return false;
    }
    while (1) {

        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(objFile, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(objFile, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);

            if (vertex.x < minX) minX = vertex.x;
            if (vertex.y < minY) minY = vertex.y;
            if (vertex.z < minZ) minZ = vertex.z;
            if (vertex.x > maxX) maxX = vertex.x;
            if (vertex.y > maxY) maxY = vertex.y;
            if (vertex.z > maxZ) maxZ = vertex.z;
            sumX += vertex.x;
            sumY += vertex.y;
            sumZ += vertex.z;

            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(objFile, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(objFile, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(objFile, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9) {
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }

    std::cout << "minX: " << minX << " minY: " << minY << " minZ: " << minZ << std::endl;
    std::cout << "maxX: " << maxX << " maxY: " << maxY << " maxZ: " << maxZ << std::endl;

    aveX = sumX / vertexIndices.size();
    aveY = sumY / vertexIndices.size();
    aveZ = sumZ / vertexIndices.size();
    scaleX = (1.0 - maxX) * 10 + 1;
    scaleY = (1.0 - maxY) * 10 + 1;
    scaleZ = (1.0 - maxZ) * 10 + 1;

    if (scaleX > scaleY) {
        if (scaleY > scaleZ)
            scaleAll = scaleZ;
        else
            scaleAll = scaleY;
    }
    else if (scaleX < scaleY) {
        if (scaleX < scaleZ)
            scaleAll = scaleX;
        else
            scaleAll = scaleZ;
    }
    std::cout << "aveX: " << aveX << " aveY: " << aveY << " aveZ: " << aveZ << std::endl;

    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        unsigned int vertexIndex = vertexIndices[i];
        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        outvertex.push_back(vertex);
    }
    for (unsigned int i = 0; i < uvIndices.size(); i++) {
        unsigned int uvIndex = uvIndices[i];
        glm::vec2 vertex = temp_uvs[uvIndex - 1];
        outuv.push_back(vertex);
    }
    for (unsigned int i = 0; i < normalIndices.size(); i++) {
        unsigned int normalIndex = normalIndices[i];
        glm::vec3 vertex = temp_normals[normalIndex - 1];
        outnormal.push_back(vertex);
    }
    return outvertex.size();
}
void Objectload::InitTexture(const char* filename)
{
    glGenTextures(1, &texture);
    stbi_set_flip_vertically_on_load(true);
    glBindTexture(GL_TEXTURE_2D, texture);
    // 텍스처 wrapping/filtering 옵션 설정(현재 바인딩된 텍스처 객체에 대해)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 텍스처 로드 및 생성
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

Objectload CastleWall_load, CastleGround_load, CastleDoors_load, CastleInterior_load;
Objectload Catapult_Trigger_load, Catapult_wood_load, Catapult_metal_load, Catapult_Body_load;

GLint Wall = CastleWall_load.loadObj("exterior.obj");
GLint Ground = CastleGround_load.loadObj("ground.obj");
GLint Interior = CastleInterior_load.loadObj("interior.obj");
GLint Doors = CastleDoors_load.loadObj("doors.obj");

GLint Trigger = Catapult_Trigger_load.loadObj("trigger.obj");
GLint Body = Catapult_Body_load.loadObj("body.obj");

GLuint VAO_wall, VBO_pos_wall, VBO_uv_wall, VBO_normal_wall;
GLuint VAO_ground, VBO_pos_ground, VBO_uv_ground, VBO_normal_ground;
GLuint VAO_doors, VBO_pos_doors, VBO_uv_doors, VBO_normal_doors;
GLuint VAO_interior, VBO_pos_interior, VBO_uv_interior, VBO_normal_interior;
GLuint VAO_trigger, VBO_pos_trigger, VBO_uv_trigger, VBO_normal_trigger;
GLuint VAO_body, VBO_pos_body, VBO_uv_body, VBO_normal_body;

void InitBuffer()
{
    glGenVertexArrays(1, &VAO_wall);
    glGenBuffers(1, &VBO_pos_wall);
    glGenBuffers(1, &VBO_uv_wall);
    glGenBuffers(1, &VBO_normal_wall);
    glBindVertexArray(VAO_wall);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos_wall);
    glBufferData(GL_ARRAY_BUFFER, CastleWall_load.outvertex.size() * sizeof(glm::vec3), &CastleWall_load.outvertex[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_uv_wall);
    glBufferData(GL_ARRAY_BUFFER, CastleWall_load.outuv.size() * sizeof(glm::vec2), &CastleWall_load.outuv[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &VAO_ground);
    glGenBuffers(1, &VBO_pos_ground);
    glGenBuffers(1, &VBO_uv_ground);
    glGenBuffers(1, &VBO_normal_ground);
    glBindVertexArray(VAO_ground);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos_ground);
    glBufferData(GL_ARRAY_BUFFER, CastleGround_load.outvertex.size() * sizeof(glm::vec3), &CastleGround_load.outvertex[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_uv_ground);
    glBufferData(GL_ARRAY_BUFFER, CastleGround_load.outuv.size() * sizeof(glm::vec2), &CastleGround_load.outuv[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &VAO_interior);
    glGenBuffers(1, &VBO_pos_interior);
    glGenBuffers(1, &VBO_uv_interior);
    glGenBuffers(1, &VBO_normal_interior);
    glBindVertexArray(VAO_interior);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos_interior);
    glBufferData(GL_ARRAY_BUFFER, CastleInterior_load.outvertex.size() * sizeof(glm::vec3), &CastleInterior_load.outvertex[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_uv_interior);
    glBufferData(GL_ARRAY_BUFFER, CastleInterior_load.outuv.size() * sizeof(glm::vec2), &CastleInterior_load.outuv[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &VAO_doors);
    glGenBuffers(1, &VBO_pos_doors);
    glGenBuffers(1, &VBO_uv_doors);
    glGenBuffers(1, &VBO_normal_doors);
    glBindVertexArray(VAO_doors);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos_doors);
    glBufferData(GL_ARRAY_BUFFER, CastleDoors_load.outvertex.size() * sizeof(glm::vec3), &CastleDoors_load.outvertex[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_uv_doors);
    glBufferData(GL_ARRAY_BUFFER, CastleDoors_load.outuv.size() * sizeof(glm::vec2), &CastleDoors_load.outuv[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &VAO_trigger);
    glGenBuffers(1, &VBO_pos_trigger);
    glGenBuffers(1, &VBO_uv_trigger);
    glGenBuffers(1, &VBO_normal_trigger);
    glBindVertexArray(VAO_trigger);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos_trigger);
    glBufferData(GL_ARRAY_BUFFER, Catapult_Trigger_load.outvertex.size() * sizeof(glm::vec3), &Catapult_Trigger_load.outvertex[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_uv_trigger);
    glBufferData(GL_ARRAY_BUFFER, Catapult_Trigger_load.outuv.size() * sizeof(glm::vec2), &Catapult_Trigger_load.outuv[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &VAO_body);
    glGenBuffers(1, &VBO_pos_body);
    glGenBuffers(1, &VBO_uv_body);
    glGenBuffers(1, &VBO_normal_body);
    glBindVertexArray(VAO_body);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos_body);
    glBufferData(GL_ARRAY_BUFFER, Catapult_Body_load.outvertex.size() * sizeof(glm::vec3), &Catapult_Body_load.outvertex[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_uv_body);
    glBufferData(GL_ARRAY_BUFFER, Catapult_Body_load.outuv.size() * sizeof(glm::vec2), &Catapult_Body_load.outuv[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
}


float cameraX = 0;
float cameraY = 0;
float cameraZ = 0;


void drawScene()
{
    //glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CW);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(shaderID);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int transLoc = glGetUniformLocation(shaderID, "transform");
    unsigned int viewLoc = glGetUniformLocation(shaderID, "viewTransform"); //--- 뷰잉 변환 설정
    unsigned int projectionLoc = glGetUniformLocation(shaderID, "projectionTransform");
    unsigned int ChosenColor = glGetUniformLocation(shaderID, "ChosenColor");
    unsigned int twoTextures = glGetUniformLocation(shaderID, "twoTextures");
    glm::vec3 cameraPos = glm::vec3(0.0f + cameraX, 10.0f + cameraY, 50.0f + cameraZ); //--- 카메라 위치
    glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 full_camera_rotate = glm::mat4(1.0f); //카메라 공전
    glm::mat4 box_camera_rotate = glm::mat4(1.0f); //배경 회전

    view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)800, 0.1f, 500.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    glUniform1i(glGetUniformLocation(shaderID, "passTexture1"), 0); // 직접 설정
    glUniform1i(glGetUniformLocation(shaderID, "passTexture2"), 1);

    glUniform1f(twoTextures, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, CastleWall_load.texture);
    glBindVertexArray(VAO_wall);
    glm::mat4 Wall_TF = glm::mat4(1.0f);
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(Wall_TF));
    glDrawArrays(GL_TRIANGLES, 0, Wall);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, CastleGround_load.texture);
    glBindVertexArray(VAO_ground);
    glm::mat4 Ground_TF = glm::mat4(1.0f);
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(Ground_TF));
    glDrawArrays(GL_TRIANGLES, 0, Ground);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, CastleInterior_load.texture);
    glBindVertexArray(VAO_interior);
    glm::mat4 Interior_TF = glm::mat4(1.0f);
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(Interior_TF));
    glDrawArrays(GL_TRIANGLES, 0, Interior);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, CastleDoors_load.texture);
    glBindVertexArray(VAO_doors);
    glm::mat4 Doors_TF = glm::mat4(1.0f);
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(Doors_TF));
    glDrawArrays(GL_TRIANGLES, 0, Doors);

    glUniform1f(twoTextures, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Catapult_wood_load.texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, Catapult_metal_load.texture);
    glBindVertexArray(VAO_trigger);
    glm::mat4 Trigger_TF = glm::mat4(1.0f);
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(Trigger_TF));
    glDrawArrays(GL_TRIANGLES, 0, Trigger);
    glBindVertexArray(VAO_body);
    glm::mat4 Body_TF = glm::mat4(1.0f);
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(Body_TF));
    glDrawArrays(GL_TRIANGLES, 0, Body);

    glutSwapBuffers();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
    std::cout << key << std::endl;
    std::cout << "입력" << std::endl;

    switch (key)
    {
    case 'z':
        cameraZ += 1;
        break;
    case 'x':
        cameraX += 1;
        break;
    case 'y':
        cameraY += 1;
        break;
    case 'Z':
        cameraZ -= 1;
        break;
    case 'X':
        cameraX -= 1;
        break;
    case 'Y':
        cameraY -= 1;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Example21");
    srand(time(NULL));
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Unable to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "GLEW Initialized\n";
    make_vertexShaders(vertexShader);
    make_fragmentShaders(fragmentShader);
    shaderID = make_shaderProgram(vertexShader, fragmentShader);
    InitBuffer();
    CastleWall_load.InitTexture("exterior.jpg");
    CastleGround_load.InitTexture("ground.jpg");
    CastleInterior_load.InitTexture("interior.jpg");
    CastleDoors_load.InitTexture("doors.jpg");
    Catapult_wood_load.InitTexture("wood.jpg");
    Catapult_metal_load.InitTexture("metal.jpg");
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(Keyboard);
    glutMainLoop();
}