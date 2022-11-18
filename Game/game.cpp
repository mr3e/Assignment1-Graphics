#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "../Engine3D/stb_image.h"
#include <stdio.h>
#include <iostream>

static void printMat(const glm::mat4 mat)
{
	std::cout << " matrix:" << std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout << mat[j][i] << " ";
		std::cout << std::endl;
	}
}

Game::Game() : Scene()
{
}

Game::Game(float angle, float relationWH, float near1, float far1) : Scene(angle, relationWH, near1, far1)
{
}

unsigned char* averageRGB(unsigned char* data, int size) {
	unsigned char* average = (unsigned char*)malloc(size/4);
	for (int i = 0; i < size/4; i++) {
		average[i] = (data[4 * i] + data[4 * i + 1] + data[4 * i + 2]) / 3;
	}
	return average;
}

void saveToFile(char* fileName, unsigned char* data, int size, int floyd) {
	FILE* file = fopen(fileName, "w");
	unsigned char* dataNoAlpha = averageRGB(data, size);
	for (int i = 0; i < size/4-1; i++) {
		if(!floyd) (dataNoAlpha[i] == 0) ? fprintf(file, "0,") : fprintf(file, "1,");
		else fprintf(file, "%d,", dataNoAlpha[i]/16);
	}
	(dataNoAlpha[size/4-2] == 0) ? fprintf(file, "0") : fprintf(file, "1");
	fclose(file);
}

unsigned char* grayIt(unsigned char* data) {
	unsigned char* grayScale = (unsigned char*)malloc(512 * 512);
	for (int i = 0, counter = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++, counter += 4) {
			unsigned char val = (unsigned char)(data[4 * (i * 256 + j)] * 0.3 + data[4 * (i * 256 + j) + 1] * 0.58 + data[4 * (i * 256 + j) + 2] * 0.11);
			grayScale[counter] = val;
			grayScale[counter + 1] = val;
			grayScale[counter + 2] = val;
			grayScale[counter + 3] = val;
		}
	}
	return grayScale;
}
unsigned char* finalTouch(unsigned char* data, unsigned char* original) {
	unsigned char* finalTouch = (unsigned char*)calloc(512 * 512, 1);
	unsigned char(*finalTouch2D)[1024] = (unsigned char(*)[1024])finalTouch;
	unsigned char(*data2D)[256] = (unsigned char(*)[256])data;
	unsigned char(*original2D)[1024] = (unsigned char(*)[1024])original;

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			if (data2D[i][j] == 75) {
				if ((data2D[(i - 1)][j - 1] == 255) ||
					(data2D[(i - 1)][j] == 255) ||
					(data2D[(i - 1)][j + 1] == 255) ||
					(data2D[i][j - 1] == 255) ||
					(data2D[i][j + 1] == 255) ||
					(data2D[(i + 1)][j - 1] == 255) ||
					(data2D[(i + 1)][j] == 255) ||
					(data2D[(i + 1)][j + 1] == 255)) {
					finalTouch2D[i][4 * j] = 255; finalTouch2D[i][4 * j + 1] = 255; finalTouch2D[i][4 * j + 2] = 255; finalTouch2D[i][4 * j + 3] = original2D[i][4 * j + 3];
				}
				else {
					finalTouch2D[i][4 * j] = 0; finalTouch2D[i][4 * j + 1] = 0; finalTouch2D[i][4 * j + 2] = 0; finalTouch2D[i][4 * j + 3] = original2D[i][4 * j + 3];
				}
			}
			else {
				finalTouch2D[i][4 * j] = data2D[i][j]; finalTouch2D[i][4 * j + 1] = data2D[i][j]; finalTouch2D[i][4 * j + 2] = data2D[i][j]; finalTouch2D[i][4 * j + 3] = original2D[i][4 * j + 3];
			}
		}
	}
	saveToFile("../img4.txt", finalTouch, 512 * 512, 0);
	return finalTouch;
}
unsigned char* threshold(unsigned char* data, unsigned char* original) {
	unsigned char* threshold = (unsigned char*)calloc(256 * 256, 1);
	unsigned char(*threshold2D)[256] = (unsigned char(*)[256])threshold;
	unsigned char(*data2D)[256] = (unsigned char(*)[256])data;
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			if (data2D[i][j] >= 180) threshold2D[i][j] = 255;
			else if (data2D[i][j] <= 25) threshold2D[i][j] = 0;
			else threshold2D[i][j] = 75;
		}
	}
	return finalTouch(threshold, original);
}

