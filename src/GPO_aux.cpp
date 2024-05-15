

//####include <GpO.h>
#include <GpO.h>
#include <GpOObjectLoader.hpp>

#define STB_IMAGE_IMPLEMENTATION
//####include <stb\stb_image.h>
#include <stb_image.h>

////////////////////   

extern int ANCHO, ALTO;


// Funciones inicialización librerias, ventanas, OpenGL
void init_GLFW(void)
{
	if (!glfwInit())
	{
		fprintf(stdout, "No se inicializo libreria GLFW\n");
		exit(EXIT_FAILURE);
	}
	int Major, Minor, Rev;
	glfwGetVersion(&Major, &Minor, &Rev);
	printf("Libreria GLFW (ver. %d.%d.%d) inicializada\n", Major, Minor, Rev);
}


GLFWwindow*  Init_Window(const char* nombre)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(ANCHO, ALTO, nombre, NULL, NULL);
	if (window == NULL)
	{
		fprintf(stdout, "Fallo al crear ventana GLFW con OpenGL context %d.%d\n", OPENGL_MAJOR, OPENGL_MINOR);
		glfwTerminate();
		exit(EXIT_FAILURE);
		return NULL;
	}
	
	fprintf(stdout, "Ventana GLFW creada con contexto OpenGL %d.%d\n", OPENGL_MAJOR, OPENGL_MINOR);
	glfwMakeContextCurrent(window);
		
	asigna_funciones_callback(window);

	return window;
}


void load_Opengl(void)
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stdout, "Fallo al cargar funciones de OpenGL con GLAD\n");
		exit(EXIT_FAILURE);
	}	
	fprintf(stdout, "OpenGL Version: %s\n",glGetString(GL_VERSION));
	glViewport(0, 0, ANCHO, ALTO);

	printf("---------------------------------------------\n");
	return;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CARGA, COMPILACION Y LINKADO DE LOS SHADERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* leer_codigo_de_fichero(const char* fich)
{
	FILE* fid;
	fopen_s(&fid, fich, "rb");  if (fid == NULL) return NULL;

	fseek(fid, 0, SEEK_END);  long nbytes = ftell(fid);
	fprintf(stdout, "Leyendo codigo de %s (%ld bytes)\n", fich, nbytes);

	char* buf = new char[nbytes + 1];
	fseek(fid, 0, SEEK_SET);
	fread(buf, 1, nbytes, fid);
	buf[nbytes] = 0;
	fclose(fid);

//	for (int k = 0; k < nbytes; k++) fprintf(stdout,"%c", buf[k]);
//	fprintf(stdout, "\n ------------------------\n");

	return buf;
}

GLuint compilar_shader(const char* Shader_source, GLuint type)
{
	//printf("--------------------------------\n");
	switch (type)
	{
	case GL_VERTEX_SHADER: printf("Compilando Vertex Shader :: "); break;
	case GL_FRAGMENT_SHADER: printf("Compilando Fragment Shader :: "); break;
	case GL_GEOMETRY_SHADER: printf("Compilando Geometric Shader :: "); break;
	}
		
  GLuint ShaderID = glCreateShader(type);  // Create shader object

  glShaderSource(ShaderID, 1, &Shader_source , NULL);    // Compile Shader
  glCompileShader(ShaderID);

  GLint Result = GL_FALSE;
  int InfoLogLength; char error[512]; 

  glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
  if (Result==GL_TRUE) fprintf(stdout,"Sin errores\n");  
  else 
  {
   fprintf(stdout,"ERRORES\n");  
   glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
   if(InfoLogLength>512) InfoLogLength=512;
   glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, error);
   fprintf(stdout,"\n%s\n", error);

  }
 printf("----------------------------------------------\n");
 return ShaderID;
}

void check_errores_programa(GLuint id)
{
 GLint Result = GL_FALSE;
 int InfoLogLength;
 char error[512]; 

 printf("Resultados del linker (GPU): ");
 glGetProgramiv(id, GL_LINK_STATUS, &Result);
 if (Result==GL_TRUE) fprintf(stdout,"Sin errores\n"); // Compiled OK
 else 
	{
     fprintf(stdout,"ERRORES\n");  
     glGetProgramiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	 if(InfoLogLength<1) InfoLogLength=1; if(InfoLogLength>512) InfoLogLength=512;
     glGetProgramInfoLog(id, InfoLogLength, NULL, error);
     fprintf(stdout, "\n%s\n",error);
	 glfwTerminate();
	}
 printf("---------------------------------------------\n");
}


