/*
# Mehmet Mazhar SALIKCI
# 21.3.2016

Introduction:
--DDA algorithm
--Brehensam Algorithm
--Benchmark 
*/
#include <math.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <gl/glut.h>
#define MIN( x, y ) ((x) <= (y) ? (x) :  (y))
#define MAX( x, y ) ((x) >= (y) ? (x) :  (y))
#define ABS( x )    ((x) >= 0.0 ? (x) : -(x))
#define ROUND(a) ((int) (a + 0.5))

// Rather than specifying RGB values for colors, this trivial
// program will simply use an enumerated type, since we only
// need four different colors.
enum ColorFlag {
	unset,
	highlight,
	picked,
	rasterized
};

// Define some basic parameters that determine the size and
// spacing of the pixels, the maximum size of the raster, etc.
static const int
NUM_PIXELS = 200,
WINDOW_X_SIZE = 800,
WINDOW_Y_SIZE = 800,
PIXEL_SPACING = 8,
LINE_NUMBER = 500;


static int option = 0;
static int MOUSE_CLICK_SIZE = 0;
static double timeDDA = 0;
static double timeBrehensam = 0;
// Declare the global variables that will be used to keep
// track of which pixel is highlighted, which pixels have been
// chosen as the start and end points, etc.
static int
curr_i = 0,
curr_j = 0,
last_i = -1,
last_j = -1,
pick_i = -1,
pick_j = -1,
count = 0,
first_i = 0,
first_j = 0,
second_i = 0,
second_j = 0;

// Define the array that will hold the colors associated with
// each pixel in the raster, and also the original color of the
// pixel that is currently being highlighted.
static ColorFlag
last_color = unset,
raster[NUM_PIXELS][NUM_PIXELS] = { unset };

// Translate the enumerated type into an actual RGB triple and
// tell OpenGL to use this color for drawing subsequent polygons
// (which will be used to depict pixels).
void SetColor(ColorFlag c)
{
	switch (c)
	{
	case unset: glColor3f(0.2, 0.2, 0.2); break;
	case highlight: glColor3f(0.1, 1.0, 0.1); break;
	case picked: glColor3f(1.0, 0.0, 0.0); break;
	case rasterized: glColor3f(0.3, 0.6, 0.6); break;
	}
}

// Draw a circle centered at the point (x,y) with radius "rad".
// The circle will be drawn using a polygon with N vertices.
void DrawSquare(int x, int y, int temp)
{
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + 4, y);
	glVertex2f(x + 4, y + 4);
	glVertex2f(x, y + 4);
	
	glEnd();
}

void SetPixel(int i, int j, ColorFlag color)
{
	if (i < 0 || i >= NUM_PIXELS) return;
	if (j < 0 || j >= NUM_PIXELS) return;
	int x = i * PIXEL_SPACING;
	int y = j * PIXEL_SPACING;
	SetColor(color);
	DrawSquare(x,y, 1);
	raster[i][j] = color;
	if (i == last_i && j == last_j) last_color = color;
}

ColorFlag GetPixel(int i, int j)
{
	return raster[i][j];
}

void ClearRaster()
{
	count = 0;
	glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer
	for (int i = 0; i < NUM_PIXELS; i++)
	{
		for (int j = 0; j < NUM_PIXELS; j++)
		{
			SetPixel(i, j, unset);
		}
	}
}

void BresenhamAlgorithm(int x0, int y0, int x1, int y1)
{
	int delta_x = x1 - x0;
	int delta_y = y1 - y0;
	
	int steps = MAX(ABS(delta_x), ABS(delta_y));
	double c = 1.0 / MAX(1, steps);
	for (int i = 0; i <= steps; i++)
	{
		int x = int(x0 + (i * delta_x * c) + 0.5);
		int y = int(y0 + (i * delta_y * c) + 0.5);
		SetPixel(x, y, rasterized);
	}
}
void DDAAlgorithm(int x0, int y0, int x1, int y1) {
	
	//dont check whether (x1 - x0) is zero
	double m = (double)(y1 - y0) / (x1 - x0);
	double y = (double)y0;
	double x = (double)x0;
	
	if (m<1) {
		while (x <= x1) {
			SetPixel(x, floor(y + 0.5), rasterized);
			y = y + m;
			x++;
		}
	}
	else{
		double m1 = 1.0 / m;
		while (y <= y1) {
			SetPixel(floor(fabs(x) + 0.5),y, rasterized);
			y++;
			x = x + m1;
		}
	}
}
void display()
{
	static int initialized = 0;
	if (!initialized)
	{
		ClearRaster();
		initialized = 1;
	}
	if (curr_i != last_i || curr_j != last_j)
	{
		SetPixel(last_i, last_j, last_color);
		last_color = GetPixel(curr_i, curr_j);
		SetPixel(curr_i, curr_j, highlight);
		last_i = curr_i;
		last_j = curr_j;
	}
	glFlush();
}

void mouse_motion(int x, int y)
{
	// Nothing to do while button is pressed.
}

void mouse_passive_motion(int x, int y)
{
	double space = double(PIXEL_SPACING);
	curr_i = int(0.5 + x / space);
	curr_j = int(0.5 + (WINDOW_Y_SIZE - y) / space);
	glutPostRedisplay();
}

