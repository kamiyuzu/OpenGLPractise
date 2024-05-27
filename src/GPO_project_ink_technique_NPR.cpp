/************************  GPO NPR Ink Technique  **********************************
ATG, 2020
******************************************************************************/

#include <GpO.h>

// SIZE y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Size inicial ventana
const char* prac = "OpenGL(GpO) Iluminacion";   // Nombre de la practica (aparecera en el titulo de la ventana).


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

//////////////////////////////////////////////////////////////////////////////////

//  Shadder de vertices
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

// Shader de fragmentos

const char* fragment_prog2 = GLSL(
in vec3 v;
in vec3 n;
in vec3 viewDir;
uniform vec3 luz = vec3(1, 1, 0) / sqrt(2.0f);
uniform vec4 ilu_coef;
uniform float outter;
uniform float mid;
uniform float lower;
uniform float iTime;
out vec3 col;  // Color fragmento

// Funcion para generar ruido (pseudoaleatorio en intervalo [0,1])
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Funcion para calcular el ruido en base a los cuatro vertices de una cuadricula
float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

void main() {
    vec3 v_n = normalize(v); // unit_vertex_to_cam
    vec3 nn = normalize(n);
    vec3 halfwayDir = normalize(luz + viewDir);

    // Calcular componentes difusa y especular
    float spec = pow(max(dot(nn, halfwayDir), 0.0), 16.0f);
    float difusa = max(dot(luz, nn), 0.0);

    // Simular la textura de tinta con ruido
    float tinta = noise(gl_FragCoord.xy * 0.05 + iTime * 0.1);

    // Modificar el sombreado para un efecto de tinta estilizado
    float ilu = (ilu_coef.x + ilu_coef.y * difusa + ilu_coef.z * spec);
    ilu = ilu * tinta;

    // Deteccion de bordes utilizando normales
    float edge_detection = dot(nn, viewDir);
    edge_detection = smoothstep(0.0, 0.1, edge_detection);

    // Aplicar diferentes tonos de azul segun variables (modificables con entrada por teclado)
    vec3 color_tinta;
    if (ilu > outter)
        color_tinta = vec3(0.8, 0.8, 1.0); // Azul muy claro
    else if (ilu > mid)
        color_tinta = vec3(0.6, 0.6, 0.9); // Azul claro
    else if (ilu > lower)
        color_tinta = vec3(0.4, 0.4, 0.8); // Azul medio
    else
        color_tinta = vec3(0.2, 0.2, 0.6); // Azul mas oscuro
    
    color_tinta *= tinta; // Aplicar ruido para textura de tinta

    // Mezclar con el color de los bordes (negro)
    col = mix(color_tinta, vec3(0.0, 0.0, 0.0), edge_detection);
}
);
////////////////////////////////  FIN PROGRAMAS GPU (SHADERS) //////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   CODIGO PRINCIPAL Y RENDERS
///////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog[1];
obj modelo;

// Dibuja objeto indexado
void dibujar_indexado(obj obj)
{
  glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
  glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
  glBindVertexArray(0);
}

// Variables globales
mat4 Proy, View, M, R, S;
vec3 campos = vec3(0.0f, 0.0f, 1.5f);											
vec3 target = vec3(0.0f, 0.0f, 0.0f);
vec3 up = vec3(0.0f, 1.0f, 0.0f);
vec4 ilu_coef[3] = {vec4(0.2, 0, 0, 10), vec4(0, 1, 0, 40), vec4(0, 0, 1, 60)};
int prog_selected = 0;
float az = 1.14f, el = 1.4f, d = 1.2f, zfar = 25.0f, znear = 0.2f;
float outter = 0.96f, mid = 0.85f, lower = 0.61f;

// Compilacion programas a ejecutar:  vertex and fragment shaders
// Preparacion de los datos de los objetos a dibujar
// Opciones generales de render de OpenGL
void init_scene()
{
	prog[0] = Compile_Link_Shaders(vertex_prog2, fragment_prog2); // Compilar shaders
	
	glUseProgram(prog[prog_selected]);

	modelo = cargar_obj((char*) "bin/data/melinoe_weapons.obj");

	glEnable(GL_CULL_FACE); 
    	glEnable(GL_DEPTH_TEST);
}

