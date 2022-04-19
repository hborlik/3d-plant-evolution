/* Jack Ortega: 
*  SuperShape 3D Music Visualizer
*  "Stereo Mix" audio component must be enabled and audio output device must be the same device as the one providing the "Stereo Mix"
*/

#include "glut.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "Bmp.h"
#include "Sphere.h"
#include "recordAudio.h"
#include <math.h>
#include <algorithm>    
#include "kiss_fft.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;


void displayCB();
void reshapeCB(int w, int h);
void timerCB(int millisec);
void keyboardCB(unsigned char key, int x, int y);
void mouseCB(int button, int stat, int x, int y);
void mouseMotionCB(int x, int y);

void initGL();
int  initGLUT(int argc, char** argv);
bool initSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void drawString(const char* str, int x, int y, float color[4], void* font);
void drawString3D(const char* str, float pos[3], float color[4], void* font);
void toPerspective();
GLuint loadTexture(const char* fileName, bool wrap = true);


const int   SCREEN_WIDTH = 1920;
const int   SCREEN_HEIGHT = 980;
const float CAMERA_DISTANCE = 20.0f;
const int   TEXT_WIDTH = 8;
const int   TEXT_HEIGHT = 13;


void* font = GLUT_BITMAP_8_BY_13;
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
int drawMode;
GLuint texId;
GLuint MeshPosID, MeshTexID;

int imageWidth;
int imageHeight;

// sphere: min sector = 3, min stack = 2
Sphere sphere2(1.0f, 72, 72);           // radius, sectors, stacks, smooth(default)

extern captureAudio actualAudioData;
extern int running;


#define FFTW_ESTIMATEE (1U << 6)
#define FFT_MAXSIZE 500


float amplitude_on_frequency[FFT_MAXSIZE] = {0};
float amplitude_on_frequency_10steps[10] = { 0 };

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool fft(float* amplitude_on_frequency, int& length)
{


    int N = pow(2, 10);
    BYTE data[MAXS];
    int size = 0;
    //cout << "size1 = " << size << endl;
    actualAudioData.readAudio(data, size);
    //cout << "size2 = " << size << endl;	
    length = size / 8;
    if (size == 0)
        return false;

    double* samples = new double[length];
    for (int ii = 0; ii < length; ii++)
    {
        float* f = (float*)&data[ii * 8];
        samples[ii] = (double)(*f);
    }


    kiss_fft_cpx* cx_in = new kiss_fft_cpx[length];
    kiss_fft_cpx* cx_out = new kiss_fft_cpx[length];
    kiss_fft_cfg cfg = kiss_fft_alloc(length, 0, 0, 0);
    for (int i = 0; i < length; ++i)
    {
        cx_in[i].r = samples[i];
        cx_in[i].i = 0;
    }

    kiss_fft(cfg, cx_in, cx_out);

    float amplitude_on_frequency_old[FFT_MAXSIZE];
    for (int i = 0; i < length / 2 && i < FFT_MAXSIZE; ++i)
        amplitude_on_frequency_old[i] = amplitude_on_frequency[i];

    for (int i = 0; i < length / 2 && i < FFT_MAXSIZE; ++i)
        amplitude_on_frequency[i] = sqrt(pow(cx_out[i].i, 2.) + pow(cx_out[i].r, 2.));


    //that looks better, decomment for no filtering: +++++++++++++++++++
    for (int i = 0; i < length / 2 && i < FFT_MAXSIZE; ++i)
    {
        float diff = amplitude_on_frequency_old[i] - amplitude_on_frequency[i];
        float attack_factor = 0.1;//for going down
        if (amplitude_on_frequency_old[i] < amplitude_on_frequency[i])
            attack_factor = 0.85; //for going up
        diff *= attack_factor;
        amplitude_on_frequency[i] = amplitude_on_frequency_old[i] - diff;
    }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


    length /= 2;
    free(cfg);

    return true;
}
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