// Handle key presses: Q for "Quit", and C for "Clear".
void key_press(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
	case 'Q':
		return;
	case 'c':
	case 'C':
		ClearRaster();
		glutPostRedisplay();
		break;
	}
}

void PickPixel(int i, int j)
{
	SetPixel(i, j, picked);
	if (count == 0)
	{
		// This is the first pixel of a pair.
		first_i = i;
		first_j = j;
		count = 1;
	}
	else if (count == 1)
	{
		// This is the second pixel of a pair.
		second_i = i;
		second_j = j;
		count = 0;
		if (option == 1)
		{
			DDAAlgorithm(first_i, first_j, second_i, second_j);
		}
		else if (option == 2)
		{
			BresenhamAlgorithm(first_i, first_j, second_i, second_j);
		}
		else if(option == 3)
		{
			if (MOUSE_CLICK_SIZE == 1)
			{	
				BresenhamAlgorithm(first_i, first_j, second_i, second_j);
			}
			else if(MOUSE_CLICK_SIZE == 2)
			{
				DDAAlgorithm(first_i, first_j, second_i, second_j);	
			}
		}
	}
}

void mouse_button(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		++MOUSE_CLICK_SIZE;
		srand(time(NULL));
		if (option == 3) {
			clock_t tStart = clock();
			
			for (size_t i = 0; i < LINE_NUMBER; i++)
			{
				PickPixel(rand() % NUM_PIXELS, rand() % NUM_PIXELS);
				glutPostRedisplay();
			}

			if (MOUSE_CLICK_SIZE == 1)
			{
				std::cout << "Bresenham Algorithm Execution Time:";
				timeBrehensam = (double)(clock() - tStart) / CLOCKS_PER_SEC;
				std::cout << (double)(clock() - tStart) / CLOCKS_PER_SEC << std::endl;
			}
			else if (MOUSE_CLICK_SIZE == 2) {
				std::cout << "DDA Algorithm Execution Time:";
				timeDDA = (double)(clock() - tStart) / CLOCKS_PER_SEC;
				std::cout << (double)(clock() - tStart) / CLOCKS_PER_SEC << std::endl << std::endl;
				
				if (timeDDA > timeBrehensam)
				{
					std::cout << "Brehensam's Algorithm faster than DDA Algorithm." << std::endl;
				}
				else if(timeDDA < timeBrehensam)
				{
					std::cout << "DDA Algorithm faster than Brehensam's Algorithm." << std::endl;

				}
				else
				{
					std::cout << "Same speed" << std::endl << std::endl;
				}
				
				MOUSE_CLICK_SIZE = 0;
			}
		}
		else
		{
			PickPixel(curr_i, curr_j);
			glutPostRedisplay();
		}
	}
}

void main(int argc, char** argv)
{
	bool loop = true;
	while (loop) {

		std::cout << "1.Use DDA Algorithm" << std::endl;
		std::cout << "2.Use Bresenham Algorithm" << std::endl;
		std::cout << "3.Start Benchmark" << std::endl;
		std::cout << "4.Exit" << std::endl;
		std::cin >> option;
		if (option == 1)
		{
			glutInit(&argc, argv);
			glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
			glutInitWindowSize(WINDOW_X_SIZE, WINDOW_Y_SIZE);
			glutInitWindowPosition(200, 200);
			glutCreateWindow("DDA Line Algorithm");
			glClearColor(0.0, 0.0, 0.1, 0); // clear window

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0.0, WINDOW_X_SIZE, 0.0, WINDOW_Y_SIZE, -1.0, 1.0);

			glutDisplayFunc(display);
			glutMotionFunc(mouse_motion);
			glutPassiveMotionFunc(mouse_passive_motion);
			glutMouseFunc(mouse_button);
			glutKeyboardFunc(key_press);
			glutMainLoop();
		}
		else if(option == 2)
		{
			glutInit(&argc, argv);
			glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
			glutInitWindowSize(WINDOW_X_SIZE, WINDOW_Y_SIZE);
			glutInitWindowPosition(200, 200);
			glutCreateWindow("Bresenham's Line Algorithm");
			glClearColor(0.0, 0.0, 0.1, 0); // clear window

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0.0, WINDOW_X_SIZE, 0.0, WINDOW_Y_SIZE, -1.0, 1.0);

			glutDisplayFunc(display);
			glutMotionFunc(mouse_motion);
			glutPassiveMotionFunc(mouse_passive_motion);
			glutMouseFunc(mouse_button);
			glutKeyboardFunc(key_press);
			glutMainLoop();
		}
		else if(option == 3)
		{
			//when window opened ,if you want to exe DDA algorithm one click,
			//or to exe Bresenham Algorithm press two click
			//to clear window,press c or C,
			glutInit(&argc, argv);
			glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
			glutInitWindowSize(WINDOW_X_SIZE, WINDOW_Y_SIZE);
			glutInitWindowPosition(200, 200);
			glutCreateWindow("Benchmark");
			glClearColor(0.0, 0.0, 0.1, 0); // clear window
		
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0.0, WINDOW_X_SIZE, 0.0, WINDOW_Y_SIZE, -1.0, 1.0);

			glutDisplayFunc(display);
			glutMotionFunc(mouse_motion);
			glutPassiveMotionFunc(mouse_passive_motion);
			glutMouseFunc(mouse_button);
			glutKeyboardFunc(key_press);
			
			glutMainLoop();
		}
		else
		{
			loop = false;
		}
	}
}