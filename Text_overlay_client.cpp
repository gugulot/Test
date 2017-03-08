/*******************************************************************************************************************************************

  Initial Version - 10/27/2016 - Email - alok.gupta@xilinx.com
  Xilinx Inc.

  MULTI COLOR CUBE APP

  Matrix calculations are being pulled from ARM SDK
 *******************************************************************************************************************************************/

#define _GLIBCXX_USE_CXX11_ABI 0

#include "triCube.h"
#include "assert.h"
#include "stdio.h"  
#include "stdlib.h"
#include "stdarg.h"
#include <cstdio>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* for tty */
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

/* for fbdev */
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* for EGL */
#include <EGL/egl.h>
#include <GLES2/gl2.h>


/* A text object to draw text on the screen. */
Text *text[10];


char buffer [50];
unsigned int MYFPS;
using std::string;
float timeLocation = 0.0f;

//These are just matrix functions...

/* Identity matrix. */
const float Matrix::identityArray[16] =
{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

Matrix Matrix::identityMatrix = Matrix(identityArray);

Matrix::Matrix(const float* array)
{
	memcpy(elements, array, 16 * sizeof(float));
}

Matrix::Matrix(void)
{
}

float& Matrix::operator[] (unsigned element)
{
	if (element > 15)
	{
		printf("Matrix only has 16 elements, tried to access element %d", element);
		exit(1);
	}
	return elements[element];
}

Matrix Matrix::operator* (Matrix right)
{
	return multiply(this, &right);
}

Matrix& Matrix::operator= (const Matrix &another)
{
	if(this != &another)
	{
		memcpy(this->elements, another.elements, 16 * sizeof(float));
	}

	return *this;
}

float* Matrix::getAsArray(void)
{
	return elements;
}

float Matrix::matrixDeterminant(float *matrix)
{
	float result = 0.0f;

	result  = matrix[0] * (matrix[4] * matrix[8] - matrix[7] * matrix[5]);
	result -= matrix[3] * (matrix[1] * matrix[8] - matrix[7] * matrix[2]);
	result += matrix[6] * (matrix[1] * matrix[5] - matrix[4] * matrix[2]);

	return result;
}

float Matrix::matrixDeterminant(Matrix *matrix)
{
	float matrix3x3[9];
	float determinant3x3 = 0.0f;
	float result = 0.0f;

	/* Remove (i, j) (1, 1) to form new 3x3 matrix. */
	matrix3x3[0] = matrix->elements[ 5];
	matrix3x3[1] = matrix->elements[ 6];
	matrix3x3[2] = matrix->elements[ 7];
	matrix3x3[3] = matrix->elements[ 9];
	matrix3x3[4] = matrix->elements[10];
	matrix3x3[5] = matrix->elements[11];
	matrix3x3[6] = matrix->elements[13];
	matrix3x3[7] = matrix->elements[14];
	matrix3x3[8] = matrix->elements[15];
	determinant3x3 = matrixDeterminant(matrix3x3);
	result += matrix->elements[0] * determinant3x3;

	/* Remove (i, j) (1, 2) to form new 3x3 matrix. */
	matrix3x3[0] = matrix->elements[ 1];
	matrix3x3[1] = matrix->elements[ 2];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 9];
	matrix3x3[4] = matrix->elements[10];
	matrix3x3[5] = matrix->elements[11];
	matrix3x3[6] = matrix->elements[13];
	matrix3x3[7] = matrix->elements[14];
	matrix3x3[8] = matrix->elements[15];
	determinant3x3 = matrixDeterminant(matrix3x3);
	result -= matrix->elements[4] * determinant3x3;

	/* Remove (i, j) (1, 3) to form new 3x3 matrix. */
	matrix3x3[0] = matrix->elements[ 1];
	matrix3x3[1] = matrix->elements[ 2];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 5];
	matrix3x3[4] = matrix->elements[ 6];
	matrix3x3[5] = matrix->elements[ 7];
	matrix3x3[6] = matrix->elements[13];
	matrix3x3[7] = matrix->elements[14];
	matrix3x3[8] = matrix->elements[15];
	determinant3x3 = matrixDeterminant(matrix3x3);
	result += matrix->elements[8] * determinant3x3;
	/* Remove (i, j) (1, 4) to form new 3x3 matrix. */
	matrix3x3[0] = matrix->elements[ 1];
	matrix3x3[1] = matrix->elements[ 2];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 5];
	matrix3x3[4] = matrix->elements[ 6];
	matrix3x3[5] = matrix->elements[ 7];
	matrix3x3[6] = matrix->elements[ 9];
	matrix3x3[7] = matrix->elements[10];
	matrix3x3[8] = matrix->elements[11];
	determinant3x3 = matrixDeterminant(matrix3x3);
	result -= matrix->elements[12] * determinant3x3;

	return result;
}

