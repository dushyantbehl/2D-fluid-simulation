#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

using namespace std;

//---------------
#include "defines.h"
#include "RenderTexture.h"

#include "debug.h"

#define screen_width DIM
#define screen_height DIM

struct Program
{
	CGcontext cgContext;
	CGprofile fragmentProfile;
	CGprogram fragmentProgram;
    
} *adVel, *velBoundary, *diffuseVel, *divergenceProg, *compPressure, *presBoundary, *subPres, *adCol, *compVorticity, *applyVorticity, *acc;

bool startUpdate = false ;

float velocity[screen_height][screen_width][2];

float divergence[screen_height][screen_width];

float pressure[screen_height][screen_width];

float dye[screen_height][screen_width][4]; //Dye RGBA components

float vorticity[screen_height][screen_width];

GLuint velTex,divTex,pressureTex,colTex,vorTex,texOut;
GLuint fb;
CGparameter xParam, uParam, rdx, timestep;

CGcontext cgContext;
CGprofile fragmentProfile;
CGprogram fragmentProgram;

RenderTexture *rt1,*rt2,*rt3;

int startX=-1,startY,endX,endY;
int mouseColor = 1 ;
bool mouseEnable = true ; //True implies injecting dye on Left Mouse click 
bool mouseEnable2 = true ; //True implies adding force impulse on Right Mouse click 

inline float abs(float x)
{
	if(x<0) return -x;
	else return x;
}

//Function Declarations
void initCG();
void genProfiles();
void genTex();
void swapVelocities();
void swapColors();
void advectVelocity();
void advectColor();
void diffuseVelocity();
void computeDivergence();
void computePressure();
void computeVorticity();
void subtractPressureGrad();
void applyVelocityBoundary();
void applyPressureBoundary();
void applyVortex();
void dispFun();
void update();
void handleKey(unsigned char key, int x, int y );
void handleMouse(int button, int state, int x, int y) ;
void mouseDrag(int x,int y);
void InjectDye(int x, int y);
void randomize();
void randomizeVel();
void cgErrorCallback();

