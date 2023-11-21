/*********************************/
/* CS 590CGS Lab framework        */
/* (C) Bedrich Benes 2020        */
/* bbenes ~ at ~ purdue.edu      */
/* Press +,- to add/remove points*/
/*       r to randomize          */
/*       s to change rotation    */
/*       c to render curve       */
/*       t to render tangents    */
/*       p to render points      */
/*       s to change rotation    */
/*********************************/

#include <stdio.h>
#include <iostream>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <string>
#include <vector>			//Standard template library class
#include <GL/freeglut.h>
#include <random>
#include "PerlinNoise2d.h"
#include "TerrainGenerator.h"
#include "DEMLoader.h"
#include "ColorInterpolator.h"
#include "math/vect3d.h"
#include <fstream>
#include<windows.h> 
#include <omp.h>

//in house created libraries
#include "math/vect3d.h"    //for vector manipulation
#include "trackball.h"

#include "Particle.h"
#include "Fluid.h"

#pragma warning(disable : 4996)
#pragma comment(lib, "freeglut.lib")

using namespace std;

//some trackball variables -> used in mouse feedback
TrackBallC trackball;
bool mouseLeft, mouseMid, mouseRight;


GLuint points = 0;  //number of points to display the object
int steps = 20;     //# of subdivisions
bool needRedisplay = false;
GLfloat  sign = +1; //diretcion of rotation
const GLfloat defaultIncrement = 0.7f; //speed of rotation
GLfloat  angleIncrement = defaultIncrement;

int particleMatrixSize[3] = { 10,10,10 };
Fluid fluid(particleMatrixSize);

PerlinNoise2d noise = PerlinNoise2d(0, 32, 1, 1, 10, 10);
float zpos = 1.0f;
float maxHeight = 0.8f;
float minHeight = 0.0f;
float pointSize = 0.03;

bool takeScreenshot = true;// false;

//bool isDEM = false;
bool isDEM = true;
vector<vector<TerrainPoint>> GetTerrain() {
	vector<vector<TerrainPoint>> t;
	if (isDEM) {
		DEMLoader terrainLoader = DEMLoader(0.0, 0.0, -zpos, 3.0, 3.0, pointSize, minHeight, maxHeight);
		t = terrainLoader.getTerrain();
	}
	else {
		TerrainGenerator terrainG = TerrainGenerator(noise, 0.0, 0.0, -zpos, 3.0, 3.0, 0.05, 0.0, 1.0, 3, 0.01, 4.0);
		t = terrainG.points();
	}
	return t;
}
vector<vector<TerrainPoint>> terrain = GetTerrain();



ColorInterpolator colorInterpolator;

vector <Vect3d> v;   //all the points will be stored here

//window size
GLint wWindow = 1200;
GLint hWindow = 800;
int numframes = 10000;
int framesElapsed = 0;

//this defines what will be rendered
//see Key() how is it controlled
bool tangentsFlag = false;
bool pointsFlag = false;
bool curveFlag = true;
bool debugMode = false;
bool showNormals = false;
int framerate = 1;

float elapsed_time;
int iterations;

/*********************************
Some OpenGL-related functions DO NOT TOUCH
**********************************/
//displays the text message in the GL window
void GLMessage(char* message)
{
	static int i;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.f, 100.f, 0.f, 100.f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3ub(0, 0, 255);
	glRasterPos2i(10, 10);
	for (i = 0; i < (int)strlen(message); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, message[i]);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

//called when a window is reshaped
void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glEnable(GL_DEPTH_TEST);
	//remember the settings for the camera
	wWindow = w;
	hWindow = h;
}

//Some simple rendering routines using old fixed-pipeline OpenGL
//draws line from a to b with color 
void DrawLine(Vect3d a, Vect3d b, Vect3d color) {

	glColor3fv(color);
	glBegin(GL_LINES);
	glVertex3fv(a);
	glVertex3fv(b);
	glEnd();
}

//draws point at a with color 
void DrawPoint(Vect3d a, Vect3d color, int pointSize = 5) {

	glColor3fv(color);
	glPointSize(pointSize);
	glBegin(GL_POINTS);
	glVertex3fv(a);
	glEnd();
}

/**********************
LAB related MODIFY
***********************/

//This defines the actual curve 
inline Vect3d P(GLfloat t)
{
	const float rad = 0.2f;
	const float height = 1.f;
	const float rot = 5.f;
	return Vect3d(rad * (float)sin(rot * M_PI * t), height * t, rad * (float)cos(rot * M_PI * t)); //spiral with radius rad, height, and rotations rot
}

//This fills the <vector> *a with data. 
void CreateCurve(vector <Vect3d>* a, int n)
{
	GLfloat step = 1.f / n;
	for (int i = 0; i < n; i++)
	{
		a->push_back(P(i * step));
	}
}