// Actualizar escena: cambiar posicion objetos, nuevos objetos, posicion camara, luces, etc.
void render_scene()
{
	glClearColor(0.9f, 0.9f, 0.9f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float tt = (float)glfwGetTime();  // Contador de tiempo en segundos 
	
	glUseProgram(prog[prog_selected]);

	campos = d * vec3(sin(az) * cos(el), cos(az) * cos(el), sin(el));
	vec3 light_dir = vec3(cos(el) * cos(az), sin(el), cos(el) * sin(az));
	S = scale(mat4(1.0f), vec3(2.0f));
    R = rotate(95.0f, vec3(0.0f, 0.0f, 1.0f)) * rotate(90.0f, vec3(1.0f, 0.0f, 0.0f)) * rotate(120.0f, vec3(0.0f, 1.0f, 0.0f));
	M = R * S;

	Proy = perspective(radians(55.0f), 4.0f / 3.0f, znear, zfar); 
	View = lookAt(campos, target, up);
	
	transfer_mat4("PV", Proy * View); 
    transfer_mat4("M", M);
	transfer_vec3("luz", light_dir);
	transfer_vec3("campos", campos);
	transfer_vec4("ilu_coef", ilu_coef[1]);
	transfer_float("outter", outter);
	transfer_float("mid", mid);
	transfer_float("lower", lower);
    transfer_float("iTime", tt);

	dibujar_indexado(modelo);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PROGRAMA PRINCIPAL

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	init_GLFW();            // Inicializa lib GLFW
	window = Init_Window(prac);  // Crea ventana usando GLFW, asociada a un contexto OpenGL X.Y
	load_Opengl();         // Carga funciones de OpenGL, comprueba versión.
	init_scene();          // Prepara escena

	while (!glfwWindowShouldClose(window))
	{
		render_scene();  
		glfwSwapBuffers(window); 
        glfwPollEvents();
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

	fps++; 
    tt = glfwGetTime();  // Contador de tiempo en segundos 

	elapsed = (tt - last_tt);
	if (elapsed >= 0.5)  // Refrescar cada 0.5 segundo
	{
		sprintf_s(nombre_ventana, 128, "%s: %4.0f FPS @ %d x %d", prac, fps / elapsed, ANCHO, ALTO);
		glfwSetWindowTitle(window, nombre_ventana);
		last_tt = tt; 
        fps = 0;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////  INTERACCION  TECLADO RATON
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Callback de cambio size
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
				if(prog_selected) prog_selected = 0;
				else prog_selected = 1;
			}
			break;
		case GLFW_KEY_UP: if(action) el += 0.02f; break;
		case GLFW_KEY_DOWN: if(action) el -= 0.02f; break;
		case GLFW_KEY_LEFT: if(action) az += 0.02f; break;
		case GLFW_KEY_RIGHT: if(action) az -= 0.02f; break;

        //Modificacion de variables para zonas de color de la tinta
		case GLFW_KEY_1: if(action) outter -= 0.01f; break;
		case GLFW_KEY_2: if(action) outter += 0.01f; break;
		case GLFW_KEY_3: if(action) mid -= 0.01f; break;
		case GLFW_KEY_4: if(action) mid += 0.01f; break;
		case GLFW_KEY_5: if(action) lower -= 0.01f; break;
		case GLFW_KEY_6: if(action) lower += 0.01f; break;				
	}
    fprintf(stdout,"el %f az %f\n", el, az);
	fprintf(stdout,"outter %f mid %f lower %f\n", outter, mid, lower);
}

double xp, yp;

static void moverCallback(GLFWwindow* window, double x, double y) {
    // Los parametros x, y nos dan la posicion actual del cursor en la ventana.
    fprintf(stdout,"x %.1f y %.1f\n", x, y);
    double F = 0.007f * d;
    double dx = x - xp;
    double dy = y - yp;
    az = F * dx;
    el = F * dy;
}

GLfloat zp;

void pulsarCallback(GLFWwindow* window, int Button, int Action, int Mode) {
    // El parametro Button nos da el boton pulsado (0=izquierdo, 1= derecho). 
    // Action nos indica si se ha pulsado (1) o liberado (0). 
    // Mode si se ha pulsado en combinacion con alguna otra tecla (Shift, Ctrl, etc).
	fprintf(stdout, "Button %d Act %d Mode %d\n", Button, Action, Mode);
    if (!Button && Action) {
        glfwGetCursorPos(window, &xp, &yp);
        glfwSetCursorPosCallback(window, moverCallback);
    }

    if (!Button && !Action) glfwSetCursorPosCallback(window, NULL);

    if (Button && Action) {
        glfwGetCursorPos(window, &xp, &yp);
        glReadPixels((GLint) xp, (GLint) yp, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zp);
        fprintf(stdout, "xp= %.0f yp= %.0f zp= %.3f\n", xp, yp, zp);
        float xn = (2 * (xp / ANCHO)) - 1;
        float yn = (2 * (yp / ALTO)) - 1;
        float zn = 2 * zp - 1;
        fprintf(stdout, "xn= %.0f yn= %.0f zn= %.3f\n", xn, yn, zn);
        float z_cam = (-2 * znear * zfar) / ((zfar + znear) + zn * (znear - zfar));
        float x_cam = -((xn * z_cam) / Proy[0][0]); 
        float y_cam = -((yn * z_cam) / Proy[1][1]);
        fprintf(stdout, "xcam= %.0f ycam= %.0f zcam= %.3f\n", x_cam, y_cam, z_cam);
    }
}

static void scrollCallback(GLFWwindow* window, double dx, double dy) {
    // Los parametros dx, dy nos dan el "scrolling" en los ejes X e Y. 
    // En el caso de usar la rueda del raton dx=0 y solo tenemos "scrolling" vertical.
    fprintf(stdout,"x %.1f y %.1f\n", dx, dy);
    if (dy > 0.0f) d += 0.1f;
    if (dy < 0.0f) d -= 0.1f;
    fprintf(stdout,"x %.1f y %.1f z %.1f\n", campos.x, campos.y, campos.z);
    fprintf(stdout,"d %.1f\n", d);
}

void asigna_funciones_callback(GLFWwindow* window)
{
	glfwSetWindowSizeCallback(window, ResizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, pulsarCallback);
    glfwSetScrollCallback(window, scrollCallback);
}