int main( int argc, char **argv)
{
    cgSetErrorCallback(cgErrorCallback);
	glutInit(&argc,argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (screen_width, screen_height); 
    glutInitWindowPosition (100, 100);
	glutCreateWindow("Fluid Dynamics");
	
    // First initialize extensions.
    int err = glewInit();
    if (GLEW_OK != err)
    {
    fprintf(stderr, "Error glew initialization failes: %s\n", glewGetErrorString(err));
    exit(-1);
    }
    
	genProfiles();
	genTex();
	glClearColor(0,0,0,1.0);

	rt1=new RenderTexture(screen_width,screen_height,GL_RG,GL_RG32F);
	rt2=new RenderTexture(screen_width,screen_height,GL_RED,GL_R32F);
	rt3=new RenderTexture(screen_width,screen_height,GL_RGBA,GL_RGBA);

    if(debug)
    openLOG();
    
    printDetails();
	initCG();
	glutDisplayFunc(dispFun);
	glutIdleFunc(NULL);
	glutKeyboardFunc(handleKey);
    glutMouseFunc(handleMouse);
    glutMotionFunc(NULL);
	glutMainLoop();
    glutReshapeFunc(NULL);
    
	return EXIT_SUCCESS;
}


void initCG()
{
	adVel= new Program;

	adVel->cgContext = cgCreateContext();
	adVel->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(adVel->fragmentProfile);
	adVel->fragmentProgram=cgCreateProgramFromFile( adVel->cgContext,CG_SOURCE,"pShader.cg", adVel->fragmentProfile,"advect",NULL );
	

	velBoundary = new Program;
	velBoundary->cgContext = cgCreateContext();
	velBoundary->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(velBoundary->fragmentProfile);
	velBoundary->fragmentProgram=cgCreateProgramFromFile( velBoundary->cgContext,CG_SOURCE,"pShader.cg", velBoundary->fragmentProfile,"boundary",NULL );
	cgGLLoadProgram (velBoundary->fragmentProgram);

	diffuseVel= new Program;
	diffuseVel->cgContext = cgCreateContext();
	diffuseVel->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(diffuseVel->fragmentProfile);
	diffuseVel->fragmentProgram=cgCreateProgramFromFile( diffuseVel->cgContext,CG_SOURCE,"pShader.cg", diffuseVel->fragmentProfile,"jacobi",NULL );
	cgGLLoadProgram (diffuseVel->fragmentProgram);


	divergenceProg= new Program;
	divergenceProg->cgContext = cgCreateContext();
	divergenceProg->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(divergenceProg->fragmentProfile);
	divergenceProg->fragmentProgram=cgCreateProgramFromFile( divergenceProg->cgContext,CG_SOURCE,"pShader.cg", divergenceProg->fragmentProfile,"divergence",NULL );
	cgGLLoadProgram (divergenceProg->fragmentProgram);

	compPressure= new Program;
	compPressure->cgContext = cgCreateContext();
	compPressure->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(compPressure->fragmentProfile);
	compPressure->fragmentProgram=cgCreateProgramFromFile( compPressure->cgContext,CG_SOURCE,"pShader.cg", compPressure->fragmentProfile,"jacobi",NULL );
	cgGLLoadProgram (compPressure->fragmentProgram);

	presBoundary= new Program;
	presBoundary->cgContext = cgCreateContext();
	presBoundary->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(presBoundary->fragmentProfile);
	presBoundary->fragmentProgram=cgCreateProgramFromFile( presBoundary->cgContext,CG_SOURCE,"pShader.cg", presBoundary->fragmentProfile,"boundary",NULL );
	cgGLLoadProgram (presBoundary->fragmentProgram);


	subPres= new Program;
	subPres->cgContext = cgCreateContext();
	subPres->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(subPres->fragmentProfile);
	subPres->fragmentProgram=cgCreateProgramFromFile( subPres->cgContext,CG_SOURCE,"pShader.cg", subPres->fragmentProfile,"gradient",NULL );
	cgGLLoadProgram (subPres->fragmentProgram);


	adCol= new Program;
	adCol->cgContext = cgCreateContext();
	adCol->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(adCol->fragmentProfile);
	adCol->fragmentProgram=cgCreateProgramFromFile( adCol->cgContext,CG_SOURCE,"pShader.cg", adCol->fragmentProfile,"advect",NULL );
	cgGLLoadProgram (adCol->fragmentProgram);	


	compVorticity= new Program;
	compVorticity->cgContext = cgCreateContext();
	compVorticity->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(compVorticity->fragmentProfile);
	compVorticity->fragmentProgram=cgCreateProgramFromFile( compVorticity->cgContext,CG_SOURCE,"pShader.cg", compVorticity->fragmentProfile,"compvorticity",NULL );
	cgGLLoadProgram (compVorticity->fragmentProgram);	


	applyVorticity= new Program;
	applyVorticity->cgContext = cgCreateContext();
	applyVorticity->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(applyVorticity->fragmentProfile);
	applyVorticity->fragmentProgram=cgCreateProgramFromFile( applyVorticity->cgContext,CG_SOURCE,"pShader.cg", applyVorticity->fragmentProfile,"applyvorticity",NULL );
	cgGLLoadProgram (applyVorticity->fragmentProgram);	

	acc= new Program;
	acc->cgContext = cgCreateContext();
	acc->fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(acc->fragmentProfile);
	acc->fragmentProgram=cgCreateProgramFromFile( acc->cgContext,CG_SOURCE,"pShader.cg", acc->fragmentProfile,"accelerate",NULL );
	cgGLLoadProgram (acc->fragmentProgram);
    
    return ;

}

void cgErrorCallback()
{
  CGerror lastError = cgGetError();
  
  if(lastError)
  {
    printf("%s\n\n", cgGetErrorString(lastError));
    //printf("Cg error, exiting...\n");
   // exit(0);
  }
} 

void genProfiles()
{
		int i,j;
	for (i=0;i<screen_height;i++)
	{
		for (j=0;j<screen_width;j++)
		{
			
			if(i%3==0 && j%3==0)
			{
			velocity[i][j][0]=0.5;
			velocity[i][j][1]=1;
			}
			else if(i%3==0 && j%3==1)
			{
			velocity[i][j][0]=0.5;
			velocity[i][j][1]=0.6;
			}
			else if(i%3==0 && j%3==2)
			{
			velocity[i][j][0]=0.4;
			velocity[i][j][1]=0.3;
			}
			else if(i%3==1 && j%3==0)
			{
			velocity[i][j][0]=0;
			velocity[i][j][1]=0.5;
			}
			else if(i%3==1 && j%3==1)
			{
			velocity[i][j][0]=0.7;
			velocity[i][j][1]=0.3;
			}
			else if(i%3==1 && j%3==2)
			{
			velocity[i][j][0]=0.2;
			velocity[i][j][1]=0.5;
			}
			else if(i%3==2 && j%3==0)
			{
			velocity[i][j][0]=0;
			velocity[i][j][1]=0;
			}
			else if(i%3==2 && j%3==1)
			{
			velocity[i][j][0]=0.5;
			velocity[i][j][1]=0.5;
			}
			else
			{
			velocity[i][j][0]=1;
			velocity[i][j][1]=0.5;
			}

			pressure[i][j]=1;

			dye[i][j][0]=0.0; //RED
			dye[i][j][1]=0.0; //GREEN
			dye[i][j][2]=0.0; //BLUE
			dye[i][j][3]=1.0; //ALPHA
		}
	}

//	for (i=1;i<30;i++)
//		for (j=10;j<60;j++)
//  			dye[i][j][2]=0.6;

	for (i=1;i<30;i++)
		for (j=50;j<180;j++)
   			dye[i][j][0]=0.6;

//	for (i=1;i<30;i++)
//		for (j=180;j<230;j++)
//  			dye[i][j][0]=0.8;

    pressure[1][1]=1;

    return ;
}

void genTex()
{
	glGenTextures (1, &vorTex);
	glBindTexture(GL_TEXTURE_RECTANGLE,vorTex);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, screen_width, screen_height, 0, GL_RED, GL_FLOAT, 0);

	glGenTextures (1, &divTex);
	glBindTexture(GL_TEXTURE_RECTANGLE,divTex);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, screen_width, screen_height, 0, GL_RED, GL_FLOAT, 0);

	glGenTextures (1, &velTex);
	glBindTexture(GL_TEXTURE_RECTANGLE,velTex);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RG32F, screen_width, screen_height, 0, GL_RG, GL_FLOAT, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, velTex);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE,0,0,0,screen_width,screen_height,GL_RG,GL_FLOAT,velocity);

	glGenTextures (1, &pressureTex);
	glBindTexture(GL_TEXTURE_RECTANGLE,pressureTex);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RED, screen_width, screen_height, 0, GL_RED, GL_FLOAT, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, pressureTex);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE,0,0,0,screen_width,screen_height,GL_RED,GL_FLOAT,pressure);

	glGenTextures (1, &colTex);
	glBindTexture(GL_TEXTURE_RECTANGLE,colTex);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, colTex);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE,0,0,0,screen_width,screen_height,GL_RGBA,GL_FLOAT,dye);

	return ;
}