void aquire_fft_scaling_arrays()
{
    //get FFT array
    static int length = 0;
    if (fft(amplitude_on_frequency, length))
    {
        //put the height of the frequencies 20Hz to 20000Hz into the height of the line-vertices
        vec3 vertices[FFT_MAXSIZE];
        float steps = 10. / (float)FFT_MAXSIZE;
        for (int i = 0; i < FFT_MAXSIZE; i++)
        {
            float step = i / (float)length;
            step *= 20;

            float height = 0;
            if (i < length)
                height = amplitude_on_frequency[i] * 0.05 * (1 + step);
            vertices[i] = vec3(-5. + (float)i * steps, height, 0.0);
        }

        //calculate the average amplitudes for the 10 spheres
        for (int i = 0; i < 10; i++)
            amplitude_on_frequency_10steps[i] = 0;

        int mean_range = length / 10;
        int bar = 0;
        int count = 0;

        for (int i = 0; ; i++, count++)
        {
            if (mean_range == count)
            {
                count = -1;
                amplitude_on_frequency_10steps[bar] /= (float)mean_range;
                bar++;
                if (bar == 10)break;
            }
            if (i < length && i < FFT_MAXSIZE)
                amplitude_on_frequency_10steps[bar] += amplitude_on_frequency[i];
            //cout << "scaling = " << amplitude_on_frequency_10steps[1] << endl;

        }
    }
}


int main(int argc, char** argv)
{
    initSharedMem();

    initGLUT(argc, argv);
    initGL();
    thread t1(start_recording);

    texId = loadTexture("earth2048.bmp", true);


    //Similar to render in other program for this class, just using GLUT based callbacks.
    glutMainLoop();
    running = FALSE;
    t1.join();

    return 0;
}


int initGLUT(int argc, char** argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode

    glutInitWindowSize(screenWidth, screenHeight);  // window size

    glutInitWindowPosition(100, 100);               // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]);     // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    glutTimerFunc(33, timerCB, 33);             // redraw only every given millisec
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);

    return handle;
}



void initGL()
{
    glShadeModel(GL_SMOOTH);                    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);


    glClearColor(0, 0, 0, 0);                   
    glClearStencil(0);                          
    glClearDepth(1.0f);                         
    glDepthFunc(GL_LEQUAL);

    initLights();
}



// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
void drawString(const char* str, int x, int y, float color[4], void* font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while (*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}




// draw a string in 3D space
void drawString3D(const char* str, float pos[3], float color[4], void* font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos3fv(pos);        // place text position

    // loop all characters in the string
    while (*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}




bool initSharedMem()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = mouseMiddleDown = false;
    mouseX = mouseY = 0;

    cameraAngleX = cameraAngleY = 0.0f;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0;

    sphere2.printSelf();

    return true;
}



void initLights()
{
    GLfloat lightKa[] = { .3f, .3f, .3f, 1.0f };  // ambient light
    GLfloat lightKd[] = { .7f, .7f, .7f, 1.0f };  // diffuse light
    GLfloat lightKs[] = { 1, 1, 1, 1 };           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    float lightPos[4] = { 0, 0, 200, 0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        
}



void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0);
}




GLuint loadTexture(const char* fileName, bool wrap)
{
    Image::Bmp bmp;
    if (!bmp.read(fileName))
        return 0;     

    int width = bmp.getWidth();
    int height = bmp.getHeight();
    const unsigned char* data = bmp.getDataRGB();
    GLenum type = GL_UNSIGNED_BYTE;

    GLenum format;
    int bpp = bmp.getBitCount();
    if (bpp == 8)
        format = GL_LUMINANCE;
    else if (bpp == 24)
        format = GL_RGB;
    else if (bpp == 32)
        format = GL_RGBA;
    else
        return 0;              

    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);


    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data);

    switch (bpp)
    {
    case 8:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 1, width, height, GL_LUMINANCE, type, data);
        break;
    case 24:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, type, data);
        break;
    case 32:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, type, data);
        break;
    }

    return texture;
}