//Call THIS for a new curve. It clears the old one first
void InitArray(int n)
{
	v.clear();
	CreateCurve(&v, n);
}

//returns random number from <-1,1>
inline float random11() {
	return 2.f * rand() / (float)RAND_MAX - 1.f;
}

//randomizes an existing curve by adding random number to each coordinate
void Randomize(vector <Vect3d>* a) {
	const float intensity = 0.01f;
	for (unsigned int i = 0; i < a->size(); i++) {
		Vect3d r(random11(), random11(), random11());
		a->at(i) = a->at(i) + intensity * r;
	}
}

//display coordinate system
void CoordSyst() {
	Vect3d a, b, c;
	Vect3d origin(0, 0, 0);
	Vect3d red(1, 0, 0), green(0, 1, 0), blue(0, 0, 1), almostBlack(0.1f, 0.1f, 0.1f), yellow(1, 1, 0);

	//draw the coordinate system 
	a.Set(1, 0, 0);
	b.Set(0, 1, 0);
	c.Set(Vect3d::Cross(a, b)); //use cross product to find the last vector
	glLineWidth(4);
	DrawLine(origin, a, red);
	DrawLine(origin, b, green);
	DrawLine(origin, c, blue);
	glLineWidth(1);

}

Vect3d TerrainZTangent(TerrainPoint T, std::vector<std::vector<TerrainPoint>>& terrain) {
	int imin = T.coordi - 1;
	if (imin < 0) {
		imin = 0;
	}
	int imax = T.coordi + 1;
	if (imax >= terrain.size()) {
		imax = terrain.size() - 1;
	}
	return terrain[imin][T.coordj].pt - terrain[imax][T.coordj].pt;
}

Vect3d TerrainXTangent(TerrainPoint T, std::vector<std::vector<TerrainPoint>>& terrain) {
	int imin = T.coordj - 1;
	if (imin < 0) {
		imin = 0;
	}
	int imax = T.coordj + 1;
	if (imax >= terrain[T.coordi].size()) {
		imax = terrain[T.coordi].size() - 1;
	}
	return terrain[T.coordi][imin].pt - terrain[T.coordi][imax].pt;
}

Vect3d TerrainNormal(TerrainPoint T, std::vector<std::vector<TerrainPoint>>& terrain) {
	Vect3d tanz = TerrainZTangent(T, terrain);
	Vect3d tanx = TerrainXTangent(T, terrain);
	Vect3d normal = tanz.Cross(tanx).GetNormalized();
	if (isDEM) {
		normal = tanx.Cross(tanz).GetNormalized();
	}
	return normal;
}

void SetTerrainNormals() {
#pragma omp parallel for collapse(2)
	for (int i = 0; i < terrain.size(); i++) {
		for (int j = 0; j < terrain[i].size(); j++) {
			terrain[i][j].coordi = i;
			terrain[i][j].coordj = j;
		}
	}
#pragma omp parallel for collapse(2)
	for (int i = 0; i < terrain.size(); i++) {
		for (int j = 0; j < terrain[i].size(); j++) {
			//DrawPoint(points[i][j].pt, Vect3d(1, 1, 1));
			terrain[i][j].SetNormal(TerrainNormal(terrain[i][j], terrain));
		}
	}
}

//this is the actual code for the lab
void FluidStuff() {

	//std::vector<Vect3d> t;

	//for (int i = 0; i < terrain.size(); i++) {
	//	Vect3d max = Vect3d(0,-10,0);
	//	for (int j = 0; j < terrain[i].size(); j++) {
	//		if (terrain[i][j].y() > max.y()) {
	//			max = terrain[i][j];
	//		}
	//	}
	//	//DrawPoint(max, Vect3d(0, 1, 1), 30);
	//	t.push_back(max);
	//}

	terrain = fluid.GetTerrain();

	if (iterations >= framerate) {
		float new_time = glutGet(GLUT_ELAPSED_TIME) * 0.0001;
		fluid.SetTime(new_time - elapsed_time);

		fluid.AdvectParticles();

		elapsed_time = new_time;
		iterations = 0;
	}

	std::vector<std::vector<std::vector<Particle>>> fp = fluid.GetParticles();

	//#pragma omp parallel for collapse(3)
	for (int i = 0; i < fp.size(); i++) {
		for (int j = 0; j < fp[i].size(); j++) {
			for (int k = 0; k < fp[i][j].size(); k++) {
				Vect3d pos = fp[i][j][k].GetPos();
				if (debugMode) {
					DrawPoint(pos, Vect3d(i*1.0f/particleMatrixSize[0], j * 1.0f / particleMatrixSize[1], k * 1.0f / particleMatrixSize[2]));
					if (showNormals) {
						DrawLine(pos, pos + 0.1 * fp[i][j][k].GetVelocity(), Vect3d(0, 1, 1));
					}
				}
				else {
					DrawPoint(pos, Vect3d(0, 0, 1), 10);
				}
			}
		}
	}

	iterations++;
}

