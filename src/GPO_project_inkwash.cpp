/************************  GPO Iluminacion  **********************************
ATG, 2020
******************************************************************************/

#include <GpO.h>

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL(GpO) Ink wash";   // Nombre de la practica (aparecera en el titulo de la ventana).


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 410 core\n" #src

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

    float spec = pow(max(dot(nn, halfwayDir), 0.0), 32.0f); // Aumentar el exponente especular para un efecto más marcado
    spec = smoothstep(0.005, 0.01, spec);
    float difusa = max(dot(luz, nn), 0.0);
    float ilu = ilu_coef.x + ilu_coef.y * difusa + ilu_coef.z * spec;

	// Detección de bordes usando derivadas de la normal
    float edge = 1.0 - smoothstep(0.1, 0.9, length(fwidth(nn))); // Ajustar los parámetros de detección de bordes

    // Ajustar la intensidad de la tinta y aumentar el contraste
    float tinta_intensity = clamp(ilu * 2.0 - 0.5, 0.0, 1.0);
	float intensity = dot(vec3(0.29, 0.59, 0.11), tinta_intensity * vec3(1.0)); 
    vec3 inkColor = vec3(intensity) * vec3(0.5, 0.5, 0.5); // Hacer la tinta más oscura
	// Aplicar un efecto difuminado adicional
    float blurAmount = smoothstep(0.0, 0.5, difusa) * 0.5 + 0.5; // Ajustar el efecto difuminado para transiciones más suaves
    // Ajuste de color para simular la tinta
	inkColor *= blurAmount;
    col = inkColor * edge;
}
);



////////////////////////////////  FIN PROGRAMAS GPU (SHADERS) //////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
///////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog[1];
obj modelo;
GLuint tex0;


// Dibuja objeto indexado
void dibujar_indexado(obj obj)
{
  glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
  glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
  glBindVertexArray(0);
}


// Variables globales
mat4 Proy,View,M,R,S;
vec3 campos=vec3(0.0f,0.0f,1.5f);											
vec3 target=vec3(0.0f,0.0f,0.0f);
vec3 up = vec3(0.0f, 1.0f, 0.0f);
vec4 ilu_coef[3] = {vec4(0.2, 0, 0, 10), vec4(0, 1, 0, 40), vec4(0, 0, 1, 60)};
int prog_selected = 0;
float az=7.2f, el=3.16;

// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Opciones generales de render de OpenGL
void init_scene()
{
	prog[0] = Compile_Link_Shaders(vertex_prog2, fragment_prog2); // Compile shaders prog2
	
	glUseProgram(prog[prog_selected]);

	modelo = cargar_obj((char*) "bin/data/melinoeweapons.obj");
	tex0 = cargar_textura("bin/data/melinoe.jpg", GL_TEXTURE0);

	Proy = glm::perspective(glm::radians(55.0f), 4.0f / 3.0f, 0.1f, 100.0f); 
	View = glm::lookAt(campos,target,up);

	glEnable(GL_CULL_FACE); glEnable(GL_DEPTH_TEST);
}


// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	glClearColor(0.9f, 0.9f, 0.9f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float tt = (float)glfwGetTime();  // Contador de tiempo en segundos 
	
	glUseProgram(prog[prog_selected]);

	vec3 light_dir = vec3(cos(el)*cos(az), sin(el), cos(el)*sin(az));
	S = scale(mat4(1.0f),vec3(2.0f));
    R = rotate(95.0f, vec3(0.0f, 0.0f, 1.0f))*rotate(90.0f, vec3(1.0f, 0.0f, 0.0f))*rotate(120.0f, vec3(0.0f, 1.0f, 0.0f));
	M = R * S;
	
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
		case GLFW_KEY_UP: if(action) el+=0.02f; break;
		case GLFW_KEY_DOWN: if(action) el-=0.02f; break;
		case GLFW_KEY_LEFT: if(action) az+=0.02f; break;
		case GLFW_KEY_RIGHT: if(action) az-=0.02f; break;
	}
}


void asigna_funciones_callback(GLFWwindow* window)
{
	glfwSetWindowSizeCallback(window, ResizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
}