unsigned char* non_max_suppression(unsigned char* data, unsigned char* original) {
	unsigned char* suppressed = (unsigned char*)calloc(256 * 256 * 4, 1);
	unsigned char* suppression = (unsigned char*)calloc(256 * 256, 1);
	unsigned char* angle = (unsigned char*)calloc(256 * 256, 1);
	unsigned char(*suppression2D)[256] = (unsigned char(*)[256])suppression;
	unsigned char(*angle2D)[256] = (unsigned char(*)[256])angle;
	unsigned char(*data2D)[256] = (unsigned char(*)[256])data;

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			unsigned char val = data2D[i][j] * (unsigned char)(180 / 3.14);
			angle2D[i][j] = (val < 0) ? val + 180 : val;
		}
	}

	for (int i = 1; i < 255; i++) {
		for (int j = 1; j < 255; j++) {
			try {
				int q = 255;
				int r = 255;

				//angle 0
				if ((0 <= angle2D[i][j] < 22.5) | (157.5 <= angle2D[i][j] <= 180))
				{
					q = data2D[i][j + 1];
					r = data2D[i][j - 1];
				}
				else if (22.5 <= angle2D[i][j] < 67.5) {

					q = data2D[i + 1][j - 1];
					r = data2D[i - 1][j + 1];
					//angle 90
				}
				else if (67.5 <= angle2D[i][j] < 112.5) {
					q = data2D[i + 1][j];
					r = data2D[i - 1][j];
					//angle2D 135
				}
				else if (112.5 <= angle2D[i][j] < 157.5) {
					q = data2D[i - 1][j - 1];
					r = data2D[i + 1][j + 1];
				}
				if ((data2D[i][j] >= q) & (data2D[i][j] >= r)) {
					suppression2D[i][j] = data2D[i][j];
				}
				else suppression2D[i][j] = 0;
			}
			catch (const std::exception&) {}
		}
	}

	for (int i = 0, counter = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++, counter+=4) {
			suppressed[counter] = suppression2D[i][j];
			suppressed[counter+1] = suppression2D[i][j];
			suppressed[counter+2] = suppression2D[i][j];
			suppressed[counter + 3] = original[counter + 3];
		}
	}
	return threshold(suppression, original);
}

unsigned char* edgeIt(unsigned char* data) {
	char* dataNoAlpha = (char*)averageRGB(data, 512 * 512);
	unsigned char* edge = (unsigned char*)malloc(256 * 256);
	int counter = 0;
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++, counter ++) {
			char gx = dataNoAlpha[(i - 1) * 256 + j - 1] + 2 * dataNoAlpha[(i - 1) * 256 + j] + dataNoAlpha[(i - 1) * 256 + j + 1]
				- dataNoAlpha[(i + 1) * 256 + j - 1] - 2 * dataNoAlpha[(i + 1) * 256 + j] - dataNoAlpha[(i + 1) * 256 + j + 1];
			char gy = -dataNoAlpha[(i + 1) * 256 + j - 1] - 2 * dataNoAlpha[i * 256 + j - 1] - dataNoAlpha[(i - 1) * 256 + j - 1]
				+ dataNoAlpha[(i + 1) * 256 + j + 1] + 2 * dataNoAlpha[i * 256 + j + 1] + dataNoAlpha[(i - 1) * 256 + j + 1];
			unsigned char val = std::abs(gx) + std::abs(gy);
			edge[counter] = val;
		}
	}
	return non_max_suppression(edge, data);
}