void GeneratePerlinNoise() {
	PerlinNoise2d noise = PerlinNoise2d();
	for (float i = -1.0; i < 1.0; i+=0.01) {
		for (float j = -1.0; j < 1.0; j += 0.01) {
			float noiseVal = noise.value(i, j);
			DrawPoint(Vect3d(i, j, 0), Vect3d(noiseVal, noiseVal, noiseVal));
		}
	}
}

void VisualizeVoxelPoints() {
	float zpos = 1.0f;
	//PerlinNoise2d noise = PerlinNoise2d(0, 32, 2, 2, 10, 10);
	//TerrainGenerator terrain = TerrainGenerator(noise, 0.0, 0.0, 0.0, 2.0, 2.0, 0.01, 0.0, 1.0, 3, 0.01, 4.0);

	if (!debugMode) {
		//std::vector<std::vector<std::vector<TerrainPoint>>> voxelPoints = terrainG.voxelPoints();
		//for (int i = 0; i < voxelPoints.size(); i++) {
		//	for (int j = 0; j < voxelPoints[i].size(); j++) {
		//		for (int k = 0; k < voxelPoints[i][j].size(); k++) {
		//			DrawPoint(voxelPoints[i][j][k].pt, Vect3d((zpos + voxelPoints[i][j][k].pt.y()) * .95, (zpos + voxelPoints[i][j][k].pt.y()) * .5, (zpos + voxelPoints[i][j][k].pt.y()) * 0.05), 25);
		//		}
		//	}
		//}

		for (int i = 0; i < terrain.size(); i++) {
			for (int j = 0; j < terrain[i].size(); j++) {
				for (float k = -zpos; k < terrain[i][j].pt.y(); k+=pointSize) {
					Vect3d color = colorInterpolator.interpolate((k + zpos) / (maxHeight - minHeight) * 0.9 + 0.1);
					if (terrain[i][j].isEroded) {
						DrawPoint(Vect3d(terrain[i][j].pt.x(), k, terrain[i][j].pt.z()), color, 25);
					}
					else if(terrain[i][j].isDeposited) {
						DrawPoint(terrain[i][j].pt, Vect3d(1, 0, 0), 25);
					}
					else {
						DrawPoint(Vect3d(terrain[i][j].pt.x(), k, terrain[i][j].pt.z()), color, 25);
					}
				}
			}
		}
	}
	else {
		//#pragma omp parallel for collapse(2)
		for (int i = 0; i < terrain.size(); i++) {
			for (int j = 0; j < terrain[i].size(); j++) {
				//printf("i = %d, j= %d,  threadId = %d \n", i, j, omp_get_thread_num());
				DrawPoint(terrain[i][j].pt, Vect3d(0.0, i * 1.0f / terrain.size(), j * 1.0f / terrain[i].size()));
				if (showNormals) {
					DrawLine(terrain[i][j].pt, terrain[i][j].pt + 0.1 * terrain[i][j].normal, Vect3d(1, 1, 1));
				}
			}
		}
	
	}
}

void Screenshot() {

	std::ofstream ImageFile;
	std::string name = "images/Image" + std::to_string(framesElapsed) + ".ppm";
	ImageFile.open(name);

	float* pixels = new float[wWindow*hWindow*3];
	glReadPixels(0, 0, wWindow, hWindow, GL_RGB, GL_FLOAT, pixels);

	ImageFile << "P3\n" << wWindow << " " << hWindow << "\n255\n";

	//std::cout << hWindow << " : " << wWindow << " : " << hWindow * wWindow << "\n";

	for (int i = hWindow-1; i > 0; i--) {
		for (int j = 0; j < wWindow*3; j++) {
			ImageFile << (int)(255.0f * pixels[i * wWindow * 3 + j]) << " ";
			//std::cout << (int)(255.0f * pixels[i * wWindow + j]) << " " << (int)(255.0f * pixels[i * wWindow + j + 1]) << " " << (int)(255.0f * pixels[i * wWindow + j + 2]) << "\n";
		}
		ImageFile << "\n";
	}

	ImageFile.close();

}