void swapVelocities()
{
	glBindTexture(GL_TEXTURE_RECTANGLE, velTex);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE,0,0,0,screen_width,screen_height,GL_RG,GL_FLOAT,velocity);
    return ;
}

void swapColors()
{
	glBindTexture(GL_TEXTURE_RECTANGLE, colTex);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE,0,0,0,screen_width,screen_height,GL_RGBA,GL_FLOAT,dye);
    return ;
}

void advectVelocity()
{
	cgGLLoadProgram (adVel->fragmentProgram);
	cgGLEnableProfile(adVel->fragmentProfile);

	cgGLBindProgram(adVel->fragmentProgram);

	uParam = cgGetNamedParameter (adVel->fragmentProgram,"u");
	xParam = cgGetNamedParameter (adVel->fragmentProgram,"x");
	timestep= cgGetNamedParameter (adVel->fragmentProgram,"timestep");
	rdx= cgGetNamedParameter (adVel->fragmentProgram,"rdx");
		
	cgGLSetTextureParameter(uParam, velTex);
	cgGLEnableTextureParameter(uParam);
	cgGLSetTextureParameter(xParam, velTex);
	cgGLEnableTextureParameter(xParam);
	cgGLSetParameter1f(timestep,dt);
	cgGLSetParameter1f(rdx,1.0f/dx);

	rt1->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt1->finishRender();
	rt1->bind();

	texOut=rt1->textureID();
	
	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RG,GL_FLOAT,velocity);
	swapVelocities();

    return ;
}