Matrix Matrix::matrixInvert(Matrix *matrix)
{
	Matrix result;
	float matrix3x3[9];

	/* Find the cofactor of each element. */
	/* Element (i, j) (1, 1) */
	matrix3x3[0] = matrix->elements[ 5];
	matrix3x3[1] = matrix->elements[ 6];
	matrix3x3[2] = matrix->elements[ 7];
	matrix3x3[3] = matrix->elements[ 9];
	matrix3x3[4] = matrix->elements[10];
	matrix3x3[5] = matrix->elements[11];
	matrix3x3[6] = matrix->elements[13];
	matrix3x3[7] = matrix->elements[14];
	matrix3x3[8] = matrix->elements[15];
	result.elements[0] = matrixDeterminant(matrix3x3);

	/* Element (i, j) (1, 2) */
	matrix3x3[0] = matrix->elements[ 1];
	matrix3x3[1] = matrix->elements[ 2];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 9];
	matrix3x3[4] = matrix->elements[10];
	matrix3x3[5] = matrix->elements[11];
	matrix3x3[6] = matrix->elements[13];
	matrix3x3[7] = matrix->elements[14];
	matrix3x3[8] = matrix->elements[15];
	result.elements[4] = -matrixDeterminant(matrix3x3);

	/* Element (i, j) (1, 3) */
	matrix3x3[0] = matrix->elements[ 1];
	matrix3x3[1] = matrix->elements[ 2];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 5];
	matrix3x3[4] = matrix->elements[ 6];
	matrix3x3[5] = matrix->elements[ 7];
	matrix3x3[6] = matrix->elements[13];
	matrix3x3[7] = matrix->elements[14];
	matrix3x3[8] = matrix->elements[15];
	result.elements[8] = matrixDeterminant(matrix3x3);

	/* Element (i, j) (1, 4) */
	matrix3x3[0] = matrix->elements[ 1];
	matrix3x3[1] = matrix->elements[ 2];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 5];
	matrix3x3[4] = matrix->elements[ 6];
	matrix3x3[5] = matrix->elements[ 7];
	matrix3x3[6] = matrix->elements[ 9];
	matrix3x3[7] = matrix->elements[10];
	matrix3x3[8] = matrix->elements[11];
	result.elements[12] = -matrixDeterminant(matrix3x3);

	/* Element (i, j) (2, 1) */
	matrix3x3[0] = matrix->elements[ 4];
	matrix3x3[1] = matrix->elements[ 6];
	matrix3x3[2] = matrix->elements[ 7];
	matrix3x3[3] = matrix->elements[ 8];
	matrix3x3[4] = matrix->elements[10];
	matrix3x3[5] = matrix->elements[11];
	matrix3x3[6] = matrix->elements[12];
	matrix3x3[7] = matrix->elements[14];
	matrix3x3[8] = matrix->elements[15];
	result.elements[1] = -matrixDeterminant(matrix3x3);

	/* Element (i, j) (2, 2) */
	matrix3x3[0] = matrix->elements[ 0];
	matrix3x3[1] = matrix->elements[ 2];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 8];
	matrix3x3[4] = matrix->elements[10];
	matrix3x3[5] = matrix->elements[11];
	matrix3x3[6] = matrix->elements[12];
	matrix3x3[7] = matrix->elements[14];
	matrix3x3[8] = matrix->elements[15];
	result.elements[5] = matrixDeterminant(matrix3x3);

	/* Element (i, j) (2, 3) */
	matrix3x3[0] = matrix->elements[ 0];
	matrix3x3[1] = matrix->elements[ 2];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 4];
	matrix3x3[4] = matrix->elements[ 6];
	matrix3x3[5] = matrix->elements[ 7];
	matrix3x3[6] = matrix->elements[12];
	matrix3x3[7] = matrix->elements[14];
	matrix3x3[8] = matrix->elements[15];
	result.elements[9] = -matrixDeterminant(matrix3x3);

	/* Element (i, j) (2, 4) */
	matrix3x3[0] = matrix->elements[ 0];
	matrix3x3[1] = matrix->elements[ 2];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 4];
	matrix3x3[4] = matrix->elements[ 6];
	matrix3x3[5] = matrix->elements[ 7];
	matrix3x3[6] = matrix->elements[ 8];
	matrix3x3[7] = matrix->elements[10];
	matrix3x3[8] = matrix->elements[11];
	result.elements[13] = matrixDeterminant(matrix3x3);

	/* Element (i, j) (3, 1) */
	matrix3x3[0] = matrix->elements[ 4];
	matrix3x3[1] = matrix->elements[ 5];
	matrix3x3[2] = matrix->elements[ 7];
	matrix3x3[3] = matrix->elements[ 8];
	matrix3x3[4] = matrix->elements[ 9];
	matrix3x3[5] = matrix->elements[11];
	matrix3x3[6] = matrix->elements[12];
	matrix3x3[7] = matrix->elements[13];
	matrix3x3[8] = matrix->elements[15];
	result.elements[2] = matrixDeterminant(matrix3x3);

	/* Element (i, j) (3, 2) */
	matrix3x3[0] = matrix->elements[ 0];
	matrix3x3[1] = matrix->elements[ 1];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 8];
	matrix3x3[4] = matrix->elements[ 9];
	matrix3x3[5] = matrix->elements[11];
	matrix3x3[6] = matrix->elements[12];
	matrix3x3[7] = matrix->elements[13];
	matrix3x3[8] = matrix->elements[15];
	result.elements[6] = -matrixDeterminant(matrix3x3);

	/* Element (i, j) (3, 3) */
	matrix3x3[0] = matrix->elements[ 0];
	matrix3x3[1] = matrix->elements[ 1];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 4];
	matrix3x3[4] = matrix->elements[ 5];
	matrix3x3[5] = matrix->elements[ 7];
	matrix3x3[6] = matrix->elements[12];
	matrix3x3[7] = matrix->elements[13];
	matrix3x3[8] = matrix->elements[15];
	result.elements[10] = matrixDeterminant(matrix3x3);

	/* Element (i, j) (3, 4) */
	matrix3x3[0] = matrix->elements[ 0];
	matrix3x3[1] = matrix->elements[ 1];
	matrix3x3[2] = matrix->elements[ 3];
	matrix3x3[3] = matrix->elements[ 4];
	matrix3x3[4] = matrix->elements[ 5];
	matrix3x3[5] = matrix->elements[ 7];
	matrix3x3[6] = matrix->elements[ 8];
	matrix3x3[7] = matrix->elements[ 9];
	matrix3x3[8] = matrix->elements[11];
	result.elements[14] = -matrixDeterminant(matrix3x3);

	/* Element (i, j) (4, 1) */
	matrix3x3[0] = matrix->elements[ 4];
	matrix3x3[1] = matrix->elements[ 5];
	matrix3x3[2] = matrix->elements[ 6];
	matrix3x3[3] = matrix->elements[ 8];
	matrix3x3[4] = matrix->elements[ 9];
	matrix3x3[5] = matrix->elements[10];
	matrix3x3[6] = matrix->elements[12];
	matrix3x3[7] = matrix->elements[13];
	matrix3x3[8] = matrix->elements[14];
	result.elements[3] = -matrixDeterminant(matrix3x3);

	/* Element (i, j) (4, 2) */
	matrix3x3[0] = matrix->elements[ 0];
	matrix3x3[1] = matrix->elements[ 1];
	matrix3x3[2] = matrix->elements[ 2];
	matrix3x3[3] = matrix->elements[ 8];
	matrix3x3[4] = matrix->elements[ 9];
	matrix3x3[5] = matrix->elements[10];
	matrix3x3[6] = matrix->elements[12];
	matrix3x3[7] = matrix->elements[13];
	matrix3x3[8] = matrix->elements[14];
	result.elements[7] = matrixDeterminant(matrix3x3);

	/* Element (i, j) (4, 3) */
	matrix3x3[0] = matrix->elements[ 0];
	matrix3x3[1] = matrix->elements[ 1];
	matrix3x3[2] = matrix->elements[ 2];
	matrix3x3[3] = matrix->elements[ 4];
	matrix3x3[4] = matrix->elements[ 5];
	matrix3x3[5] = matrix->elements[ 6];
	matrix3x3[6] = matrix->elements[12];
	matrix3x3[7] = matrix->elements[13];
	matrix3x3[8] = matrix->elements[14];
	result.elements[11] = -matrixDeterminant(matrix3x3);

	/* Element (i, j) (4, 4) */
	matrix3x3[0] = matrix->elements[ 0];
	matrix3x3[1] = matrix->elements[ 1];
	matrix3x3[2] = matrix->elements[ 2];
	matrix3x3[3] = matrix->elements[ 4];
	matrix3x3[4] = matrix->elements[ 5];
	matrix3x3[5] = matrix->elements[ 6];
	matrix3x3[6] = matrix->elements[ 8];
	matrix3x3[7] = matrix->elements[ 9];
	matrix3x3[8] = matrix->elements[10];
	result.elements[15] = matrixDeterminant(matrix3x3);

	/* The adjoint is the transpose of the cofactor matrix. */
	matrixTranspose(&result);

	/* The inverse is the adjoint divided by the determinant. */
	result = matrixScale(&result, 1.0f / matrixDeterminant(matrix));

	return result;
}