//the main rendering function
void RenderObjects()
{
	if (framesElapsed >= numframes) {
		std::cout << "Simulation Complete\n";
		exit(0);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//set camera
	glMatrixMode(GL_MODELVIEW);
	trackball.Set3DViewCamera();
	//call the student's code from here
	//Lab01();
	//GeneratePerlinNoise();
	VisualizeVoxelPoints();
	//GenerateVoxelPoints();
	//cout<< glutGet(GLUT_ELAPSED_TIME) <<endl;
	
	//DrawLine(Vect3d(-1.5f, 0.0f, 0.0f), Vect3d(0.0f, 0.0f, 0.0f), Vect3d(1.0f, 0.0f, 0.0f));
	FluidStuff();

	if (takeScreenshot) {
		//takeScreenshot = false;

		//Screenshot();
	}

	framesElapsed++;
	//std::cout << framesElapsed << "\n";
}

//Add here if you want to control some global behavior
//see the pointFlag and how is it used
void Kbd(unsigned char a, int x, int y)//keyboard callback
{
	switch (a)
	{
	case 27: exit(0); break;
	case 't': tangentsFlag = !tangentsFlag; break;
	case 'p': pointsFlag = !pointsFlag; break;
	case 'c': curveFlag = !curveFlag; break;
	case 'd': {
		debugMode = !debugMode; 
		showNormals = false;
		break;
	}
	case 'n': showNormals = !showNormals; break;
	case 'f': {
		framerate = framerate * 10;
		if (framerate > 100) {
			framerate = 1;
		}
		break;
	}
	case 32: {
		if (angleIncrement == 0) angleIncrement = defaultIncrement;
		else angleIncrement = 0;
		break;
	}
	case 's': {sign = -sign; break; }
	case '-': {
		steps--;
		if (steps < 1) steps = 1;
		InitArray(steps);
		break;
	}
	case '+': {
		steps++;
		InitArray(steps);
		break;
	}
	case 'r': {
		Randomize(&v);
		break;
	}
	case 'q': {
		takeScreenshot = true;
		break;
	}
	}
	cout << "[points]=[" << steps << "]" << endl;
	glutPostRedisplay();
}


/*******************
OpenGL code. Do not touch.
******************/
void Idle(void)
{
	glClearColor(0.5f, 0.5f, 0.5f, 1); //background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLMessage("Lab 2 - CS 590CGS");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, (GLfloat)wWindow / (GLfloat)hWindow, 0.01, 100); //set the camera
	glMatrixMode(GL_MODELVIEW); //set the scene
	glLoadIdentity();
	gluLookAt(0, 10, 20, 0, 0, 0, 0, 1, 0); //set where the camera is looking at and from. 
	static GLfloat angle = 0;
	angle += angleIncrement;
	if (angle >= 360.f) angle = 0.f;
	glRotatef(sign * angle, 0, 1, 0);
	RenderObjects();
	glutSwapBuffers();
}

void Display(void)
{

}

void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		trackball.Set(true, x, y);
		mouseLeft = true;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		trackball.Set(false, x, y);
		mouseLeft = false;
	}
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
	{
		trackball.Set(true, x, y);
		mouseMid = true;
	}
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
	{
		trackball.Set(true, x, y);
		mouseMid = false;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		trackball.Set(true, x, y);
		mouseRight = true;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		trackball.Set(true, x, y);
		mouseRight = false;
	}
}

void MouseMotion(int x, int y) {
	if (mouseLeft)  trackball.Rotate(x, y);
	if (mouseMid)   trackball.Translate(x, y);
	if (mouseRight) trackball.Zoom(x, y);
	glutPostRedisplay();
}


int main(int argc, char** argv)
{
	colorInterpolator.addColorPoint(0.0, Vect3d(0.1, 0.1, 0.1)); // Gray
	colorInterpolator.addColorPoint(0.33, Vect3d(0.8, 0.8, 0.1)); // Yellow
	colorInterpolator.addColorPoint(0.66, Vect3d(0.1, 0.5, 0.0)); // Dark Green
	colorInterpolator.addColorPoint(1.0, Vect3d(0.2, .8, 0.0)); // Less Dark Green

	//fluid = Fluid(particleMatrixSize);
	SetTerrainNormals();
	fluid.SetTerrain(terrain);

	glutInitDisplayString("stencil>=2 rgb double depth samples");
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInit(&argc, argv);
	glutInitWindowSize(wWindow, hWindow);
	glutInitWindowPosition(500, 100);
	glutCreateWindow("Surface of Revolution");

	elapsed_time = 0;
	iterations = 0;

	//GLenum err = glewInit();
	// if (GLEW_OK != err){
	// fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	//}
	glutDisplayFunc(Display);
	glutIdleFunc(Idle);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Kbd); //+ and -
	glutSpecialUpFunc(NULL);
	glutSpecialFunc(NULL);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMotion);
	InitArray(steps);
	glutMainLoop();
	return 0;
}
