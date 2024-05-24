#version 410 core
uniform vec3 luz;
uniform vec3 color;
uniform vec3 color_luz;
uniform vec3 campos;
uniform int pattern;
uniform float factor;
uniform int ANCHO;
uniform int ALTO;
uniform sampler2D unit;

in vec3 COLOR;
in vec3 POS;
in vec3 NORMAL;
in vec2 UV;
in vec3 vertPos;
flat in vec3 startPos;

out vec4 col;

void main(void) {
	vec3 lightDir = normalize(luz-POS);  
	vec3 viewDir = normalize(campos-POS);
	
	//ambient lighting
	float Kamb = 1.0;
	vec3 ambient = Kamb*color_luz;
	
	//diffused lighting
	float Kdiff = 1.0; 
	vec3 norm = normalize(NORMAL);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = Kdiff*diff*color_luz;
	
	//specular lighting
	float Kspec = 1.0f;
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
	vec3 specular = Kspec*spec*color_luz;
	
	float edge_thresh = 0.0;
	float visiblity = dot(viewDir, norm);

	float edge_detection = (visiblity > edge_thresh) ? 0 : 1;
	
	vec3 final_color;
	vec2 dir  = (vertPos.xy-startPos.xy) * vec2(ALTO,ANCHO)/2.0;
	float dist = length(dir);
	uint bit = uint(round(dist / factor)) & 15U;

	vec3 tex_final = texture(unit, UV).rgb;
	if(edge_detection == 0) {
		if ((uint(pattern) & (1U<<bit)) == 0U) {
			final_color = color;
		} else {
			final_color = (ambient+diffuse+specular)*tex_final;
		}
	} else {
		float scale_origin = 0.5;
		float scale = scale_origin+edge_thresh;
		float scale_factor = (visiblity+scale_origin)/scale;
		final_color = scale_factor*ambient*tex_final;
	}
	col = vec4(final_color, 1.0f);
}