void showInfo()
{
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1); // set to orthogonal projection

    float color[4] = { 1, 1, 1, 1 };

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3);

    ss << "Sector Count: " << sphere2.getSectorCount() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (2 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Stack Count: " << sphere2.getStackCount() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (3 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Vertex Count: " << sphere2.getVertexCount() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (4 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Index Count: " << sphere2.getIndexCount() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (5 * TEXT_HEIGHT), color, font);
    ss.str("");

    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix();                   // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
    glPopMatrix();                   // restore to previous modelview matrix
}

void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0f, (float)(screenWidth) / screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//similar to render()
void displayCB()
{

    //static float time = .0f;
    static float time = .85f;

    //time = time + 0.005f;
    //time += amplitude_on_frequency_10steps[0]/1000.f;

    if (time >= 3.14f)
    {
        time = 0.0f;
    }
    //cout << "time = " << time << endl;

    //Set the FFT arrays
    aquire_fft_scaling_arrays();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();

    glTranslatef(0, 0, -cameraDistance);

    // set material
    float ambient[] = { map(amplitude_on_frequency_10steps[0] + amplitude_on_frequency_10steps[0] + amplitude_on_frequency_10steps[1], 0.f, 15.f, 0.f, 1.f) * .75f, map(amplitude_on_frequency_10steps[4] + amplitude_on_frequency_10steps[5] + amplitude_on_frequency_10steps[6], 0.f, 5.f, 0.f, 1.3f) * 1.f, map(amplitude_on_frequency_10steps[1] * 1.5 + amplitude_on_frequency_10steps[2], 0.f, 8.f, 0.f, .8f) * .5f, 1};
    float diffuse[] = { 0.1f, 0.1f, 0.1f, 1 };
    float specular[] = { map(amplitude_on_frequency_10steps[1] * 1.5 + amplitude_on_frequency_10steps[2], 0.f, 8.f, 0.f, .8f) * .8f, map(amplitude_on_frequency_10steps[3] + amplitude_on_frequency_10steps[4] + amplitude_on_frequency_10steps[5] + amplitude_on_frequency_10steps[6], 0.f, 5.f, 0.f, .5f) * 1.f, map(amplitude_on_frequency_10steps[0], 0.f, 15.f, 0.f, 1.f) * .8f, 1 };
    float shininess = 120;
    for (int i = 0; i < 4; i++) {
        ambient[i] = ambient[i] * .5f;
        specular[i] = specular[i] * .1f;
    }
    ambient[1] = ambient[1] * 1.1f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    // line color
    float lineColor[] = { 0, 0, 0, 1 };

    glPushMatrix();
    glTranslatef(0.f, 0, -2);
    glRotatef(cameraAngleY, 0, 1, 0);
    glRotatef(cameraAngleX, 1, 0, 0);
    glRotatef(270, 0, 0, 1);
    //glRotatef(-90, 1, 0, 0);
    glBindTexture(GL_TEXTURE_2D, texId);
    sphere2.set(1.0f, 100, 100, time, amplitude_on_frequency_10steps);
    sphere2.draw();
    glPopMatrix();


    glBindTexture(GL_TEXTURE_2D, 0);

    showInfo();     // print max range of glDrawRangeElements

    glPopMatrix();

    glutSwapBuffers();
}


//resize window
void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    toPerspective();
    std::cout << "window resized: " << w << " x " << h << std::endl;

#ifdef _WIN32
    HWND handle = ::GetActiveWindow();
    RECT rect;
    ::GetWindowRect(handle, &rect); // with non-client area; border, titlebar etc.
    std::cout << "=========================" << std::endl;
    std::cout << "full window size with border: " << (rect.right - rect.left) << "x" << (rect.bottom - rect.top) << std::endl;
    ::GetClientRect(handle, &rect); // only client dimension
    std::cout << "client window size: " << (rect.right - rect.left) << "x" << (rect.bottom - rect.top) << std::endl;
    std::cout << "=========================" << std::endl;
#endif
}


void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);
    glutPostRedisplay();
}


void keyboardCB(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // ESCAPE
        exit(0);
        break;

    case 'd': // switch rendering modes (fill -> wire -> point)
    case 'D':
        ++drawMode;
        drawMode %= 3;
        if (drawMode == 0)        // fill mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        else if (drawMode == 1)  // wireframe mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else                    // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        break;

    default:
        ;
    }
}


void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if (state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if (state == GLUT_UP)
            mouseRightDown = false;
    }

    else if (button == GLUT_MIDDLE_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if (state == GLUT_UP)
            mouseMiddleDown = false;
    }
}


void mouseMotionCB(int x, int y)
{
    if (mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if (mouseRightDown)
    {
        cameraDistance -= (y - mouseY) * 0.2f;
        mouseY = y;
    }
}