void advectColor()
{
	
	CGparameter u = cgGetNamedParameter (adCol->fragmentProgram,"u");
	CGparameter rdx= cgGetNamedParameter (adCol->fragmentProgram,"rdx");
	CGparameter color = cgGetNamedParameter (adCol->fragmentProgram,"x");
	CGparameter timestep= cgGetNamedParameter (adCol->fragmentProgram,"timestep");

	cgGLEnableProfile(adCol->fragmentProfile);

	cgGLBindProgram(adCol->fragmentProgram);

	cgGLSetTextureParameter(u,velTex);
	cgGLEnableTextureParameter(u);
	
	cgGLSetTextureParameter(color, colTex);
	cgGLEnableTextureParameter(color);

	cgGLSetParameter1f(timestep,dt);

	cgGLSetParameter1f(rdx,1.0f/dx);

	rt3->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt3->finishRender();
	rt3->bind();

	texOut=rt3->textureID();

	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RGBA,GL_FLOAT,dye);
	swapColors();

    
    return ;
}

void diffuseVelocity()
{
	
	CGparameter alpha = cgGetNamedParameter (diffuseVel->fragmentProgram,"alpha");
	CGparameter rBeta = cgGetNamedParameter (diffuseVel->fragmentProgram,"rBeta");
	CGparameter x= cgGetNamedParameter (diffuseVel->fragmentProgram,"x");
	CGparameter b= cgGetNamedParameter (diffuseVel->fragmentProgram,"b");

	cgGLEnableProfile(diffuseVel->fragmentProfile);

	cgGLBindProgram(diffuseVel->fragmentProgram);
	cgGLSetTextureParameter(x, velTex);
	cgGLEnableTextureParameter(x);
	cgGLSetTextureParameter(b, velTex);
	cgGLEnableTextureParameter(b);

	float alp = (dx)*(dx) / (n*dt);
	float rb = 1/(alp+4);

	cgGLSetParameter1f(alpha,alp);
	cgGLSetParameter1f(rBeta,rb);

	rt1->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt1->finishRender();
	rt1->bind();

	texOut=rt1->textureID();
	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RG,GL_FLOAT,velocity);
	swapVelocities();

    return ;
}


void computeDivergence()
{
	
	CGparameter halfrdx = cgGetNamedParameter (divergenceProg->fragmentProgram,"halfrdx");
	CGparameter w= cgGetNamedParameter (divergenceProg->fragmentProgram,"w");

	cgGLSetTextureParameter(w, velTex);
	cgGLEnableTextureParameter(w);

	cgGLSetParameter1f(halfrdx,0.5/dx);

	rt2->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt2->finishRender();
	rt2->bind();

	texOut=rt2->textureID();
	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RED,GL_FLOAT,divergence);

	glBindTexture(GL_TEXTURE_RECTANGLE, divTex);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE,0,0,0,screen_width,screen_height,GL_RED,GL_FLOAT,divergence);


	return ;
}

