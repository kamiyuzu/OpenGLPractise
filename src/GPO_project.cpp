/************************  GPO Iluminacion  **********************************
ATG, 2020
******************************************************************************/

#include <GpO.h>
#include <GpOObjectLoader.hpp>

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL(GpO) Iluminacion";   // Nombre de la practica (aparecera en el titulo de la ventana).


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 410 core\n" #src

// const char* vertex_prog = leer_codigo_de_fichero("../data/prog.vs");
// const char* fragment_prog = leer_codigo_de_fichero("../data/prog.fs");

const char* vertex_prog1 = GLSL(
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
out vec2 UV;
out vec3 POS;
out vec3 NORMAL;
out vec3 COLOR;
uniform mat4 MVP;
uniform mat4 View;
void main(){
	gl_Position = MVP*vec4(pos.x*0.25, pos.y*0.25, pos.z*0.25, 1.0);

	vec4 temp= View*vec4(pos, 1.0f); 
	POS= vec3(temp.x, temp.y, temp.z);
    
	temp= transpose(inverse(View))*vec4(normal, 1.0f);
	NORMAL= vec3(temp.x, temp.y, temp.z);
   
    COLOR = vec3(0.5,0.5,0.5);
	UV=uv;
}
);

const char* fragment_prog1 = GLSL(
uniform vec3 luz;
uniform vec3 color_luz;
uniform vec3 campos;

uniform sampler2D unit;

in vec3 COLOR;
in vec3 POS;
in vec3 NORMAL;
in vec2 UV;
out vec4 col;

void main(void) {
	vec3 lightDir= normalize(luz-POS);  
	vec3 viewDir= normalize(campos-POS);
	
	//ambient lighting
	float Kamb= 1.0;
	vec3 ambient= Kamb*color_luz;
	
	//diffused lighting
	float Kdiff= 1.0; 
	vec3 norm= normalize(NORMAL);
	float diff= max(dot(norm, lightDir), 0.0); 	//to clamp between 0 and 1
	vec3 diffuse= Kdiff*diff*color_luz;  	//light intensity * cos
	
	//specular lighting
	float Kspec = 1.0f;
	vec3 reflectDir= reflect(-lightDir, norm);
	float spec= pow(max(dot(viewDir, reflectDir), 0.0), 16);
	vec3 specular= Kspec*spec*color_luz;
	
	float edge_thresh=0.0;
	float visiblity=dot(viewDir, norm);

	float edge_detection = (visiblity > edge_thresh) ? 0 : 1;
	
	vec3 tex_final= texture(unit, UV).rgb;
	vec3 final_color;
	if(edge_detection ==0){
		final_color= (ambient+diffuse+specular)*COLOR;
	}else{
		float scale_origin=0.5;
		float scale=scale_origin+edge_thresh;
		float factor= (visiblity+scale_origin)/scale;
		final_color= factor*ambient*COLOR;
	}
	col= vec4(final_color, 1.0f);
}
);

//////////////////////////////////////////////////////////////////////////////////


// //  PROGRAMA 2 (aqui implementaremos version en fragmentos)
// const char* vertex_prog2 = GLSL(
// layout(location = 0) in vec3 pos;
// layout(location = 1) in vec3 normal;
// out vec3 n;
// out vec3 v;

// uniform vec3 campos;
// uniform mat4 M;
// uniform mat4 PV;

// void main() {
// 	gl_Position = PV*M*vec4(pos, 1);

// 	vec3 scene_vertex_position = vec3(M*vec4(pos, 1));
// 	v = scene_vertex_position - campos; // unit_vertex_to_cam

// 	mat3 M_adj = mat3(transpose(inverse(M)));
// 	n = M_adj * normal;
// }
// );

// const char* fragment_prog2 = GLSL(
// in vec3 v;
// in vec3 n;
// uniform vec3 luz = vec3(1, 1, 0) / sqrt(2.0f);
// out vec3 col;  // Color fragmento
// void main()
// {
// 	vec3 v_n = normalize(v); // unit_vertex_to_cam
// 	vec3 nn = normalize(n);
// 	vec3 r = reflect(-luz,nn); // perfect_reflexion

// 	float difusa = dot(luz,nn); if (difusa < 0) difusa = 0; 
// 	float prod_esc_vertex_perf_reflex = dot(r,v_n); if (prod_esc_vertex_perf_reflex < 0) prod_esc_vertex_perf_reflex = 0; 
// 	float especular = pow(prod_esc_vertex_perf_reflex,16);
// 	float ilu = (0.1 + 0.6*difusa + 0.3*especular);  //10% Ambiente + 60% difusa + 30 especular

// 	col = vec3(1, 1, 0.9);
// 	col = col*ilu;
// }
// );

////////////////////////////////  FIN PROGRAMAS GPU (SHADERS) //////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
///////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog[2];
obj modelo;


// Dibuja obj indexado
void dibujar_indexado(obj obj)
{
  glBindVertexArray(obj.VAO);              // Activamos VAO asociado al obj
  glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
  glBindVertexArray(0);
}


// Variables globales
float az=0.0f, el=0.75f, d = 2.0f, zfar = 25.0f, znear = 0.2f;
mat4 Proy,View,M, R, T, S;
vec3 campos=vec3(0.0f,0.0f,d);											
vec3 target=vec3(0.0f,0.0f,0.0f);
vec3 up = vec3(0.0f, 1.0f, 0.0f);
int prog_selected = 0;

// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Preparaci�n de los datos de los objs a dibujar, envialarlos a la GPU
// Opciones generales de render de OpenGL
void init_scene()
{
	prog[0] = Compile_Link_Shaders(vertex_prog1, fragment_prog1); // Compile shaders prog1
	// prog[1] = Compile_Link_Shaders(vertex_prog2, fragment_prog2); // Compile shaders prog2
	
	glUseProgram(prog[prog_selected]);

	modelo = cargar_obj((char*) "bin/data/melinoeweapons.obj");
	GLuint tex0 = cargar_textura("bin/data/melinoe.jpg", GL_TEXTURE0);

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
	vec3 xy=vec3(cos(tt), 1.0f, sin(tt));
	S = scale(mat4(1.0f),vec3(2.0f));
    R = rotate(90.0f, vec3(0.0f, 0.0f, 1.0f))*rotate(30.0f, vec3(1.0f, 0.0f, 0.0f))*rotate(120.0f, vec3(0.0f, 1.0f, 0.0f));
    // T = translate(glm::vec3(2.5*cos(t), 2.5*sin(t), 0));   	
	M = R * S;
	// M = rotate(90.0f, vec3(0.0f, 0.0f, 1.0f))*rotate(90.0f, vec3(0.0f, 1.0f, 0.0f));   // Mov modelo 
	
	transfer_mat4("MVP", Proy*View*M);
	transfer_mat4("View", View);
	transfer_vec3("luz", light_dir);
	transfer_vec3("color_luz", vec3(1.0f, 1.0f, 1.0f));
	transfer_vec3("campos", campos);
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
		case GLFW_KEY_UP: if(action && el<=M_PI/2) el+=0.02f; break;
		case GLFW_KEY_DOWN: if(action && el>=-M_PI/2) el-=0.02f; break;
		case GLFW_KEY_LEFT: if(action) az+=0.02f; break;
		case GLFW_KEY_RIGHT: if(action) az-=0.02f; break;
	}
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



 