// Mundo.cpp: implementation of the CMundo class.
//
//////////////////////////////////////////////////////////////////////
#include "MundoCliente.h"
#include "glut.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMundo::CMundo()
{
	Init();
}

CMundo::~CMundo()
{
	close(fdTuberia);
	close(fdTeclas);
	unlink("/tmp/FIFOTeclas");
}

void CMundo::InitGL()
{
	//Habilitamos las luces, la renderizacion y el color de los materiales
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	
	glMatrixMode(GL_PROJECTION);
	gluPerspective( 40.0, 800/600.0f, 0.1, 150);
}

void print(char *mensaje, int x, int y, float r, float g, float b)
{
	glDisable (GL_LIGHTING);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT) );

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColor3f(r,g,b);
	glRasterPos3f(x, glutGet(GLUT_WINDOW_HEIGHT)-18-y, 0);
	int len = strlen (mensaje );
	for (int i = 0; i < len; i++) 
		glutBitmapCharacter (GLUT_BITMAP_HELVETICA_18, mensaje[i] );
		
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable( GL_DEPTH_TEST );
}
void CMundo::OnDraw()
{
	//Borrado de la pantalla	
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Para definir el punto de vista
	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();
	
	gluLookAt(0.0, 0, 17,  // posicion del ojo
		0.0, 0.0, 0.0,      // hacia que punto mira  (0,0,0) 
		0.0, 1.0, 0.0);      // definimos hacia arriba (eje Y)    

	/////////////////
	///////////
	//		AQUI EMPIEZA MI DIBUJO
	char cad[100];
	sprintf(cad,"Jugador1: %d",puntos1);
	print(cad,10,0,1,1,1);
	sprintf(cad,"Jugador2: %d",puntos2);
	print(cad,650,0,1,1,1);
	int i;
	for(i=0;i<paredes.size();i++)
		paredes[i].Dibuja();

	fondo_izq.Dibuja();
	fondo_dcho.Dibuja();
	jugador1.Dibuja();
	jugador2.Dibuja();
	esfera.Dibuja();

	/////////////////
	///////////
	//		AQUI TERMINA MI DIBUJO
	////////////////////////////

	//Al final, cambiar el buffer
	glutSwapBuffers();
}

void CMundo::OnTimer(int value)
{	
	if(puntos1 >= 3) {
		fifo << "Juego terminado, gana jugador 1." << std::endl;
		exit(0);
	}

	if(puntos2 >= 3) {
		fifo << "Juego terminado, gana jugador 2." << std::endl;
		exit(0);
	}

	botParams->esfera = esfera;
	botParams->raqueta1 = jugador2;

	if (botParams->accion != ultimaaccion) {
		switch (botParams->accion) {
			case -1:
				OnKeyboardDown('l', 0, 0);
				break;

			case 1:
				OnKeyboardDown('o', 0, 0);
				break;

			case 0:
			default:
				OnKeyboardDown('p', 0, 0);
				break;
		}
	}

	ultimaaccion = botParams->accion;

	// jugador1.Mueve(0.025f);
	// jugador2.Mueve(0.025f);
	// esfera.Mueve(0.025f);
	// int i;
	// for(i=0;i<paredes.size();i++)
	// {
	// 	paredes[i].Rebota(esfera);
	// 	paredes[i].Rebota(jugador1);
	// 	paredes[i].Rebota(jugador2);
	// }

	// jugador1.Rebota(esfera);
	// jugador2.Rebota(esfera);
	// if(fondo_izq.Rebota(esfera))
	// {
	// 	esfera.centro.x=0;
	// 	esfera.centro.y=rand()/(float)RAND_MAX;
	// 	esfera.velocidad.x=2+2*rand()/(float)RAND_MAX;
	// 	esfera.velocidad.y=2+2*rand()/(float)RAND_MAX;
	// 	puntos2++;
	// 	fifo << "Jugador 2 marca 1 punto, lleva un total de " << puntos2 << std::endl;
	// }

	// if(fondo_dcho.Rebota(esfera))
	// {
	// 	esfera.centro.x=0;
	// 	esfera.centro.y=rand()/(float)RAND_MAX;
	// 	esfera.velocidad.x=-2-2*rand()/(float)RAND_MAX;
	// 	esfera.velocidad.y=-2-2*rand()/(float)RAND_MAX;
	// 	puntos1++;
	// 	fifo << "Jugador 1 marca 1 punto, lleva un total de " << puntos1 << std::endl;
	// }

	char cad[200];
	int n = read(fdTuberia, cad, sizeof(cad));
	if (n > 0) {
	sscanf(cad, "%f %f %f %f %f %f %f %f %f %f %d %d", 
			&esfera.centro.x, &esfera.centro.y,
			&jugador1.x1, &jugador1.y1, &jugador1.x2, &jugador1.y2, 
			&jugador2.x1, &jugador2.y1, &jugador2.x2, &jugador2.y2,
			&puntos1, &puntos2);
	}
}

