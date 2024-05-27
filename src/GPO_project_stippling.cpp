/************************  GPO Iluminacion  **********************************
ATG, 2020
******************************************************************************/

#include <GpO.h>
#include <GpOObjectLoader.hpp>

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL(GpO) Iluminacion stippling";   // Nombre de la practica (aparecera en el titulo de la ventana).
static const float MAX_BPP = pow(2, VGA_NUM_PIX);
static const int MAX_PATTERN = pow(2, PATTERN_BITS);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* vertex_prog_stippling = leer_codigo_de_fichero("bin/data/stippling.vs");
const char* fragment_prog_stippling = leer_codigo_de_fichero("bin/data/stippling.fs");

////////////////////////////////  FIN PROGRAMAS GPU (SHADERS) //////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
///////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog[1];
obj modelo;


// Dibuja obj indexado
void dibujar_indexado(obj obj)
{
  glBindVertexArray(obj.VAO);              // Activamos VAO asociado al obj
  glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
  glBindVertexArray(0);
}


// Variables globales
float az=0.0f, el=0.75f, d = 1.3f, zfar = 25.0f, znear = 0.2f;
mat4 Proy,View,M, R, T, S;
vec3 campos=vec3(0.0f,0.0f,d);											
vec3 target=vec3(0.0f,0.0f,0.0f);
vec3 up = vec3(0.0f, 1.0f, 0.0f);
int prog_selected = 0;
int pattern = 0xAAAA;
float factor = 1.0f;

// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Preparaci�n de los datos de los objs a dibujar, envialarlos a la GPU
// Opciones generales de render de OpenGL
void init_scene()
{
	prog[0] = Compile_Link_Shaders(vertex_prog_stippling, fragment_prog_stippling); // Compile shaders prog_stippling1
	
	glUseProgram(prog[prog_selected]);

	modelo = cargar_obj((char*) "bin/data/melinoe_weapons.obj");
	GLuint tex0 = cargar_textura("bin/data/melinoe.jpg", GL_TEXTURE0);

	// transfer_vec3("color", vec3(0.314,0.784,0.471));
	transfer_vec3("color_luz", vec3(1,1,1));

	glEnable(GL_CULL_FACE); glEnable(GL_DEPTH_TEST);
}


// Actualizar escena: cambiar posici�n objs, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float tt = (float)glfwGetTime();  // Contador de tiempo en segundos 
	
	glUseProgram(prog[prog_selected]);
	campos = d * vec3(sin(az)*cos(el), cos(az)*cos(el), sin(el));

	Proy = perspective(radians(55.0f), 4.0f / 3.0f, znear, zfar); 
	View = lookAt(campos,target,up);

	vec3 light_dir = vec3(cos(el)*cos(az), sin(el), cos(el)*sin(az));
	S = scale(mat4(1.0f),vec3(2.0f));
    R = rotate(90.0f, vec3(0.0f, 0.0f, 1.0f))*rotate(30.0f, vec3(1.0f, 0.0f, 0.0f))*rotate(120.0f, vec3(0.0f, 1.0f, 0.0f));
	M = R * S;
	
	transfer_mat4("MVP", Proy*View*M);
	transfer_mat4("View", View);
	transfer_vec3("luz", light_dir);
	transfer_vec3("campos", campos);
	transfer_int("ALTO", ALTO);
	transfer_int("ANCHO", ANCHO);
	transfer_int("pattern", pattern);
	transfer_float("factor", factor);
	dibujar_indexado(modelo);
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PROGRAMA PRINCIPAL

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	init_GLFW();            // Inicializa lib GLFW
	window = Init_Window(prac);  // Crea ventana usando GLFW, asociada a un contexto OpenGL	X.Y
	load_Opengl();         // Carga funciones de OpenGL, comprueba versi�n.
	init_scene();          // Prepara escena

	while (!glfwWindowShouldClose(window))
	{
		render_scene();  
		glfwSwapBuffers(window); glfwPollEvents();
		show_info();
	}

	glfwTerminate();

	exit(EXIT_SUCCESS);
}



