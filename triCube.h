/*******************************************************************************************************************************************

  Initial Version - 10/27/2016 - Email - alok.gupta@xilinx.com
  Xilinx Inc.

  MULTI  COLOR CUBE APP

 *******************************************************************************************************************************************/


//Include Header Files
/*********************************************************************************************************************************************************/
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <string>
#define GLES_VERSION 2
#include <GLES2/gl2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "unistd.h"
#include <sys/time.h>


using std::string;

//Constants
/*********************************************************************************************************************************************************/
/* Asset directories and filenames. */




//Macros
/*********************************************************************************************************************************************************/
//Earlier ENABLE_THREAD_EXECUTION was defined as EXECUTE_ROTATION , Naming Convention changed
//#define ENABLE_THREAD_EXECUTION
/*********************************************************************************************************************************************************/
int WINDOW_WIDTH = 3840;
int WINDOW_HEIGHT = 2160;
#define SHOW_FPS 1
inline float degreesToRadians(float degrees)
{
	return M_PI * degrees / 180.0f;
}
#define GL_CHECK(x) \
	x; \
{ \
	GLenum glError = glGetError(); \
	if(glError != GL_NO_ERROR) { \
		printf("glGetError() = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__); \
		exit(1); \
	} \
}


//Static/Global Variables
/*********************************************************************************************************************************************************/
int windowWidth = -1;
int windowHeight = -1;
GLint iLocPosition;
GLint iLocColor;
GLint TiLocPosition;
GLint TiLocColor;

GLint iLocMVP;



EGLDisplay eglDisplay;
EGLContext eglContext;
EGLSurface eglSurface;
EGLConfig eglConfig;
EGLint eglConfigCount;


static GLuint program;
static int FPS = 0;
Display *dpy = NULL;
char *dpyName = NULL;
Window window;
string resourceDirectory = "shaders/";


//Static Function
/*********************************************************************************************************************************************************/

static char *loadShader (const char *filename)
{
	FILE *file = fopen (filename, "rb");
	if (file == NULL)
	{
		printf ("Cannot read file '%s'\n", filename);
		exit (1);
	}
	fseek (file, 0, SEEK_END);
	long length = ftell (file);
	fseek (file, 0, SEEK_SET);
	char *shader = (char *) calloc (length + 1, sizeof (char));
	if (shader == NULL)
	{
		printf ("Out of memory at %s:%i\n", __FILE__, __LINE__);
		exit (1);
	}
	size_t numberOfBytesRead = fread (shader, sizeof (char), length, file);
	if (numberOfBytesRead != length)
	{
		printf ("Error reading %s (read %d of %d)", filename,
				numberOfBytesRead, length);
		exit (1);
	}
	shader[length] = '\0';
	fclose (file);

	return shader;
}


static void processShader (GLuint * shader, const char *filename, GLint shaderType)
{
	const char *strings[1] = { NULL };
	*shader = GL_CHECK (glCreateShader (shaderType));
	strings[0] = loadShader (filename);
	GL_CHECK (glShaderSource (*shader, 1, strings, NULL));
	free ((void *) (strings[0]));
	strings[0] = NULL;
	GL_CHECK (glCompileShader (*shader));
	GLint status;
	GL_CHECK (glGetShaderiv (*shader, GL_COMPILE_STATUS, &status));
	if (status != GL_TRUE)
	{
		GLint length;
		char *debugSource = NULL;
		char *errorLog = NULL;

		GL_CHECK (glGetShaderiv (*shader, GL_SHADER_SOURCE_LENGTH, &length));
		debugSource = (char *) malloc (length);
		GL_CHECK (glGetShaderSource (*shader, length, NULL, debugSource));
		printf ("Debug source START:\n%s\nDebug source END\n\n", debugSource);
		free (debugSource);

		GL_CHECK (glGetShaderiv (*shader, GL_INFO_LOG_LENGTH, &length));
		errorLog = (char *) malloc (length);
		GL_CHECK (glGetShaderInfoLog (*shader, length, NULL, errorLog));
		printf ("Log START:\n%s\nLog END\n\n", errorLog);
		free (errorLog);

		printf ("Compilation FAILED!\n\n");
		exit (1);
	}
}


static void loadData(const char *filename, unsigned char **textureData)

{

	FILE *file = fopen(filename, "rb");
        if(file == NULL)
        {
            printf("Failed to open '%s'\n", filename);
            exit(1);
        }
        fseek(file, 0, SEEK_END);
        unsigned int length = ftell(file);
        unsigned char *loadedTexture = (unsigned char *)calloc(length, sizeof(unsigned char));
        if(loadedTexture == NULL)
        {
            printf("Out of memory at %s:%i\n", __FILE__, __LINE__);
            exit(1);
        }
        fseek(file, 0, SEEK_SET);
        size_t read = fread(loadedTexture, sizeof(unsigned char), length, file);
        if(read != length)
        {
            printf("Failed to read in '%s'\n", filename);
            exit(1);
        }
        fclose(file);

        *textureData = loadedTexture;



}

//Structures
/*********************************************************************************************************************************************************/

struct CubesAndPlaneProgramProperties
{
	GLuint programId;
	GLuint iLocPosition;
	GLuint iLocColor;
	GLuint TiLocPosition;
        GLuint TiLocColor;
	GLuint iLocMVP;

} cubesAndPlaneProgram;

static const EGLint context_attribs[] = {
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
};

static const EGLint config_attribs[] = {
	EGL_RED_SIZE, 1,
	EGL_GREEN_SIZE, 1,
	EGL_BLUE_SIZE, 1,
	EGL_DEPTH_SIZE, 1,
	EGL_RENDERABLE_TYPE, 
	EGL_OPENGL_ES2_BIT,
	EGL_NONE
};


class Vec2
{
	public:
		int x, y;
};

class Vec3
{
	public:
		int x, y, z;
};

class Vec4
{
	public:
		int x, y, z, w;
};

class Vec2f
{
	public:
		float x, y;
};

class Vec3f
{
	public:
		float x, y, z;

		void normalize(void)
		{
			float length = sqrt(x * x + y * y + z * z);

			x /= length;
			y /= length;
			z /= length;
		}

		static Vec3f cross(const Vec3f& vector1, const Vec3f& vector2)
		{
			/* Floating point vector to be returned. */
			Vec3f crossProduct;

			crossProduct.x = (vector1.y * vector2.z) - (vector1.z * vector2.y);
			crossProduct.y = (vector1.z * vector2.x) - (vector1.x * vector2.z);
			crossProduct.z = (vector1.x * vector2.y) - (vector1.y * vector2.x);

			return crossProduct;
		}

		static float dot(Vec3f& vector1, Vec3f& vector2)
		{
			return (vector1.x * vector2.x + vector1.y * vector2.y + vector1.z * vector2.z);
		}
};

class Vec4f
{
	public:
		float x, y, z, w;

		void normalize(void)
		{
			float length = sqrt(x * x + y * y + z * z + w * w);

			x /= length;
			y /= length;
			z /= length;
			w /= length;
		}
};



class Matrix
{
	private:
		float elements[16];
		static Matrix multiply(Matrix *left, Matrix *right);
		static const float identityArray[];
	public:
		float* getAsArray(void);
		Matrix(void);
		float& operator[] (unsigned element);
		Matrix operator* (Matrix right);
		Matrix& operator=(const Matrix &another);
		Matrix(const float* array);
		static Matrix identityMatrix;
		static Vec4f vertexTransform(Vec4f *vector, Matrix *matrix);
		static Vec3f vertexTransform(Vec3f *vector, Matrix *matrix);
		static void matrixTranspose(Matrix *matrix);
		static Matrix createRotationX(float angle);
		static Matrix createRotationY(float angle);
		static Matrix createRotationZ(float angle);
		static Matrix createTranslation(float x, float y, float z);
		static Matrix createScaling(float x, float y, float z);
		void print(void);
		static Matrix matrixPerspective(float FOV, float ratio, float zNear, float zFar);
		static Matrix matrixCameraLookAt(Vec3f eye, Vec3f center, Vec3f up);
		static Matrix matrixOrthographic(float left, float right, float bottom, float top, float zNear, float zFar);
		static Matrix matrixInvert(Matrix *matrix);
		static float matrixDeterminant(float *matrix);
		static float matrixDeterminant(Matrix *matrix);
		static Matrix matrixScale(Matrix *matrix, float scale);

};



class Text
    {
    private:
        static const std::string textureFilename;
        static const std::string vertexShaderFilename;
        static const std::string fragmentShaderFilename;
        static const float scale;


        Matrix projectionMatrix;
        int numberOfCharacters;
        float *textVertex;
        float *textTextureCoordinates;
        float *color;
        GLshort *textIndex;
        int m_iLocPosition;
        int m_iLocProjection;
        int m_iLocTextColor;
        int m_iLocTexCoord;
        int m_iLocTexture;
        GLuint vertexShaderID;
        GLuint fragmentShaderID;
        GLuint programID;
        GLuint textureID;

    public:

        static const int textureCharacterWidth;
        static const int textureCharacterHeight;
        Text(const char * resourceDirectory, int windowWidth, int windowHeight);
        virtual ~Text(void);
        void clear(void);
        void addString(int xPosition, int yPosition, const char *string, int red, int green, int blue, int alpha);
        void draw(void);
    };




//Function Declrations
/*********************************************************************************************************************************************************/
void prepareGraphics (int WINDOW_WIDTH,int WINDOW_HEIGHT);
void doGraphics(void);
bool setupGraphics(int width, int height);
void runGraphics(int time);
void CleanupEGL(void);
void ComputeFrameRate (void);
unsigned ComputeTickCount ();
bool setupPrograms ();
bool setupCubesAndPlaneProgram (void);
void setUpAndUseProgramObject (GLint programId, string fragmentShaderFileName, string vertexShaderFileName);




//Primitives Data (Vertex,texture,Color,Normal)
/*********************************************************************************************************************************************************/
/* 3D data. Vertex range -1.0..1.0 in all axes.
 * Z -1.0 is near, 1.0 is far. */

/* 3D data. Vertex range -1.0..1.0 in all axes.
* Z -1.0 is near, 1.0 is far. */
const float vertices[] =
{
    /* Front face. */
    /* Bottom left */
    -1.0,  1.0, -1.0,
    1.0, -1.0, -1.0,
    -1.0, -1.0, -1.0,
    /* Top right */
    -1.0,  1.0, -1.0,
    1.0,  1.0, -1.0,
    1.0, -1.0, -1.0,
    /* Left face */
    /* Bottom left */
    -1.0,  1.0,  1.0,
    -1.0, -1.0, -1.0,
    -1.0, -1.0,  1.0,
    /* Top right */
    -1.0,  1.0,  1.0,
    -1.0,  1.0, -1.0,
    -1.0, -1.0, -1.0,
    /* Top face */
    /* Bottom left */
    -1.0,  1.0,  1.0,
    1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0,
    /* Top right */
    -1.0,  1.0,  1.0,
    1.0,  1.0,  1.0,
    1.0,  1.0, -1.0,
    /* Right face */
    /* Bottom left */
    1.0,  1.0, -1.0,
    1.0, -1.0,  1.0,
    1.0, -1.0, -1.0,
    /* Top right */
    1.0,  1.0, -1.0,
    1.0,  1.0,  1.0,
    1.0, -1.0,  1.0,
    /* Back face */
    /* Bottom left */
    1.0,  1.0,  1.0,
    -1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    /* Top right */
    1.0,  1.0,  1.0,
    -1.0,  1.0,  1.0,
    -1.0, -1.0,  1.0,
    /* Bottom face */
    /* Bottom left */
    -1.0, -1.0, -1.0,
    1.0, -1.0,  1.0,
    -1.0, -1.0,  1.0,
    /* Top right */
    -1.0, -1.0, -1.0,
    1.0, -1.0, -1.0,
    1.0, -1.0,  1.0,
};

const float colors[] =
{
    /* Front face */
    /* Bottom left */
    1.0, 0.0, 0.0, /* red */
    0.0, 0.0, 1.0, /* blue */
    0.0, 1.0, 0.0, /* green */
    /* Top right */
    1.0, 0.0, 0.0, /* red */
    1.0, 1.0, 0.0, /* yellow */
    0.0, 0.0, 1.0, /* blue */
    /* Left face */
    /* Bottom left */
    1.0, 1.0, 1.0, /* white */
    0.0, 1.0, 0.0, /* green */
    0.0, 1.0, 1.0, /* cyan */
    /* Top right */
    1.0, 1.0, 1.0, /* white */
    1.0, 0.0, 0.0, /* red */
    0.0, 1.0, 0.0, /* green */
    /* Top face */
    /* Bottom left */
    1.0, 1.0, 1.0, /* white */
    1.0, 1.0, 0.0, /* yellow */
    1.0, 0.0, 0.0, /* red */
    /* Top right */
    1.0, 1.0, 1.0, /* white */
    0.0, 0.0, 0.0, /* black */
    1.0, 1.0, 0.0, /* yellow */
    /* Right face */
    /* Bottom left */
    1.0, 1.0, 0.0, /* yellow */
    1.0, 0.0, 1.0, /* magenta */
    0.0, 0.0, 1.0, /* blue */
    /* Top right */
    1.0, 1.0, 0.0, /* yellow */
    0.0, 0.0, 0.0, /* black */
    1.0, 0.0, 1.0, /* magenta */
    /* Back face */
    /* Bottom left */
    0.0, 0.0, 0.0, /* black */
    0.0, 1.0, 1.0, /* cyan */
    1.0, 0.0, 1.0, /* magenta */
    /* Top right */
    0.0, 0.0, 0.0, /* black */
    1.0, 1.0, 1.0, /* white */
    0.0, 1.0, 1.0, /* cyan */
    /* Bottom face */
    /* Bottom left */
    0.0, 1.0, 0.0, /* green */
    1.0, 0.0, 1.0, /* magenta */
    0.0, 1.0, 1.0, /* cyan */
    /* Top right */
    0.0, 1.0, 0.0, /* green */
    0.0, 0.0, 1.0, /* blue */
    1.0, 0.0, 1.0, /* magenta */
};