void computePressure()
{
	
	CGparameter alpha = cgGetNamedParameter (compPressure->fragmentProgram,"alpha");
	CGparameter rBeta = cgGetNamedParameter (compPressure->fragmentProgram,"rBeta");
	CGparameter x= cgGetNamedParameter (compPressure->fragmentProgram,"x");
	CGparameter b= cgGetNamedParameter (compPressure->fragmentProgram,"b");

	cgGLEnableProfile(compPressure->fragmentProfile);

	cgGLBindProgram(compPressure->fragmentProgram);
	cgGLSetTextureParameter(x, pressureTex);
	cgGLEnableTextureParameter(x);
	cgGLSetTextureParameter(b, divTex);
	cgGLEnableTextureParameter(b);
	float alp = -(dx)*(dx);
	float rb=0.25;
	cgGLSetParameter1f(alpha,alp);
	cgGLSetParameter1f(rBeta,rb);

	rt2->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt2->finishRender();
	rt2->bind();

	texOut=rt2->textureID();
	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RED,GL_FLOAT,pressure);

	glBindTexture(GL_TEXTURE_RECTANGLE, pressureTex);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE,0,0,0,screen_width,screen_height,GL_RED,GL_FLOAT,pressure);


    return ;
}

void subtractPressureGrad()
{
	
	CGparameter halfrdx = cgGetNamedParameter (subPres->fragmentProgram,"halfrdx");
	CGparameter p= cgGetNamedParameter (subPres->fragmentProgram,"p");
	CGparameter w= cgGetNamedParameter (subPres->fragmentProgram,"w");

	cgGLEnableProfile(subPres->fragmentProfile);

	cgGLBindProgram(subPres->fragmentProgram);
	cgGLSetTextureParameter(p, pressureTex);
	cgGLEnableTextureParameter(p);
	cgGLSetTextureParameter(w, velTex);
	cgGLEnableTextureParameter(w);

	cgGLSetParameter1f(halfrdx,0.5/dx);

	rt1->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt1->finishRender();
	rt1->bind();

	texOut=rt1->textureID();
	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RG,GL_FLOAT,velocity);

	swapVelocities();

    return ;
}

void applyVelocityBoundary()
{
	CGparameter scale= cgGetNamedParameter (velBoundary->fragmentProgram,"scale");
	CGparameter x= cgGetNamedParameter (velBoundary->fragmentProgram,"x");

	cgGLEnableProfile(velBoundary->fragmentProfile);

	cgGLBindProgram(velBoundary->fragmentProgram);
	cgGLSetTextureParameter(x, velTex);
	cgGLEnableTextureParameter(x);
	
	cgGLSetParameter1f(scale,-1);

	rt1->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt1->finishRender();
	rt1->bind();

	texOut=rt1->textureID();
	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RG,GL_FLOAT,velocity);

	swapVelocities();


    return ;
}

void applyPressureBoundary()
{
	
	CGparameter scale= cgGetNamedParameter (presBoundary->fragmentProgram,"scale");
	CGparameter x= cgGetNamedParameter (presBoundary->fragmentProgram,"x");

	cgGLEnableProfile(presBoundary->fragmentProfile);

	cgGLBindProgram(presBoundary->fragmentProgram);
	cgGLSetTextureParameter(x, pressureTex);
	cgGLEnableTextureParameter(x);
	
	cgGLSetParameter1f(scale,1);

	rt2->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt2->finishRender();
	rt2->bind();

	texOut=rt2->textureID();
	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RED,GL_FLOAT,pressure);

	glBindTexture(GL_TEXTURE_RECTANGLE, pressureTex);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE,0,0,0,screen_width,screen_height,GL_RED,GL_FLOAT,pressure);

    return ;
}


void computeVorticity()
{
	CGparameter w= cgGetNamedParameter (compVorticity->fragmentProgram,"x");

	cgGLSetTextureParameter(w, velTex);
	cgGLEnableTextureParameter(w);


	rt2->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt2->finishRender();
	rt2->bind();

	texOut=rt2->textureID();
	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RED,GL_FLOAT,vorticity);

	glBindTexture(GL_TEXTURE_RECTANGLE, vorTex);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE,0,0,0,screen_width,screen_height,GL_RED,GL_FLOAT,vorticity);
	
	return ;

}