/////////////////////  FUNCION PARA MOSTRAR INFO EN TITULO DE VENTANA  //////////////
void show_info()
{
	static int fps = 0;
	static double last_tt = 0;
	double elapsed, tt;
	char nombre_ventana[128];   // buffer para modificar titulo de la ventana

	fps++; tt = glfwGetTime();  // Contador de tiempo en segundos 

	elapsed = (tt - last_tt);
	if (elapsed >= 0.5)  // Refrescar cada 0.5 segundo
	{
		sprintf_s(nombre_ventana, 128, "%s: %4.0f FPS @ %d x %d", prac, fps / elapsed, ANCHO, ALTO);
		glfwSetWindowTitle(window, nombre_ventana);
		last_tt = tt; fps = 0;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////  INTERACCION  TECLADO RATON
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Callback de cambio tama�o
void ResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	ALTO = height;
	ANCHO = width;
}

static void KeyCallback(GLFWwindow* window, int key, int code, int action, int mode)
{	
	switch (key)
	{
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
		case GLFW_KEY_UP: if(action) el+=0.02f; break;
		case GLFW_KEY_DOWN: if(action) el-=0.02f; break;
		case GLFW_KEY_LEFT: if(action) az+=0.02f; break;
		case GLFW_KEY_RIGHT: if(action) az-=0.02f; break;
		case GLFW_KEY_1: if(action  && factor > 1) factor-=1.0f; break;
		case GLFW_KEY_2: if(action && factor < MAX_BPP) factor+=1.0f; break;
		case GLFW_KEY_3: if(action && pattern > 1) pattern -= 1; break;
		case GLFW_KEY_4: if(action && pattern < MAX_PATTERN) pattern += 1; break;
	}
    fprintf(stdout,"factor %f pattern %d\n", factor, pattern);
}

double xp, yp;

static void moverCallback(GLFWwindow* window, double x, double y) {
    // Los parámetros x, y nos dan la posición actual del cursor en la ventana.
    fprintf(stdout,"x %.1f y %.1f\n", x, y);
    double F = 0.007f * d;
    double dx = x - xp;
    double dy = y - yp;
    az = F * dx;
    el = F * dy;
}

GLfloat zp;

void pulsarCallback(GLFWwindow* window, int Button, int Action, int Mode) {
    // El parámetro Button nos da el botón pulsado (0=izquierdo, 1= derecho). 
    // Action nos indica si se ha pulsado (1) o liberado (0). 
    // Mode si se ha pulsado en combinación con alguna otra tecla (Shift, Ctrl, …).
	fprintf(stdout, "Button %d Act %d Mode %d\n", Button, Action, Mode);
    if (!Button && Action) {
        glfwGetCursorPos(window,&xp,&yp);
        glfwSetCursorPosCallback(window, moverCallback);
    }

    if (!Button && !Action) glfwSetCursorPosCallback(window, NULL);

    if (Button && Action) {
        glfwGetCursorPos(window,&xp,&yp);
        glReadPixels((GLint) xp, (GLint) yp, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zp);
        fprintf(stdout, "xp= %.0f yp= %.0f zp= %.3f\n", xp, yp, zp);
        float xn=(2*(xp/ANCHO))-1;
        float yn=(2*(yp/ALTO))-1;
        float zn=2*zp-1;
        fprintf(stdout, "xn= %.0f yn= %.0f zn= %.3f\n", xn, yn, zn);
        float z_cam = (-2*znear*zfar)/((zfar+znear)+zn*(znear-zfar));
        float x_cam = -((xn*z_cam)/Proy[0][0]); 
        float y_cam = -((yn*z_cam)/Proy[1][1]);
        fprintf(stdout, "xcam= %.0f ycam= %.0f zcam= %.3f\n", x_cam, y_cam, z_cam);
    }
}

static void scrollCallback(GLFWwindow* window, double dx, double dy) {
    // Los parámetros dx,dy nos dan el "scrolling" en los ejes X e Y. 
    // En el caso de usar la rueda del ratón dx=0 y solo tenemos "scrolling" vertical.
    fprintf(stdout,"x %.1f y %.1f\n", dx, dy);
    if (dy > 0.0f) d+=0.1f;
    if (dy < 0.0f) d-=0.1f;
    fprintf(stdout,"x %.1f y %.1f z %.1f\n", campos.x, campos.y, campos.z);
    fprintf(stdout,"d %.1f\n", d);
}

void asigna_funciones_callback(GLFWwindow *window) {
    glfwSetWindowSizeCallback(window, ResizeCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, pulsarCallback);
    glfwSetScrollCallback(window, scrollCallback);
}



 
