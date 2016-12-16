
/* DreamWorks Spark challenge */

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
//#include <GL/stdafx.h>



#define random() ((float)rand() / RAND_MAX)
#define world_gravity - 9.8 

typedef struct {
	float x, y, z;
	float R;
} PSsphere;

typedef struct {
	
	float previous[3];                  // previous position 
	float position[3];                  // current position
	float velocity[3];                  // velocity (magnitude & direction)
	int alive;                          // particle alive state 
	float energy_loss;                  // % of energy lost on collision 
	float life_span;					// ultimate kill switch
} PSparticle;

PSparticle* particles = NULL;

int max_particle = 20000;
float flow = 500;
float slow = 1;
int point_size = 5;
float x_spin = 0;
float y_spin = 0;



#include <sys/timeb.h>
#define CLK_TCK 1000

float timedelta(void)
{
	static long begin = 0;
	static long finish, difference;
	static struct timeb tb;
	ftime(&tb);
	finish = tb.time * 1000 + tb.millitm;
	difference = finish - begin;
	begin = finish;
	return (float)difference / (float)CLK_TCK;
}



int fequal(float a, float b)
{
	float epsilon = 0.7;
	float f = a - b;
	if (f < epsilon && f > -epsilon)
		return 1;
	else
		return 0;
}

PSsphere spheres[1] = {       /* position of spheres */
	{ 1.0, 0.4, 0, 0.4 },
};

void psTimeStep(PSparticle* p, float dt)
{
	if (p->alive == 0)
		return;
	p->velocity[0] += 0;
	p->velocity[1] += world_gravity*dt; /*v = u + at*/
	p->velocity[2] += 0;
	p->previous[0] = p->position[0];
	p->previous[1] = p->position[1];
	p->previous[2] = p->position[2];
	p->position[0] += p->velocity[0] * dt; /* s = ut + 0.5at^2 with no acceleration*/
	p->position[1] += p->velocity[1] * dt;
	p->position[2] += p->velocity[2] * dt;
	p->life_span ++ ;
}


void psNewParticle(PSparticle* p, float dt)
{
		p->velocity[0] = 50 * (random() - 0.0);
		p->velocity[1] = 10/(random() - 2);
		p->velocity[2] = 2*(random() - 0.5);
		p->position[0] = -1;
		p->position[1] = 2;
		p->position[2] = 0;
		p->previous[0] = p->position[0];
		p->previous[1] = p->position[1];
		p->previous[2] = p->position[2];
		p->energy_loss = 0.45*random();
		p->alive = 1;
		p->life_span = 0;
psTimeStep(p, 2 * dt*random());
}

void pssplitParticle(PSparticle* p, PSparticle old_p ,float dt) {
	*p = old_p;
	p->velocity[0] += random();
	p->velocity[1] += random();
	p->velocity[2] += random()/10;
	psTimeStep(p, 2 * dt*random());
}

void psBounce(PSparticle* p, float dt)
{
	float s;

	if (p->alive == 0)
		return;

	s = -p->previous[1] / p->velocity[1]; // time = distance / speed;

	p->position[0] = (p->previous[0] + p->velocity[0] * s +
		p->velocity[0] * (dt - s) * p->energy_loss);
	p->position[1] = -p->velocity[1] * (dt - s) * p->energy_loss; // reflect 
	p->position[2] = (p->previous[2] + p->velocity[2] * s +
		p->velocity[2] * (dt - s) * p->energy_loss);
	p->velocity[0] *= p->energy_loss;
	p->velocity[1] *= -p->energy_loss;
	p->velocity[2] *= p->energy_loss;

}

void psCollideSphere(PSparticle* p, PSsphere* s)
{
	float velocity_x = p->position[0] - s->x;
	float velocity_y = p->position[1] - s->y;
	float velocity_z = p->position[2] - s->z;
	float distance;
	
	if (p->alive == 0)
		return;
	
	distance = sqrt(velocity_x*velocity_x + velocity_y*velocity_y + velocity_z*velocity_z);// formula for distance between two particle

	if (distance < s->R) {

		p->position[0] = s->x + (velocity_x / distance)*s->R;
		p->position[1] = s->y + (velocity_y / distance)*s->R;
		p->position[2] = s->z + (velocity_z / distance)*s->R;
		p->previous[0] = p->position[0];
		p->previous[1] = p->position[1];
		p->previous[2] = p->position[2];
		p->velocity[0] = velocity_x / distance;
		p->velocity[1] = velocity_y / distance;
		p->velocity[2] = velocity_z / distance;

}
}