GLuint Compile_Link_Shaders(const char* vertexShader_source,const char*fragmentShader_source)
{
	// Compile Shaders
	GLuint VertexShaderID = compilar_shader(vertexShader_source, GL_VERTEX_SHADER);
	GLuint FragmentShaderID = compilar_shader(fragmentShader_source,GL_FRAGMENT_SHADER);

    // Link the shaders in the final program
 

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glLinkProgram(ProgramID);
	check_errores_programa(ProgramID);

	// Limpieza final
	glDetachShader(ProgramID, VertexShaderID);  glDeleteShader(VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);  glDeleteShader(FragmentShaderID);

	

    return ProgramID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////  AUXILIARES 
/////////////////////////////////////////////////////////////////////////////////////////////////////////

GLuint cargar_textura(const char * imagepath, GLuint tex_unit)
{
  stbi_set_flip_vertically_on_load(true);

  int width, height,nrChannels;
  unsigned char* data = stbi_load(imagepath, &width, &height,&nrChannels,0);

  if (data == NULL)
  {
	  fprintf(stdout, "Error al cargar imagen: existe el fichero %s?\n",imagepath);
	  glfwTerminate();
	  return 0;
  }

  glActiveTexture(tex_unit);   //glBindTexture(GL_TEXTURE_2D, 0); 
	
	GLuint textureID;
	glGenTextures(1, &textureID);             // Crear objeto textura
	glBindTexture(GL_TEXTURE_2D, textureID);  // "Bind" la textura creada
	
	//printf("%d %d data %8X\n", width, height, data);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);  //Pasa datos a GPU

	glGenerateMipmap(GL_TEXTURE_2D);
	
	stbi_image_free(data); 

	// Opciones de muestreo, magnificación, coordenadas fuera del borde, etc.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	

	//glBindTexture(GL_TEXTURE_2D, 0); 
	// DEvolvemos ID de la textura creada y cargada con la imagen
	return textureID;
}



GLuint cargar_cube_map(const char * imagepath, GLuint tex_unit)
{
	GLuint textureID;
	char fich[128];
	//const char* suf[6];
	//suf[0] = "posx"; suf[1] = "negx";
	//suf[2] = "posy"; suf[3] = "negy";
	//suf[4] = "posz"; suf[5] = "negz";

	char suf[6][5] = { "posx", "negx", "posy", "negy", "posz", "negz" };


	glActiveTexture(tex_unit);	//glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &textureID); glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	
	unsigned char *data;  int width, height, nrChannels;

	//stbi_set_flip_vertically_on_load(true);  // AQUI PARECE QUE NO ES NECESARIO PERO PUEDE QUE OCASIONALMENTE SI

	printf("CARGANDO CUBE_MAP de %s_xxx.jpg\n", imagepath);
	for (int k = 0; k < 6; k++)
	{
	 sprintf_s(fich, 128, "%s_%s.jpg", imagepath, suf[k]); //printf("CUBE_MAP: %s\n", fich);
	 data = stbi_load(fich, &width, &height, &nrChannels, 0);
	 if (data == NULL)
	 {
		 fprintf(stdout, "Error al cargar imagen: existe el fichero %s?\n", fich);
		 stbi_image_free(data);
		 glfwTerminate();
		 return 0;
	 }
	 glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+k, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	 stbi_image_free(data);
	}

	/*sprintf_s(fich,128,"%s_%s.jpg", imagepath, "posx"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	sprintf_s(fich, 128, "%s_%s.jpg", imagepath, "negx"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	sprintf_s(fich, 128, "%s_%s.jpg", imagepath, "posy"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	sprintf_s(fich, 128, "%s_%s.jpg", imagepath, "negy"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	sprintf_s(fich, 128, "%s_%s.jpg", imagepath, "posz"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	sprintf_s(fich, 128, "%s_%s.jpg", imagepath, "negz"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);*/

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



	return textureID;
}

objeto cargar_modelo(char* fichero)
{
	objeto obj;
	GLuint VAO;

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals; // Won't be used at the moment.
	bool res = loadOBJ(fichero, vertices, uvs, normals);

	if (res==false) { 
		 printf("Error al leer datos. Existe el fichero %s?\n",fichero); 
		 glfwTerminate();
	     return obj;
	}

    glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	glBindVertexArray(0);  //Cerramos Vertex Array con todo lidto para ser pintado
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	obj.vertices=vertices;
	obj.VAO=VAO; 

	return obj;

}



void transfer_mat4(const char* name, mat4 M)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);

 loc=glGetUniformLocation(prog,name); 
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
	 //return NULL;
 }
 else glUniformMatrix4fv(loc, 1, GL_FALSE, &M[0][0]);
}

void transfer_mat3(const char* name, mat3 M)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);

 loc=glGetUniformLocation(prog,name); 
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniformMatrix3fv(loc, 1, GL_FALSE, &M[0][0]);
}


void transfer_vec4(const char* name, vec4 x)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniform4fv(loc, 1, &x[0]);
}

void transfer_vec3(const char* name, vec3 x)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniform3fv(loc, 1, &x[0]);
}

void transfer_vec2(const char* name, vec2 x)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniform2fv(loc, 1, &x[0]);
}

void transfer_int(const char* name, GLuint valor)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniform1i(loc,valor);
}


void transfer_float(const char* name, GLfloat valor)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniform1f(loc, valor);
}


void vuelca_mat4(glm::mat4 M)
{
	int j, k;
	printf("--------------------------------------\n");
	for (k = 0; k<4; k++) { for (j = 0; j<4; j++) printf("%6.3f ", M[j][k]); printf("\n"); }
	printf("--------------------------------------\n");
}

