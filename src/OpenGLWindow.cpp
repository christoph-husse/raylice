// ======================================================================== //
// Copyright 2013 Christoph Husse                                           //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //


#include "stdafx.h"


OpenGLWindow* OpenGLWindow::openWindow = nullptr;

double clamp(double value, double min, double max) { return std::max(min, std::min(max, value)); }
double round(double value) { return (value > 0.0) ? std::floor(value + 0.5) : std::ceil(value - 0.5); }

OpenGLWindow::OpenGLWindow(std::string title, std::vector<char*> cmdArgs, int wndWidth, int wndHeight)
	: handle(0), width(wndWidth), height(wndHeight), isMouseDown(false), isLeftButtonDown(false), isInitialized(false)
{
	isClosed = false;
	isRunning = false;

	thread = std::thread(
		[=]()
		{
			openWindow = this;

			// Wuuuza... come on GLUT...!! This is not funny.
			int argc = cmdArgs.size();
			auto cmdCopy = std::vector<char*>(cmdArgs);
			char** argv = cmdCopy.data();
			glutInit(&argc, argv);
			glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
			glutInitWindowPosition (0, 0);
			glutInitWindowSize(wndWidth, wndHeight);

			handle = glutCreateWindow(title.c_str());

			glutReshapeFunc(OnResizeStatic);
			glutDisplayFunc(OnRenderStatic);
			glutTimerFunc(30, OnUpdateStatic, 0);
			glutMotionFunc(OnMouseDragStatic);
			glutPassiveMotionFunc(OnMouseMoveStatic);
			glutMouseFunc(OnMouseDownStatic);
			glutKeyboardFunc(OnKeyDownStatic);

			isRunning = true;

			glutMainLoop();

			isRunning = false;
			isClosed = true;
		}
	);
}

void OpenGLWindow::OnKeyDownStatic(unsigned char key, int x, int y)
{
	auto wnd = getCurrent();
	wnd->OnKeyDown(key);
}

void OpenGLWindow::OnMouseDownStatic(int button, int state, int x, int y)
{
	auto wnd = getCurrent();

	wnd->isLeftButtonDown = button == 0;
}

void OpenGLWindow::OnMouseDragStatic(int x, int y)
{
	auto wnd = getCurrent();

	wnd->mouseXY = Vector3(x, y, 0);
	wnd->isMouseDown = true;
}

void OpenGLWindow::OnMouseMoveStatic(int x, int y)
{
	auto wnd = getCurrent();

	wnd->mouseXY = Vector3(x, y, 0);
	wnd->isMouseDown = false;
}

int OpenGLWindow::ReadPixelAtMouse() const
{
	int32_t value = 0;
	glReadPixels(
		(int)clamp(mouseXY.x, 0, width - 1), 
		(int)clamp(height - mouseXY.y, 0, height - 1), 
		1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &value);
	return value;
}

void OpenGLWindow::OnUpdateStatic(int unused)
{
	auto wnd = getCurrent();

	glutSetWindow(wnd->handle);
    glutPostRedisplay();

	glutTimerFunc(30, OnUpdateStatic, 0);
}

void OpenGLWindow::OnResizeStatic(int w, int h)
{
    // Setup projection matrix for new window
	double aspect = w / (double)h;
	double scale = 3;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-aspect * scale, aspect * scale, -1 * scale, 1 * scale);

    // Update OpenGL viewport and internal variables
    glViewport(0,0, w,h);

	auto wnd = getCurrent();
    wnd->width = w;
    wnd->height = h;
	wnd->OnResize();
}

void OpenGLWindow::OnRenderStatic()
{
    glClearColor(0.7f,0.7f,0.9f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

	auto wnd = getCurrent();

	if(!wnd->isInitialized)
	{
		wnd->isInitialized = true;
		wnd->OnInitialize();
	}

	wnd->OnRender();

	glFlush();
    glutSwapBuffers();
}

void OpenGLWindow::WaitUntilClosed()
{
	thread.join();
}

bool OpenGLWindow::IsClosed() const
{
	return isClosed;
}

Vector3 OpenGLWindow::GetMouseXY()
{
	return mouseXY;
}

bool OpenGLWindow::IsMouseDown()
{
	return isMouseDown;
}

bool OpenGLWindow::IsLeftButtonDown()
{
	return isLeftButtonDown;
}