Matrix Matrix::matrixScale(Matrix *matrix, float scale)
{
	Matrix result;

	for(int allElements = 0; allElements < 16; allElements ++)
	{
		result.elements[allElements] = matrix->elements[allElements] * scale;
	}

	return result;
}

void Matrix::matrixTranspose(Matrix *matrix)
{
	float temp;

	temp = matrix->elements[1];
	matrix->elements[1] = matrix->elements[4];
	matrix->elements[4] = temp;

	temp = matrix->elements[2];
	matrix->elements[2] = matrix->elements[8];
	matrix->elements[8] = temp;

	temp = matrix->elements[3];
	matrix->elements[3] = matrix->elements[12];
	matrix->elements[12] = temp;

	temp = matrix->elements[6];
	matrix->elements[6] = matrix->elements[9];
	matrix->elements[9] = temp;

	temp = matrix->elements[7];
	matrix->elements[7] = matrix->elements[13];
	matrix->elements[13] = temp;

	temp = matrix->elements[11];
	matrix->elements[11] = matrix->elements[14];
	matrix->elements[14] = temp;
}

Matrix Matrix::createScaling(float x, float y, float z)
{
	Matrix result = identityMatrix;

	result.elements[ 0] = x;
	result.elements[ 5] = y;
	result.elements[10] = z;

	return result;
}

Matrix Matrix::createTranslation(float x, float y, float z)
{
	Matrix result = identityMatrix;

	result.elements[12] = x;
	result.elements[13] = y;
	result.elements[14] = z;

	return result;
}

Matrix Matrix::matrixPerspective(float FOV, float ratio, float zNear, float zFar)
{
	Matrix result = identityMatrix;

	FOV = 1.0f / tan(FOV * 0.5f);

	result.elements[ 0] = FOV / ratio;
	result.elements[ 5] = FOV;
	result.elements[10] = -(zFar + zNear) / (zFar - zNear);
	result.elements[11] = -1.0f;
	result.elements[14] = (-2.0f * zFar * zNear) / (zFar - zNear);
	result.elements[15] = 0.0f;

	return result;
}