unsigned char* halfToneIt(unsigned char* data) {
	unsigned char* halftone = (unsigned char*)calloc(512 * 512 * 4, 1);
	unsigned char* dataNoAlpha = averageRGB(data, 512 * 512);
	for (int i = 0; i < 256; i++) {//4096
		for (int j = 0; j < 256; j++) {
			int val = dataNoAlpha[i * 256 + j] / 51 - 1;
			halftone[4096 * (i + 1) + j * 8 + 3] = data[512 * i + j];
			halftone[4096 * i + j * 8 + 4+3] = data[512 * i + j];
			halftone[4096 * (i + 1) + j * 8 + 4 + 3] = data[512 * i + j];
			halftone[4096 * i + j * 8 + 3] = data[512 * i + j];
			if (val > 0) { halftone[4096 * (i + 1) + j * 8] = 255; halftone[4096 * (i + 1) + j * 8 + 1] = 255; halftone[4096 * (i + 1) + j * 8 + 2] = 255; val--; }
			if (val > 0) { halftone[4096 * i + j * 8+4] = 255; halftone[4096 * i + j * 8 + 1+4] = 255; halftone[4096 * i + j * 8 + 2+4] = 255; val--; }
			if (val > 0) { halftone[4096 * (i + 1) + j * 8 + 4] = 255; halftone[4096 * (i + 1) + j * 8 + 1 + 4] = 255; halftone[4096 * (i + 1) + j * 8 + 2+4] = 255; val--; }
			if (val > 0) { halftone[4096 * i + j * 8] = 255; halftone[4096 * i + j * 8+1] = 255; halftone[4096 * i + j * 8+2] = 255; val--; }
		}
	}
	saveToFile("../img5.txt", halftone, 512 * 512*4, 0);
	return halftone;
}

unsigned char* floydIt(unsigned char* data) {
	unsigned char floyd[512][512];
	char* dataNoAlpha = (char*)averageRGB(data, 512 * 512);
	char(*data2D)[256] = (char(*)[256])dataNoAlpha;

	for (int y = 256; y >= 0; y--) {
		for (int x = 0; x < 256; x++) {
			int oldPixel = data2D[x][y];
			int newPixel = oldPixel / 255;
			floyd[x][y] = newPixel;
			int quant_error = oldPixel - newPixel;
			floyd[x + 1][y] = data2D[x + 1][y] + quant_error * 7 / 16;
			floyd[x - 1][y + 1] = data2D[x - 1][y + 1] + quant_error * 0.5 / 16; 
			floyd[x][y + 1] = data2D[x][y + 1] + quant_error * 5 / 16;
			floyd[x + 1][y + 1] = data2D[x + 1][y + 1] + quant_error * 3 / 16;
		}
	}
	unsigned char* floyded = (unsigned char*)malloc(512 * 512);
	for (int i = 0, counter = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++, counter+=4) {
			floyded[counter] = floyd[i][j]; floyded[counter+1] = floyd[i][j]; floyded[counter+2] = floyd[i][j]; floyded[counter+3] = data[counter+3];
		}
	}
	saveToFile("../img6.txt", floyded, 512 * 512, 1);
	return floyded;
}

void Game::Init()
{
	int width, height, numComponents;
	unsigned char* data = stbi_load("../res/textures/lena256.jpg", &width, &height, &numComponents, 4);
	
	AddShader("../res/shaders/pickingShader");
	AddShader("../res/shaders/basicShader");

	AddTexture(256, 256, grayIt(data));
	AddTexture(256, 256, edgeIt(data));
	AddTexture(512, 512, halfToneIt(data));
	AddTexture(256, 256, floydIt(data));

	AddShape(Plane, -1, TRIANGLES);

	pickedShape = 0;

	SetShapeTex(0, 0);
	MoveCamera(0, zTranslate, 10);
	pickedShape = -1;

	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::Update(const glm::mat4& MVP, const glm::mat4& Model, const int  shaderIndx)
{
	Shader* s = shaders[shaderIndx];
	s->Bind();
	s->SetUniformMat4f("MVP", MVP);
	s->SetUniformMat4f("Normal", Model);
	s->Unbind();
}

void Game::WhenRotate()
{
}

void Game::WhenTranslate()
{
}

void Game::Motion()
{
	if (isActive)
	{

	}
}

Game::~Game(void)
{
}