void applyVortex()
{
	cgGLLoadProgram (applyVorticity->fragmentProgram);
	cgGLEnableProfile(applyVorticity->fragmentProfile);

	cgGLBindProgram(applyVorticity->fragmentProgram);

	CGparameter w = cgGetNamedParameter (applyVorticity->fragmentProgram,"w");
	CGparameter v = cgGetNamedParameter (applyVorticity->fragmentProgram,"v");
		
	cgGLSetTextureParameter(w, vorTex);
	cgGLEnableTextureParameter(w);
	cgGLSetTextureParameter(v, velTex);
	cgGLEnableTextureParameter(v);

	rt1->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt1->finishRender();
	rt1->bind();

	texOut=rt1->textureID();
	
	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RG,GL_FLOAT,velocity);
	swapVelocities();

    return ;
}

void accelerate(int startX,int startY, int delx,int dely)
{

	//Position x is starting point of impulse
	//fx is x-component of impulse length i.e. delta(x)

	//Position y is starting point of impulse
	//fx is y-component of impulse length i.e. delta(y)

	//Lengths may be large so we factor down : 

	float fx=delx;
	float fy=dely;

	float posX = startX;
	float posY = startY;
	
	
	cgGLLoadProgram (acc->fragmentProgram);
	cgGLEnableProfile(acc->fragmentProfile);

	cgGLBindProgram(acc->fragmentProgram);

	CGparameter v = cgGetNamedParameter (acc->fragmentProgram,"v");
	CGparameter force = cgGetNamedParameter (acc->fragmentProgram,"force");
	CGparameter pos = cgGetNamedParameter (acc->fragmentProgram,"pos");
    

	cgGLSetTextureParameter(v, velTex);
	cgGLEnableTextureParameter(v);
	

	cgGLSetParameter2f(force,fx,fy);
	cgGLSetParameter2f(pos,posX,posY);


	rt1->startRender();
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);

	glEnd();
	rt1->finishRender();
	rt1->bind();

	texOut=rt1->textureID();
	
	glBindTexture(GL_TEXTURE_RECTANGLE,texOut);
	glGetTexImage(GL_TEXTURE_RECTANGLE,0,GL_RG,GL_FLOAT,velocity);
	swapVelocities();

    return ;
}

	
void dispFun()
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	cgContext = cgCreateContext();
	fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(fragmentProfile);
	fragmentProgram=cgCreateProgramFromFile( cgContext,CG_SOURCE,"pShader.cg", fragmentProfile,"colorpixel",NULL );
	cgGLLoadProgram (fragmentProgram);

	CGparameter x= cgGetNamedParameter (fragmentProgram,"x");

	cgGLEnableProfile(fragmentProfile);

	cgGLBindProgram(fragmentProgram);
	cgGLSetTextureParameter(x, colTex);
	cgGLEnableTextureParameter(x);


	glBegin(GL_QUADS);	
		glVertex2f(-1,-1);
		glVertex2f(-1,1);
		glVertex2f(1,1);
		glVertex2f(1,-1);
	glEnd();

	glFlush();
	
    return ;
}

void update()
{
	int i,j;

	applyVelocityBoundary();

	advectVelocity();
    
	for (i=0;i<num;i++)
	diffuseVelocity();

    computeDivergence();

	for (i=0;i<num2;i++)
	{
		computePressure();
	}

    subtractPressureGrad();

	applyVelocityBoundary();
    applyPressureBoundary();

	computeVorticity();

	applyVortex();
	
	applyVelocityBoundary();

	advectColor();

	glutPostRedisplay();

    return ;
}

/*Function to Handle Keyboard Buttons*/
void handleKey(unsigned char key, int x, int y ){

    switch (key) {
    default:
        break;
    case 27:
        fcloseall();
        printf("Esc key pressed\n");
        exit(EXIT_SUCCESS);
        break;
    case 'r': //RED
        mouseColor = 0 ;
        break;
    case 'g': //GREEN
        mouseColor = 1 ;
        break;
    case 'b': //BLUE
        mouseColor = 2 ;
        break;
    case 'c': //CYAN
        mouseColor = 3 ;
        break;
    case 't': //MAGENTA
        mouseColor = 4 ;
        break;
    case 'y': //YELLOW
        mouseColor = 5 ;
        break;
    case 'm':
        mouseEnable = !mouseEnable ;  
        break ;
    case 'f':
        mouseEnable2 = !mouseEnable2 ;
        break ;
    case 's':
        startUpdate = !startUpdate ;
        if(startUpdate) glutIdleFunc(update);
        else glutIdleFunc(update);
        break ;
   }
}