Matrix Matrix::matrixCameraLookAt(Vec3f eye, Vec3f center, Vec3f up)
{
	Matrix result = identityMatrix;

	Vec3f cameraX, cameraY;

	Vec3f cameraZ = {center.x - eye.x, center.y - eye.y, center.z - eye.z};
	cameraZ.normalize();

	cameraX = Vec3f::cross(cameraZ, up);
	cameraX.normalize();

	cameraY = Vec3f::cross(cameraX, cameraZ);

	/*
	 * The final cameraLookAt should look like: 
	 *
	 * cameraLookAt[] = { cameraX.x,        cameraY.x,   -cameraZ.x,  0.0f,
	 *                                        cameraX.y,    cameraY.y,   -cameraZ.y,  0.0f,
	 *                                        cameraX.z,    cameraY.z,   -cameraZ.z,  0.0f,
	 *                                       -eye.x,           -eye.y,               -eye.z,          1.0f };
	 */

	result[0]  = cameraX.x;
	result[1]  = cameraY.x;
	result[2]  = -cameraZ.x;

	result[4]  = cameraX.y;
	result[5]  = cameraY.y;
	result[6]  = -cameraZ.y;

	result[8]  = cameraX.z;
	result[9]  = cameraY.z;
	result[10] = -cameraZ.z;

	result[12] = -eye.x;
	result[13] = -eye.y;
	result[14] = -eye.z;

	return result;
}


Matrix Matrix::matrixOrthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Matrix result = identityMatrix;

	result.elements[ 0] = 2.0f / (right - left);
	result.elements[12] = -(right + left) / (right - left);

	result.elements[ 5] = 2.0f / (top - bottom);
	result.elements[13] = -(top + bottom) / (top - bottom);

	result.elements[10] = -2.0f / (zFar - zNear);
	result.elements[14] = -(zFar + zNear) / (zFar - zNear);

	return result;
}

Matrix Matrix::createRotationX(float angle)
{
	Matrix result = identityMatrix;

	result.elements[ 5] = cos(degreesToRadians(angle));
	result.elements[ 9] = -sin(degreesToRadians(angle));
	result.elements[ 6] = sin(degreesToRadians(angle));
	result.elements[10] = cos(degreesToRadians(angle));

	return result;
}

Matrix Matrix::createRotationY(float angle)
{
	Matrix result = identityMatrix;

	result.elements[ 0] = cos(degreesToRadians(angle));
	result.elements[ 8] = sin(degreesToRadians(angle));
	result.elements[ 2] = -sin(degreesToRadians(angle));
	result.elements[10] = cos(degreesToRadians(angle));

	return result;
}

Matrix Matrix::createRotationZ(float angle)
{
	Matrix result = identityMatrix;

	result.elements[0] = cos(degreesToRadians(angle));
	result.elements[4] = -sin(degreesToRadians(angle));
	result.elements[1] = sin(degreesToRadians(angle));
	result.elements[5] = cos(degreesToRadians(angle));

	return result;
}

Matrix Matrix::multiply(Matrix *left, Matrix *right)
{
	Matrix result;

	for(int row = 0; row < 4; row ++)
	{
		for(int column = 0; column < 4; column ++)
		{
			/*result.elements[row * 4 + column]  = left->elements[0 + row * 4] * right->elements[column + 0 * 4];
			  result.elements[row * 4 + column] += left->elements[1 + row * 4] * right->elements[column + 1 * 4];
			  result.elements[row * 4 + column] += left->elements[2 + row * 4] * right->elements[column + 2 * 4];
			  result.elements[row * 4 + column] += left->elements[3 + row * 4] * right->elements[column + 3 * 4];*/
			float accumulator = 0.0f;
			for(int allElements = 0; allElements < 4; allElements ++)
			{
				accumulator += left->elements[allElements * 4 + row] * right->elements[column * 4 + allElements];
			}
			result.elements[column * 4 + row] = accumulator;
		}
	}

	return result;
}

Vec4f Matrix::vertexTransform(Vec4f *vertex, Matrix *matrix)
{
	Vec4f result;

	result.x  = vertex->x * matrix->elements[ 0];
	result.x += vertex->y * matrix->elements[ 4];
	result.x += vertex->z * matrix->elements[ 8];
	result.x += vertex->w * matrix->elements[12];

	result.y  = vertex->x * matrix->elements[ 1];
	result.y += vertex->y * matrix->elements[ 5];
	result.y += vertex->z * matrix->elements[ 9];
	result.y += vertex->w * matrix->elements[13];

	result.z  = vertex->x * matrix->elements[ 2];
	result.z += vertex->y * matrix->elements[ 6];
	result.z += vertex->z * matrix->elements[10];
	result.z += vertex->w * matrix->elements[14];

	result.w  = vertex->x * matrix->elements[ 3];
	result.w += vertex->y * matrix->elements[ 7];
	result.w += vertex->z * matrix->elements[11];
	result.w += vertex->w * matrix->elements[15];

	return result;
}


Vec3f Matrix::vertexTransform(Vec3f *vertex, Matrix *matrix)
{
	Vec3f result;
	Vec4f extendedVertex;

	extendedVertex.x = vertex->x;
	extendedVertex.y = vertex->y;
	extendedVertex.z = vertex->z;
	extendedVertex.w = 1.0f;

	result.x  = extendedVertex.x * matrix->elements[ 0];
	result.x += extendedVertex.y * matrix->elements[ 4];
	result.x += extendedVertex.z * matrix->elements[ 8];
	result.x += extendedVertex.w * matrix->elements[12];

	result.y  = extendedVertex.x * matrix->elements[ 1];
	result.y += extendedVertex.y * matrix->elements[ 5];
	result.y += extendedVertex.z * matrix->elements[ 9];
	result.y += extendedVertex.w * matrix->elements[13];

	result.z  = extendedVertex.x * matrix->elements[ 2];
	result.z += extendedVertex.y * matrix->elements[ 6];
	result.z += extendedVertex.z * matrix->elements[10];
	result.z += extendedVertex.w * matrix->elements[14];

	return result;
}


