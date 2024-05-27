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
	vec3 ambient = color_luz;
	
	//diffused lighting
	vec3 norm = normalize(NORMAL);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff*color_luz;
	
	//specular lighting
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
	vec3 specular = spec*color_luz;
	
	float edge_thresh = 0.2;
	float visiblity = dot(viewDir, norm);

	float edge_detection = (visiblity > edge_thresh) ? 0 : 1;
	
	vec3 final_color;
	vec2 dir = (vertPos.xy-startPos.xy) * vec2(ANCHO,ALTO)/2.0;
	float dist = length(dir);
	uint bit = uint(round(dist / factor)) & 15U;

	vec3 tex_final = texture(unit, UV).rgb;
	if(edge_detection == 0) {
		// if ((uint(pattern) & (1U<<bit)) == 0U) {
		// 	final_color = (ambient+diffuse+specular)*tex_final;
		// } else {
		// 	final_color = (ambient+diffuse+specular)*color;
		// }
		final_color = (ambient+diffuse+specular)*tex_final;
	} else {
		float scale_origin = 0.8;
		float scale = scale_origin+edge_thresh;
		float scale_factor = (visiblity+scale_origin)/scale;
		if ((uint(pattern) & (1U<<bit)) == 0U) {
			final_color = tex_final;
		} else {
			final_color = scale_factor*ambient*vec3(0,0,0);
		}
	}
	col = vec4(final_color, 1.0f);
}