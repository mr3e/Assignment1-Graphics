#include <glfw/deps/glad/gl.h>
#include "InputManager.h"
#include "display.h"
#include "game.h"
#include "glm\glm.hpp"
#include "../Engine3D/stb_image.h"
#include <stdio.h>
#include <iostream>
#include <glfw/deps/linmath.h>

int main(int argc, char* argv[])
{
	const int DISPLAY_WIDTH = 512;
	const int DISPLAY_HEIGHT = 512;
	const float CAMERA_ANGLE = 10.0f;
	const float NEAR = 1.0f;
	const float FAR = 100.0f;

	Game* scn = new Game(CAMERA_ANGLE, (float)DISPLAY_WIDTH / DISPLAY_HEIGHT, NEAR, FAR);
	Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "Assignment 1!");
	
	Init(display);

	scn->Init();

	display.SetScene(scn);
	while (!display.CloseWindow()){
		//0 = gray
		//1 = edges
		//2 = halftone
		//3 = floyd	
		scn->SetShapeTex(0, 0);//gray
		glViewport(0, 256, 256, 256);
		scn->Draw(1, 0, scn->BACK, true, false);
		scn->SetShapeTex(0, 1);//edges
		glViewport(256, 256, 256, 256);
		scn->Draw(1, 0, scn->BACK, false, false);
		scn->SetShapeTex(0, 2);//halftone
		glViewport(0, 0, 256, 256);
		scn->Draw(1, 0, scn->BACK, false, false);
		scn->SetShapeTex(0, 3);//floyd
		glViewport(256, 0, 256, 256);
		scn->Draw(1, 0, scn->BACK, false, false);
		scn->Motion();
		display.SwapBuffers();
		display.PollEvents();
	}
	delete scn;
	return 0;
}