//Matrix Functions end...

//Text Functions Start
const string Text::textureFilename = "font.raw";
const string Text::vertexShaderFilename = "font.vert";
const string Text::fragmentShaderFilename = "font.frag";

const float Text::scale = 1.0f;

const int Text::textureCharacterWidth = 8;
const int Text::textureCharacterHeight = 16;

Text::Text(const char * resourceDirectory, int windowWidth, int windowHeight)
{
	vertexShaderID = 0;
	fragmentShaderID = 0;
	programID = 0;

	numberOfCharacters = 0;
	textVertex = NULL;
	textTextureCoordinates = NULL;
	color = NULL;
	textIndex = NULL;

	printf("Text initialization started...\n");
	static float angleX = 0.0, angleY = 0.0, angleZ = 0.0;

	/* Create an orthographic projection. */
	projectionMatrix = Matrix::matrixOrthographic(0, (float)windowWidth, 0, (float)windowHeight, 0, 1);
	Matrix scale = Matrix::createScaling(3.0,2.6,0.0);
	projectionMatrix = projectionMatrix * scale;


	/* Update cube's rotation angles for animating. */
	angleX += 3;
	angleY += 2;
	angleZ += 1;

	if(angleX >= 360) angleX -= 360;
	if(angleX < 0) angleX += 360;
	if(angleY >= 360) angleY -= 360;
	if(angleY < 0) angleY += 360;
	if(angleZ >= 360) angleZ -= 360;
	if(angleZ < 0) angleZ += 360;


	/* Shaders. */
	string vertexShader = resourceDirectory + vertexShaderFilename;
	processShader(&vertexShaderID, vertexShader.c_str(), GL_VERTEX_SHADER);

	string fragmentShader = resourceDirectory + fragmentShaderFilename;
	processShader(&fragmentShaderID, fragmentShader.c_str(), GL_FRAGMENT_SHADER);

	/* Set up shaders. */
	programID = GL_CHECK(glCreateProgram());
	GL_CHECK(glAttachShader(programID, vertexShaderID));
	GL_CHECK(glAttachShader(programID, fragmentShaderID));
	GL_CHECK(glLinkProgram(programID));
	GL_CHECK(glUseProgram(programID));

	/* Vertex positions. */
	m_iLocPosition = GL_CHECK(glGetAttribLocation(programID, "a_v4Position"));
	if(m_iLocPosition == -1)
	{
		printf("Attribute not found at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Text colors. */
	m_iLocTextColor = GL_CHECK(glGetAttribLocation(programID, "a_v4FontColor"));
	if(m_iLocTextColor == -1)
	{
		printf("Attribute not found at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* TexCoords. */
	m_iLocTexCoord = GL_CHECK(glGetAttribLocation(programID, "a_v2TexCoord"));
	if(m_iLocTexCoord == -1)
	{
		printf("Attribute not found at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Projection matrix. */
	m_iLocProjection = GL_CHECK(glGetUniformLocation(programID, "u_m4Projection"));
	if(m_iLocProjection == -1)
	{
		printf("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
	}
	else
	{
		GL_CHECK(glUniformMatrix4fv(m_iLocProjection, 1, GL_FALSE, projectionMatrix.getAsArray()));
	}


	/* Set the sampler to point at the 0th texture unit. */
	m_iLocTexture = GL_CHECK(glGetUniformLocation(programID, "u_s2dTexture"));
	if(m_iLocTexture == -1)
	{
		printf("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
	}
	else
	{
		GL_CHECK(glUniform1i(m_iLocTexture, 0));
	}

	/* Load texture. */
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	GL_CHECK(glGenTextures(1, &textureID));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureID));
	/* Set filtering. */
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	string texture = resourceDirectory + textureFilename;
	unsigned char *textureData = NULL;
	loadData(texture.c_str(), &textureData);

	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 48, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData));
	free(textureData);
	textureData = NULL;

	printf("Text initialization done.\n");
}

void Text::clear(void)
{
	numberOfCharacters = 0;

	free(textVertex);
	free(textTextureCoordinates);
	free(color);
	free(textIndex);

	textVertex = NULL;
	textTextureCoordinates = NULL;
	color = NULL;
	textIndex = NULL;
}

void Text::addString(int xPosition, int yPosition, const char *string, int red, int green, int blue, int alpha)
{
	int length = (int)strlen(string);
	int iTexCoordPos = 4 * 2 * numberOfCharacters;
	int iVertexPos = 4 * 3 * numberOfCharacters;
	int iColorPos = 4 * 4 * numberOfCharacters;
	int iIndex = 0;
	int iIndexPos = 0;

	numberOfCharacters += length;

	/* Realloc memory. */
	textVertex = (float *)realloc(textVertex, numberOfCharacters * 4 * 3 * sizeof(float));
	textTextureCoordinates = (float *)realloc(textTextureCoordinates, numberOfCharacters * 4 * 2 * sizeof(float));
	color = (float *)realloc(color, numberOfCharacters * 4 * 4 * sizeof(float));
	textIndex = (GLshort *)realloc(textIndex, (numberOfCharacters * 6 - 2) * sizeof(GLshort));
	if((textVertex == NULL) || (textTextureCoordinates == NULL) || (color == NULL) || (textIndex == NULL))
	{
		printf("Out of memory at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Re-init entire index array. */
	textIndex[iIndex ++] = 0;
	textIndex[iIndex ++] = 1;
	textIndex[iIndex ++] = 2;
	textIndex[iIndex ++] = 3;

	iIndexPos = 4;
	for(int cIndex = 1; cIndex < numberOfCharacters; cIndex ++)
	{
		textIndex[iIndexPos ++] = iIndex - 1;
		textIndex[iIndexPos ++] = iIndex;
		textIndex[iIndexPos ++] = iIndex++;
		textIndex[iIndexPos ++] = iIndex++;
		textIndex[iIndexPos ++] = iIndex++;
		textIndex[iIndexPos ++] = iIndex++;
	}

	for(int iChar = 0; iChar < (signed int)strlen(string); iChar ++)
	{
		char cChar = string[iChar];
		int iCharX = 0;
		int iCharY = 0;
		Vec2 sBottom_left;
		Vec2 sBottom_right;
		Vec2 sTop_left;
		Vec2 sTop_right;

		/* Calculate tex coord for char here. */
		cChar -= 32;
		iCharX = cChar % 32;
		iCharY = cChar / 32;
		iCharX *= textureCharacterWidth;
		iCharY *= textureCharacterHeight;
		sBottom_left.x = iCharX;
		sBottom_left.y = iCharY;
		sBottom_right.x = iCharX + textureCharacterWidth;
		sBottom_right.y = iCharY;
		sTop_left.x = iCharX;
		sTop_left.y = iCharY + textureCharacterHeight;
		sTop_right.x = iCharX + textureCharacterWidth;
		sTop_right.y = iCharY + textureCharacterHeight;

		/* Add vertex position data here. */
		textVertex[iVertexPos++] = xPosition + iChar * textureCharacterWidth * scale;
		textVertex[iVertexPos++] = (float)yPosition;
		textVertex[iVertexPos++] = 0;

		textVertex[iVertexPos++] = xPosition + (iChar + 1) * textureCharacterWidth * scale;
		textVertex[iVertexPos++] = (float)yPosition;
		textVertex[iVertexPos++] = 0;

		textVertex[iVertexPos++] = xPosition + iChar * textureCharacterWidth * scale;
		textVertex[iVertexPos++] = yPosition + textureCharacterHeight * scale;
		textVertex[iVertexPos++] = 0;

		textVertex[iVertexPos++] = xPosition + (iChar + 1) * textureCharacterWidth * scale;
		textVertex[iVertexPos++] = yPosition + textureCharacterHeight * scale;
		textVertex[iVertexPos++] = 0;

		/* Texture coords here. Because textures are read in upside down, flip Y coords here. */
		textTextureCoordinates[iTexCoordPos++] = sBottom_left.x / 256.0f;
		textTextureCoordinates[iTexCoordPos++] = sTop_left.y / 48.0f;

		textTextureCoordinates[iTexCoordPos++] = sBottom_right.x / 256.0f;
		textTextureCoordinates[iTexCoordPos++] = sTop_right.y / 48.0f;

		textTextureCoordinates[iTexCoordPos++] = sTop_left.x / 256.0f;
		textTextureCoordinates[iTexCoordPos++] = sBottom_left.y / 48.0f;

		textTextureCoordinates[iTexCoordPos++] = sTop_right.x / 256.0f;
		textTextureCoordinates[iTexCoordPos++] = sBottom_right.y / 48.0f;

		/* Color data. */
		color[iColorPos ++] = red / 255.0f;
		color[iColorPos ++] = green / 255.0f;
		color[iColorPos ++] = blue / 255.0f;
		color[iColorPos ++] = alpha / 255.0f;

		/* Copy to the other 3 vertices. */
		memcpy(&color[iColorPos], &color[iColorPos - 4], 4 * sizeof(float));
		memcpy(&color[iColorPos + 4], &color[iColorPos], 4 * sizeof(float));
		memcpy(&color[iColorPos + 8], &color[iColorPos + 4], 4 * sizeof(float));
		iColorPos += 3 * 4;
	}
}

void Text::draw(void)
{
	if(numberOfCharacters == 0) 
	{
		return;
	}

	GL_CHECK(glUseProgram(programID));

	if(m_iLocPosition != -1)
	{
		GL_CHECK(glEnableVertexAttribArray(m_iLocPosition));
		GL_CHECK(glVertexAttribPointer(m_iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, textVertex));
	}

	if(m_iLocTextColor != -1)
	{
		GL_CHECK(glEnableVertexAttribArray(m_iLocTextColor));
		GL_CHECK(glVertexAttribPointer(m_iLocTextColor, 4, GL_FLOAT, GL_FALSE, 0, color));
	}

	if(m_iLocTexCoord != -1)
	{
		GL_CHECK(glEnableVertexAttribArray(m_iLocTexCoord));
		GL_CHECK(glVertexAttribPointer(m_iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, textTextureCoordinates));
	}

	if(m_iLocProjection != -1)
	{
		GL_CHECK(glUniformMatrix4fv(m_iLocProjection, 1, GL_FALSE, projectionMatrix.getAsArray()));
	}

	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureID));

	GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, numberOfCharacters * 6 - 2, GL_UNSIGNED_SHORT, textIndex));

	if(m_iLocTextColor != -1)
	{
		GL_CHECK(glDisableVertexAttribArray(m_iLocTextColor));
	}

	if(m_iLocTexCoord != -1)
	{
		GL_CHECK(glDisableVertexAttribArray(m_iLocTexCoord));
	}

	if(m_iLocPosition != -1)
	{
		GL_CHECK(glDisableVertexAttribArray(m_iLocPosition));
	}
}

Text::~Text(void)
{
	clear();

	GL_CHECK(glDeleteTextures(1, &textureID));
}

//Text Functions End



void prepareGraphics (int width,int height)
{
	EGLint major, minor, n;
	GLuint vertex_shader, fragment_shader;
	GLint ret;


	eglDisplay = eglGetDisplay (dpy);

	if (!eglInitialize (eglDisplay, &major, &minor))
	{
		printf ("failed to initialize\n");
	}

	printf ("Display Used %p & EGL versions are %d.%d\n", eglDisplay, major,
			minor);
	printf ("Your EGL Version is \"%s\"\n", eglQueryString (eglDisplay, EGL_VERSION));
	printf ("Your EGL Vendor is \"%s\"\n", eglQueryString (eglDisplay, EGL_VENDOR));

	if (!eglBindAPI (EGL_OPENGL_ES_API))
	{
		printf ("failed to bind api EGL_OPENGL_ES_API\n");
	}

	if (!eglChooseConfig (eglDisplay, config_attribs, &eglConfig, 1, &eglConfigCount))
	{
		printf("Error: couldn't get an EGL visual config\n");
		exit(1);
	}


	EGLint vid;
	assert(eglConfig);
	assert(eglConfigCount > 0);

	if (!eglGetConfigAttrib(eglDisplay,eglConfig, EGL_NATIVE_VISUAL_ID, &vid)) {
		printf("Error: eglGetConfigAttrib() failed\n");
		exit(1);
	}


	eglContext = eglCreateContext (eglDisplay, eglConfig, EGL_NO_CONTEXT, context_attribs);
	if (eglContext == EGL_NO_CONTEXT)
	{
		fprintf (stderr, "\nFailed to create EGL context.\n");
		exit (-1);
	}


	eglSurface = eglCreateWindowSurface (eglDisplay, eglConfig, window, NULL);

	if (eglSurface == EGL_NO_SURFACE)
	{
		printf ("failed to create egl surface\n");
	}

	eglMakeCurrent (eglDisplay, eglSurface, eglSurface, eglContext);

}


bool setupGraphics(int width, int height,int x, int y, const char *string ,int string_no)
{

	GL_CHECK(glEnable(GL_BLEND));
	GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	text[string_no] = new Text(resourceDirectory.c_str(), width, height);
	text[string_no]->addString(x,y, string, 255, 255, 255, 255);
	// text->addString(700,700, "VCU DEMO", 255, 255, 255, 255);

	GL_CHECK(glEnable(GL_CULL_FACE));
	GL_CHECK(glEnable(GL_DEPTH_TEST));
	GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

}
/*bool setupGraphics1(int width, int height)
  {

  GL_CHECK(glEnable(GL_BLEND));
  GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

  text1 = new Text(resourceDirectory.c_str(), width, height);
  text1->addString(800,780, "Transcoding H.264 -> H.265", 255, 255, 255, 255);
// text->addString(700,700, "VCU DEMO", 255, 255, 255, 255);

GL_CHECK(glEnable(GL_CULL_FACE));
GL_CHECK(glEnable(GL_DEPTH_TEST));
GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

}

 */

void runGraphics(int time)
{
	int j;	
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	for (j=0;j<6;j++)
	{
		text[j]->draw();
	}
	//	text->draw();

}


void CleanupEGL(void)
{

	eglBindAPI(EGL_OPENGL_ES_API);
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, eglContext);
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(eglDisplay, eglContext);
	eglDestroySurface(eglDisplay, eglSurface);
	eglTerminate(dpy);


}

	static void
usage(void)
{
	printf("Usage:\n");
	printf("  -display <displayname>  set the display to run on\n");
	printf("  -info                   display OpenGL renderer info\n");
} 
	static void
make_fbdev_window(Display *x_dpy, EGLDisplay egl_dpy,
		const char *name,
		int x, int y, int width, int height,
		EGLContext *ctxRet,
		EGLSurface *surfRet)
{
	const EGLNativeWindowType native_win = (EGLNativeWindowType) NULL;
	static EGLContext egl_ctx;
	static EGLSurface egl_surf;
	EGLint major, minor, num_configs;
	EGLConfig conf;
	EGLint attrib[32], i = 0;
	struct fb_var_screeninfo vinfo;
	int fd, egl_verbose;

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0)
		printf("failed to open");

	memset(&vinfo, 0, sizeof(vinfo));
	if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo))
		printf("failed to get fb info");

	egl_verbose = 1;

	egl_dpy = eglGetDisplay((EGLNativeDisplayType) EGL_DEFAULT_DISPLAY);
	if (egl_dpy == EGL_NO_DISPLAY)
		printf("failed to get a display");
	if (!eglInitialize(egl_dpy, &major, &minor))
		printf("failed to initialize EGL");

	if (egl_verbose) {
		printf("EGL %d.%d\n", major, minor);
		printf("EGL_VENDOR: %s\n", eglQueryString(egl_dpy, EGL_VENDOR));
		printf("EGL_VERSION: %s\n", eglQueryString(egl_dpy, EGL_VERSION));
		printf("EGL_EXTENSIONS: %s\n", eglQueryString(egl_dpy, EGL_EXTENSIONS));
		printf("EGL_CLIENT_APIS: %s\n",
				eglQueryString(egl_dpy, EGL_CLIENT_APIS));
	}
	attrib[i++] = EGL_RENDERABLE_TYPE;
	attrib[i++] = EGL_OPENGL_ES2_BIT;
	attrib[i++] = EGL_BUFFER_SIZE;
	attrib[i++] = vinfo.bits_per_pixel;
	attrib[i++] = EGL_NONE;
	if (!eglChooseConfig(egl_dpy, attrib, &conf, 1, &num_configs) ||
			!num_configs)
		printf("failed to choose a config");

	if (egl_verbose) {
		EGLint tmp;
		eglGetConfigAttrib(egl_dpy, conf, EGL_BUFFER_SIZE, &tmp);
		printf("buffer size: %d\n", tmp);
		eglGetConfigAttrib(egl_dpy, conf, EGL_RED_SIZE, &tmp);
		printf("red size: %d\n", tmp);
		eglGetConfigAttrib(egl_dpy, conf, EGL_GREEN_SIZE, &tmp);
		printf("green size: %d\n", tmp);
		eglGetConfigAttrib(egl_dpy, conf, EGL_BLUE_SIZE, &tmp);
		printf("blue size: %d\n", tmp);
		eglGetConfigAttrib(egl_dpy, conf, EGL_CONFIG_ID, &tmp);
		printf("config id: %d\n", tmp);
	}

	i = 0;
	attrib[i++] = EGL_CONTEXT_CLIENT_VERSION;
	attrib[i++] = 2;
	attrib[i] = EGL_NONE;

	egl_ctx = eglCreateContext(egl_dpy, conf, EGL_NO_CONTEXT, attrib);
	if (egl_ctx == EGL_NO_CONTEXT)
		printf("failed to create a context");

	egl_surf = eglCreateWindowSurface(egl_dpy, conf, native_win, NULL);
	if (egl_surf == EGL_NO_SURFACE)
		printf("failed to create a surface");

	if (!eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx))
		printf("failed to make context/surface current");
	*ctxRet = egl_ctx;
	*surfRet = egl_surf;

}

