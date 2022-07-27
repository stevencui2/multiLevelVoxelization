/**
 * MIT License
 *
 * Copyright(c) 2018 Iowa State University (ISU) and ISU IDEALab
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#define EGL_EGLEXT_PROTOTYPES
#include "../includes/Object.h"
#include "../includes/GPUUtilities.h"
#include "../includes/CUDAUtilities.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

float modelSize = 3;
Float3 modelPos = Float3(0, 0, 0);

// CG and GL variables
GLParameters *glParam;

// Entities Global Variable
vector<Object *> objects;

void ReadOBJFile(char *fName, int dlID, double deltaX, double deltaY, double deltaZ)
{
	Object *tempObject = new Object();
	bool randomObjColor = false;

	Face *face = new Face();
	face->dlid = dlID;
	face->trimmed = false;
	tempObject->objID = dlID - 3;

	Float3 tempFaceColor = Float3(0.768628, 0.462745, 0.137255);
	if (randomObjColor)
		tempFaceColor = Float3(rand() / (RAND_MAX * 1.0), rand() / (RAND_MAX * 1.0), rand() / (RAND_MAX * 1.0));
	face->kdColor = Float3(tempFaceColor[0], tempFaceColor[1], tempFaceColor[2]);
	face->ksColor = Float3(tempFaceColor[0] * 0.25, tempFaceColor[1] * 0.25, tempFaceColor[2] * 0.25);

	face->ka = 0.11;
	face->shininess = 50;
	face->surfID = 0;
	tempObject->faces.push_back(face);
	tempObject->ReadObject(fName);
	tempObject->deltaX = deltaX;
	tempObject->deltaY = deltaY;
	tempObject->deltaZ = deltaZ;

	float currentModelSize = Distance(tempObject->bBoxMax, tempObject->bBoxMin) / 8.0;
	modelSize = currentModelSize;
	Float3 currentModelPos = Float3(-(tempObject->bBoxMin[0] + tempObject->bBoxMax[0]), -(tempObject->bBoxMin[1] + tempObject->bBoxMax[1]), -(tempObject->bBoxMin[2] + tempObject->bBoxMax[2]));
	modelPos = (modelPos + currentModelPos);
	modelPos = modelPos / 2.0;

	objects.push_back(tempObject);
}

void ReadOFFFile(char *fName, int dlID)
{
	Object *tempObject = new Object();
	bool randomObjColor = false;

	Face *face = new Face();
	face->dlid = dlID;
	face->trimmed = false;

	Float3 tempFaceColor = Float3(0.768628, 0.462745, 0.137255);
	if (randomObjColor)
		tempFaceColor = Float3(rand() / (RAND_MAX * 1.0), rand() / (RAND_MAX * 1.0), rand() / (RAND_MAX * 1.0));
	face->kdColor = Float3(tempFaceColor[0], tempFaceColor[1], tempFaceColor[2]);
	face->ksColor = Float3(tempFaceColor[0] * 0.25, tempFaceColor[1] * 0.25, tempFaceColor[2] * 0.25);

	face->ka = 0.11;
	face->shininess = 50;
	face->surfID = 0;
	tempObject->faces.push_back(face);
	tempObject->ReadOFFObject(fName);
	float currentModelSize = VectorMagnitude(tempObject->bBoxMax - tempObject->bBoxMin) / 8;
	if (currentModelSize > modelSize)
		modelSize = currentModelSize;
	Float3 currentModelPos = Float3(-(tempObject->bBoxMin[0] + tempObject->bBoxMax[0]), -(tempObject->bBoxMin[1] + tempObject->bBoxMax[1]), -(tempObject->bBoxMin[2] + tempObject->bBoxMax[2]));
	modelPos = (modelPos + currentModelPos);
	modelPos = modelPos / 2.0;

	objects.push_back(tempObject);
}

void ReadRAWFile(char *fName, int dlID)
{
	Object *tempObject = new Object();
	tempObject->bBoxMax = Float3(3.990815, 52.696899, -8.802678);
	tempObject->bBoxMin = Float3(-5.166523, 40.273825, -21.6738);
	tempObject->ReadRAWObject(fName);
	tempObject->objID = dlID - 3;

	float currentModelSize = VectorMagnitude(tempObject->bBoxMax - tempObject->bBoxMin) / 8;
	if (currentModelSize > modelSize)
		modelSize = currentModelSize;
	Float3 currentModelPos = Float3(-(tempObject->bBoxMin[0] + tempObject->bBoxMax[0]), -(tempObject->bBoxMin[1] + tempObject->bBoxMax[1]), -(tempObject->bBoxMin[2] + tempObject->bBoxMax[2]));
	modelPos = (modelPos + currentModelPos);
	modelPos = modelPos / 2.0;
	modelPos = currentModelPos;

	objects.push_back(tempObject);
}

void InitGLEW(void)
{
	// Initialize GLEW
	GLenum ret = glewInit();
	if (ret != GLEW_OK)
	{
		// Problem: glewInit failed, something is seriously wrong.
		fprintf(stderr, "Error: %s\n", glewGetErrorString(ret));
	}
	if (!GLEW_EXT_framebuffer_object)
	{
		fprintf(stderr, "EXT_framebuffer_object is not supported!\n\n");
		// exit(EXIT_FAILURE);
	}
	else if (!GLEW_ARB_occlusion_query)
	{
		fprintf(stderr, "Occlusion Query is not supported!\n\n");
		// exit(EXIT_FAILURE);
	}
}

void CreateFlatTriangleData()
{
	for (int objectID = 0; objectID < objects.size(); objectID++)
		objects[objectID]->CreateFlatTriangleData();

	cout << "Stored object triangle data in flat data structures" << endl;
}

// Function to calculate length of given string in a command line argument
int CommandLineArgLength(char *string)
{
	int i;
	for (i = 0; string[i] != '\0'; i++)
		;
	return i;
}
static const EGLint configAttribs[] = {
	EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
	EGL_BLUE_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_RED_SIZE, 8,
	EGL_DEPTH_SIZE, 8,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	EGL_NONE};

static const int pbufferWidth = 9;
static const int pbufferHeight = 9;

static const EGLint pbufferAttribs[] = {
	EGL_WIDTH,
	pbufferWidth,
	EGL_HEIGHT,
	pbufferHeight,
	EGL_NONE,
};
int main(int argc, char *argv[])
{
	static const int MAX_DEVICES = 4;
	EGLDeviceEXT eglDevs[MAX_DEVICES];
	EGLint numDevices;

	PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT =
		(PFNEGLQUERYDEVICESEXTPROC)
			eglGetProcAddress("eglQueryDevicesEXT");

	eglQueryDevicesEXT(MAX_DEVICES, eglDevs, &numDevices);

	printf("Detected % d devices\n", numDevices);

	PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
		(PFNEGLGETPLATFORMDISPLAYEXTPROC)
			eglGetProcAddress("eglGetPlatformDisplayEXT");

	EGLDisplay eglDpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT,
												 eglDevs[0], 0);

	EGLint major, minor;

	eglInitialize(eglDpy, &major, &minor);

	// Initialize Variables
	glParam = new GLParameters();

	// 2. Select an appropriate configuration
	EGLint numConfigs;
	EGLConfig eglCfg;

	eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);

	// 3. Create a surface
	EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg,
												 pbufferAttribs);

	// 4. Bind the API
	eglBindAPI(EGL_OPENGL_API);

	// 5. Create a context and make it current
	EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT,
										 NULL);

	eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);

	InitGLEW();

	if (argv[1] == NULL)
		cout << "File not specified!" << endl;
	for (int i = 1; i < argc; i++)
	{
		if (argv[i] != NULL)
		{
			std::size_t fileIndex = i;
			char *currentArg = argv[i++];
			double deltaX, deltaY, deltaZ;
			if (argv[i++] == std::string("deltaX"))
			{
				char *deltaXArg = argv[i++];
				deltaX = atof(deltaXArg);
			}
			else
			{
				std::cerr << "please specify delta x.\n";
			}
			if (argv[i++] == std::string("deltaY"))
			{
				char *deltaYArg = argv[i++];
				deltaY = atof(deltaYArg);
			}
			else
			{
				std::cerr << "please specify delta y.\n";
			}
			if (argv[i++] == std::string("deltaZ"))
			{
				char *deltaZArg = argv[i];
				deltaZ = atof(deltaZArg);
			}
			else
			{
				std::cerr << "please specify delta z.\n";
			}
			int argLen = CommandLineArgLength(currentArg);
			char *fileType[3];
			strncpy((char *)fileType, (const char *)(currentArg + argLen - 3), sizeof("off"));
			if (strcmp((const char *)fileType, "OBJ") == 0 || strcmp((const char *)fileType, "obj") == 0)
				ReadOBJFile(argv[fileIndex], i / 4 + 2, deltaX, deltaY, deltaZ);
			// else if (strcmp((const char *)fileType, "OFF") == 0 || strcmp((const char *)fileType, "off") == 0)
			// 	ReadOFFFile(argv[i], i + 1);
			// else if (strcmp((const char *)fileType, "RAW") == 0 || strcmp((const char *)fileType, "raw") == 0)
			// 	ReadRAWFile(argv[i], i + 1);
			else
				cout << "Unknown file type : " << (const char *)fileType << endl;
		}
	}
	// Create Flat Triangle Data Structure
	CreateFlatTriangleData();

	// Initialize CUDA
#ifdef USECUDA
	InitializeCUDA();
#endif
	// PerformBooleanOperation();
	// glParam->voxelCount = 100;
	for (int i = 0; i < objects.size(); i++)
		if (objects[i]->voxelData == NULL)
			objects[i]->PerformVoxelization(glParam, -1);

	// 6. Terminate EGL when finished
	eglTerminate(eglDpy);

	delete glParam;
}
