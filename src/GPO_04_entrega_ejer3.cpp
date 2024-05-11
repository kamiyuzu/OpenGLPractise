/************************  GPO Iluminacion  **********************************
ATG, 2020
******************************************************************************/

#include <GpO.h>

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL(GpO) Iluminacion";   // Nombre de la practica (aparecera en el titulo de la ventana).


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

//////////////////////////////////////////////////////////////////////////////////

//  PROGRAMA 2 (aqui implementaremos version en fragmentos)
const char* vertex_prog2 = GLSL(
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
out vec3 n;
out vec3 v;
out vec3 viewDir;

uniform vec3 campos;
uniform mat4 M;
uniform mat4 PV;

void main() {
	gl_Position = PV*M*vec4(pos, 1);

	vec3 scene_vertex_position = vec3(M*vec4(pos, 1));
	v = scene_vertex_position - campos; // unit_vertex_to_cam
	viewDir = campos - pos;

	mat3 M_adj = mat3(transpose(inverse(M)));
	n = M_adj * normal;
}
);

const char* fragment_prog2 = GLSL(
in vec3 v;
in vec3 n;
in vec3 viewDir;
uniform vec3 luz = vec3(1, 1, 0) / sqrt(2.0f);
uniform vec4 ilu_coef;
out vec3 col;  // Color fragmento
void main()
{
	vec3 v_n = normalize(v); // unit_vertex_to_cam
	vec3 nn = normalize(n);
	vec3 halfwayDir = normalize(luz + viewDir);

	float spec = pow(max(dot(nn, halfwayDir), 0.0), 16.0f);
	float difusa = dot(luz,nn); if (difusa < 0) difusa = 0; 
	float ilu = (ilu_coef.x + ilu_coef.y*difusa + ilu_coef.z*spec);

	col = vec3(1, 1, 0.9);
	col = col*ilu;
}
);

////////////////////////////////  FIN PROGRAMAS GPU (SHADERS) //////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
///////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog[1];
objeto modelo;


// Dibuja objeto indexado
void dibujar_indexado(objeto obj)
{
  glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
  glDrawElements(GL_TRIANGLES,obj.Ni,obj.tipo_indice,(void*)0);  // Dibujar (indexado)
  glBindVertexArray(0);
}


// Variables globales
mat4 Proy,View,M;
vec3 campos=vec3(0.0f,0.0f,2.5f);											
vec3 target=vec3(0.0f,0.9f,0.0f);
vec3 up = vec3(0, 1, 0);
vec4 ilu_coef[3] = {vec4(0.2, 0, 0, 10), vec4(0, 1, 0, 40), vec4(0, 0, 1, 60)};
int prog_selected = 0;
float az=0.0f, el=0.75;

// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Opciones generales de render de OpenGL
void init_scene()
{
	prog[0] = Compile_Link_Shaders(vertex_prog2, fragment_prog2); // Compile shaders prog2
	
	glUseProgram(prog[prog_selected]);

	modelo = cargar_modelo((char*) "bin/data/buda_n.bix");

	Proy = glm::perspective(glm::radians(55.0f), 4.0f / 3.0f, 0.1f, 100.0f); 
	View = glm::lookAt(campos,target,up);

	glEnable(GL_CULL_FACE); glEnable(GL_DEPTH_TEST);
}


// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float tt = (float)glfwGetTime();  // Contador de tiempo en segundos 
	
	glUseProgram(prog[prog_selected]);

	vec3 light_dir = vec3(cos(el)*cos(az), sin(el), cos(el)*sin(az));
	vec3 x=vec3(0.0f, 0.0f, 0.0f);
	M = translate(x)*rotate(1*tt, vec3(0.0f, 1.0f, 0.0f));   // Mov modelo 
	
	transfer_mat4("PV",Proy*View); transfer_mat4("M", M);
	transfer_vec3("luz", light_dir);
	transfer_vec3("campos", campos);
	transfer_vec4("ilu_coef", ilu_coef[1]);
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
		case GLFW_KEY_TAB:
			if(action) {
				if(prog_selected) prog_selected=0;
				else prog_selected=1;
			}
			break;
		case GLFW_KEY_UP: if(action && el<=M_PI/2) el+=0.02f; break;
		case GLFW_KEY_DOWN: if(action && el>=-M_PI/2) el-=0.02f; break;
		case GLFW_KEY_LEFT: if(action) az+=0.02f; break;
		case GLFW_KEY_RIGHT: if(action) az-=0.02f; break;
	}
}


void asigna_funciones_callback(GLFWwindow* window)
{
	glfwSetWindowSizeCallback(window, ResizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
}



 