void CMundo::OnKeyboardDown(unsigned char key, int x, int y)
{
	// switch(key)
	// {
	// // case 'a':jugador1.velocidad.x=-1;break;
	// // case 'd':jugador1.velocidad.x=1;break;
	// case 's':jugador1.velocidad.y=-4;break;
	// case 'w':jugador1.velocidad.y=4;break;
	// // case 'l':jugador2.velocidad.y=-4;break;
	// // case 'o':jugador2.velocidad.y=4;break;
	// }

	char cad[10];
	sprintf(cad, "%c", key);
	write(fdTeclas, cad, strlen(cad) + 1);
}

void CMundo::Init()
{
	// Apertura de la tuberia
	// fifo.open("/tmp/tenisFIFO");
	// if (!fifo.is_open()) {
	// 	std::cerr << "No se ha podido abrir el FIFO." << std::endl;
	// }

	int fd = open("/tmp/datosBot.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (fd == -1) {
		std::cerr << "No se ha podido crear el fichero compartido con el Bot" << std::endl;
		exit(-1);
	}
	ftruncate(fd, sizeof(DatosMemCompartida));
	botParams = (DatosMemCompartida*) mmap(NULL, sizeof(DatosMemCompartida), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if (botParams == MAP_FAILED) {
		std::cerr << "Error en la proyeccion mmap del fichero" << std::endl;
		exit(-1);
	}
	std::cout << "Memoria compartida creada" << std::endl;

	fdTuberia = open("/tmp/FIFOClientServer", O_RDONLY);
	if (fdTuberia < 0) {
		perror("No se puede abrir el FIFO");
		exit(1);
	}
	std::cout << "Tuberia para el cliente abierta" << std::endl;

	unlink("/tmp/FIFOTeclas");
	if (mkfifo("/tmp/FIFOTeclas", 0666) < 0) {
		perror("Error al crear FIFO teclas");
		exit(1);
	}
	fdTeclas = open("/tmp/FIFOTeclas", O_WRONLY);
	if (fdTeclas < 0) {
		perror("No se puede abrir el FIFO teclas");
		exit(1);
	}
	std::cout << "Tuber'ia para teclas creada" << std::endl;

	Plano p;
//pared inferior
	p.x1=-7;p.y1=-5;
	p.x2=7;p.y2=-5;
	paredes.push_back(p);

//superior
	p.x1=-7;p.y1=5;
	p.x2=7;p.y2=5;
	paredes.push_back(p);

	fondo_izq.r=0;
	fondo_izq.x1=-7;fondo_izq.y1=-5;
	fondo_izq.x2=-7;fondo_izq.y2=5;

	fondo_dcho.r=0;
	fondo_dcho.x1=7;fondo_dcho.y1=-5;
	fondo_dcho.x2=7;fondo_dcho.y2=5;

	//a la izq
	jugador1.g=0;
	jugador1.x1=-6;jugador1.y1=-1;
	jugador1.x2=-6;jugador1.y2=1;

	//a la dcha
	jugador2.g=0;
	jugador2.x1=6;jugador2.y1=-1;
	jugador2.x2=6;jugador2.y2=1;
}