int main(int argc, char **argv)
{
	int y_offset = 20;
	const char *string[]={"Zynq UltraScale+ MPSoC Video Codec Unit Demonstration",
		"H.265 Streaming Playback",
		"Input Format : H.265/HEVC",
		"Resolution   : UHD 4K (3840x2160)",
		"Frame Rate   : 30 FPS",
		"Video Format : Progressive, 4:2:0, 8 bpc"};
	int xcords=800;
	int ycords[]={800,780,760,740,720,700};
	const char* s2 = "(C)2012 Geeks3D.com";
	Display *x_dpy;
	const char *s;
	EGLSurface egl_surf;
	EGLContext egl_ctx;
	EGLDisplay egl_dpy;
	EGLint egl_major, egl_minor;
	GLboolean printInfo = GL_FALSE;
	bool finish = false;
	long int i=0,j=0;
	bool once = false;


	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-display") == 0) {
			dpyName = argv[i+1];
			i++;
		}
		else if (strcmp(argv[i], "-info") == 0) {
			printf("TRUE\n");
		}
		else {
			usage();
			return -1;
		}
	}
	x_dpy = EGL_DEFAULT_DISPLAY;
	egl_dpy = eglGetDisplay(x_dpy);
	if (!egl_dpy) {
		printf("Error: eglGetDisplay() failed\n");
		return -1;
	}

	if (!eglInitialize(egl_dpy, &egl_major, &egl_minor)) {
		printf("Error: eglInitialize() failed\n");
		return -1;
	}

	s = eglQueryString(egl_dpy, EGL_VERSION);
	printf("EGL_VERSION = %s\n", s);

	s = eglQueryString(egl_dpy, EGL_VENDOR);
	printf("EGL_VENDOR = %s\n", s);

	s = eglQueryString(egl_dpy, EGL_EXTENSIONS);
	printf("EGL_EXTENSIONS = %s\n", s);

	s = eglQueryString(egl_dpy, EGL_CLIENT_APIS);
	printf("EGL_CLIENT_APIS = %s\n", s);

	make_fbdev_window(x_dpy, egl_dpy,
			"OpenGL ES 2.x tri", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
			&egl_ctx, &egl_surf);

	if (!eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx)) {
		printf("Error: eglMakeCurrent() failed\n");
		return -1;
	}

	if (printInfo) {
		printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
		printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
		printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
		printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
	}


	while(!finish)
	{
		//ComputeFrameRate();
		if(!once){
			for (j=0;j<6;j++){
				setupGraphics(WINDOW_WIDTH, WINDOW_HEIGHT,xcords,ycords[j],string[j],j);
			}
			once = true;
		}

		runGraphics(i);

		eglSwapBuffers (egl_dpy, egl_surf);
		if(i<=500)
			i++;
		else
			i=0;

		//i++;
	}

	CleanupEGL();

}


//To Compute Frame Rate
unsigned ComputeTickCount ()
{
	struct timeval tv;
	if (gettimeofday (&tv, NULL) != 0)
		return 0;

	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
/*
   void ComputeFrameRate ()
   {
   static float framesPerSecond = 0.0f;
   static float lastTime = 0.0f;
   float currentTime = ComputeTickCount () * 0.001f;
   ++framesPerSecond;
   if (currentTime - lastTime > 1.0f)
   SPLAY;

   lastTime = currentTime;
   if (SHOW_FPS == 1){
   fprintf (stderr, "\nCurrent Frames Per Second: %d\n\n", (int) framesPerSecond);
   MYFPS = (int) framesPerSecond;
   }

   framesPerSecond = 0;
   }
   }
   SPLAY;
 */
