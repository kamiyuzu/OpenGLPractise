#version 410 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
out vec2 UV;
out vec3 POS;
out vec3 NORMAL;
out vec3 vertPos;
flat out vec3 startPos;
uniform mat4 MVP;
uniform mat4 View;
void main(){
	POS = pos;
	vec4 pos = MVP*vec4(pos.x, pos.y, pos.z, 1.0);
	gl_Position = pos;

	vec4 temp = View*vec4(pos.x, pos.y, pos.z, 1.0f); 
	POS = vec3(temp.x, temp.y, temp.z);
    
	temp = transpose(inverse(View))*vec4(normal, 1.0f);
	NORMAL = vec3(temp.x, temp.y, temp.z);

    vertPos = pos.xyz / pos.w;
    startPos = vertPos;
   
	UV=uv;
}