void handleMouse(int button, int state, int x, int y){

	switch(button)
    {
        case GLUT_LEFT_BUTTON :{
		    if(state == GLUT_DOWN)
            {
                if(mouseEnable)
                glutMotionFunc(InjectDye) ;
			}
            if(state == GLUT_UP){
                glutMotionFunc(NULL);
            }   
        }
		break;
	


	case GLUT_RIGHT_BUTTON :{
		    if(state == GLUT_DOWN)
            {
                if(mouseEnable2)
                glutMotionFunc(mouseDrag) ;
			}
            if(state == GLUT_UP){
                if(mouseEnable2){
                accelerate(startX,startY,endX-startX,endY-startY);
                startX = -1 ;
                }
                glutMotionFunc(NULL);
            }   
        }
		break;
	}
}

void mouseDrag(int tx, int ty)
{
	if(startX==-1)
	{
		startX=tx;
		startY=DIM-ty;
	}
	endX=tx;
	endY=DIM-ty;
}

//Used to Inject dye in the grid at mouse click
void InjectDye(int tx, int ty)
{
            int x = DIM - ty ;
            int y = tx ;
            if(x-mouseBox>=0 && x+mouseBox<DIM)
                if(y-mouseBox>=0 && y+mouseBox<DIM){
                for(int i = -mouseBox ; i<=mouseBox ; i++)
                    for(int j = -mouseBox ; j<=mouseBox ; j++){
                        switch(mouseColor){
                        default:
                        dye[x+i][y+j][1] = 1.0 ;
                        break;
                        case 0: //Red
                        dye[x+i][y+j][0] = 1.0 ;
                        break;
                        case 1: //Green
                        dye[x+i][y+j][1] = 1.0 ;
                        break;
                        case 2: //Blue
                        dye[x+i][y+j][2] = 1.0 ;
                        break;
                        case 3: //Cyan
                        dye[x+i][y+j][1] = 1.0 ;
                        dye[x+i][y+j][2] = 1.0 ;
                        break;
                        case 4: //Magenta
                        dye[x+i][y+j][0] = 1.0 ;
                        dye[x+i][y+j][2] = 1.0 ;
                        break;
                        case 5: //Yellow
                        dye[x+i][y+j][0] = 1.0 ;
                        dye[x+i][y+j][1] = 1.0 ;
                        break;
                        }
                    }
            }
            swapColors();
}

void randomize(){
    
    int upper = (int)DIM*DIM*0.8 ; //Coloring approximate 1/2 pixels of the grid.
    
    int tempX = 0 ;
    int tempY = 0 ;
    
    for(int i=0 ; i<upper ; i++){
    
        tempX = ((int)rand())%DIM ;
        tempY = ((int)rand())%DIM ;
        
        if(tempX>=3 && tempX<=252 && tempY>=3 && tempY<=252)
        dye[tempX][tempY][i%3] = 1.0 ;
    }
    
    return ;
}

void randomizeVel(){
    
    int upper = (int)DIM*DIM ;
    
    int tempX = 0 ;
    int tempY = 0 ;
    
    float vx , vy ;
    
    for(int i=0 ; i<upper ; i++){
    
        tempX = ((int)rand())%DIM ;
        tempY = ((int)rand())%DIM ;
        
        vx = -1+2*((float)rand())/RAND_MAX ;
        if(vx>=0 && vx<0.5) vx = 0.6 ;
        if(vx<0 && vx>-.05) vx = 0.6 ;
        
        vy = -1+2*((float)rand())/RAND_MAX ;
        if(vx>=0 && vx<0.5) vx = 0.6 ;
        if(vx<0 && vx>-.05) vx = 0.6 ;

        
        velocity[tempX][tempY][0] = vx;
        velocity[tempX][tempY][1] = vy;
    }
    
    return ;
}