void reshape(int width, int height)
{
	float black[] = { 0, 0, 0, 0 };

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float)width / height, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 1, 3, 0, 1, 0, 0, 1, 0);
	glFogfv(GL_FOG_COLOR, black);
	glFogf(GL_FOG_START, 2.5);
	glFogf(GL_FOG_END, 4);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glPointSize(point_size);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	
	timedelta();
}


void display(void)
{
	static int i;
	static float c;
	static int j = 0;
	static char s[32];
	static int frames = 0;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	
	glRotatef(y_spin, 1, 0, 0);
	glRotatef(x_spin, 0, 1, 0);
	
	glEnable(GL_LIGHTING);
	
			glPushMatrix();
			glTranslatef(spheres[0].x, spheres[0].y, spheres[0].z);
			glColor3ub(250, 255, 128);
			glutSolidSphere(spheres[0].R, 16, 16);
			glPopMatrix();
	
	glDisable(GL_LIGHTING);

	glBegin(GL_QUADS);
	glColor3ub(107, 99, 99);
	glVertex3f(-4, 0, -4);
	glVertex3f(-4, 0, 4);
	glVertex3f(4, 0, 4);
	glVertex3f(4, 0, -4);
	glEnd();

		GLfloat fsizes[2];  // Store supported line width range
		GLfloat fCurrSize;
		glGetFloatv(GL_LINE_WIDTH_RANGE, fsizes);
		fCurrSize = fsizes[0];
		glLineWidth(fCurrSize);
		glBegin(GL_LINES);
		for (i = 0; i < max_particle; i++) {
			if (particles[i].alive == 0)
				continue;
			c = particles[i].previous[1] / 2.1 * 255;
			glColor4ub(255, 255, c * 4 / 5 + 30, 200);
			glVertex3fv(particles[i].previous);
			c = particles[i].position[1] / 2.1 * 255;
			glColor4ub(255, 232,c/80 + 2, 255);
			glVertex3fv(particles[i].position);
		}
		glEnd();

	glPopMatrix();
	glutSwapBuffers();
}

void idle(void)
{
	static int i, j;
	static int living = 0;	
	static int split_living = 0;
	static float small_time;
	static float last = 0;
	
	small_time = timedelta();
	
	//particle spawn
	for (i = 0; i < 30; i++) {
		psNewParticle(&particles[living], small_time);
		living++;
		if (living >= max_particle)
			living = 0;
	}

     for (i = 0; i < max_particle; i++) {

		 psTimeStep(&particles[i], small_time);	

		 psCollideSphere(&particles[i], &spheres[0]);//check collision
		
		 /* check collision with ground */
		 if (particles[i].position[1] <= 0) {
			 psBounce(&particles[i], small_time);
			 for (int x = 0; x < 2; x++) {
				 pssplitParticle(&particles[split_living], particles[i] ,small_time);
				 split_living++; 
			 }
			 if (split_living >= max_particle)
				 split_living = 0;
			 //particles[i].alive = 0;
		 }

		 /* check particle live or dead*/
		 if (particles[i].position[1] < 0.3 &&
			 fequal(particles[i].velocity[1], 0)  ) 
			 particles[i].alive = 0;
		 else if (particles[i].life_span >50)
			 particles[i].alive = 0;
		
	 }
	 
	 glutPostRedisplay();
}

void
visible(int state)
{
	if (state == GLUT_VISIBLE)
		glutIdleFunc(idle);
	else
		glutIdleFunc(NULL);
}

void
bail(int code)
{
	free(particles);
	exit(code);
}


 int old_x, old_y;

 void			
	 mouse(int button, int state, int x, int y)
 {
	 old_x = x;
	 old_y = y;

	 glutPostRedisplay();
 }

 void
	 motion(int x, int y)
 {
	 x_spin = x - old_x;
	 y_spin = y - old_y; 
	 glutPostRedisplay();
 }

 int
	 main(int argc, char** argv)
 {
	 glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	 glutInitWindowPosition(0, 0);
	 glutInitWindowSize(640, 480);
	 glutInit(&argc, argv);
	 glutCreateWindow("Dream sparks");
	 glutDisplayFunc(display);
	 glutReshapeFunc(reshape);
	 glutMotionFunc(motion);
	 glutMouseFunc(mouse);

	 if (argc > 1) {
		 if (strcmp(argv[1], "-h") == 0) {
			 fprintf(stderr, "%s [particles] [flow] [speed%]\n", argv[0]);
			 exit(0);
		 }
		 sscanf(argv[1], "%d", &max_particle);
		 if (argc > 2)
			 sscanf(argv[2], "%f", &flow);
		 if (argc > 3)
			 sscanf(argv[3], "%f", &slow);
	 }

	 particles = (PSparticle*)malloc(sizeof(PSparticle) * max_particle);
	 
	 glutVisibilityFunc(visible);
	 glutMainLoop();
	 return 0;
 }
