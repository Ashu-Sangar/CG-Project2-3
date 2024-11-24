#version 120

varying vec2 texCoord;
varying vec4 color;
varying vec4 N, V, L, H;

uniform sampler2D texture;
uniform int enable_light, ambient_light, diffuse_light, specular_light, no_light;

vec4 ambient, diffuse, specular;

void main()
{
	if(enable_light == 1){
		vec4 the_color = texture2D(texture, texCoord);
		vec4 NN = normalize(N);
		vec4 LL = normalize(L);
		vec4 VV = normalize(V);
		vec4 HH = normalize(H);
		ambient = 0.3 * the_color;
		diffuse = max(dot(LL, NN), 0.0) * the_color;
		specular = pow(max(dot(NN, HH), 0.0), 80) * vec4(1, 1, 1, 1);
		if(ambient_light == 1 && diffuse_light == 0 && specular_light == 0) gl_FragColor = ambient;
		else if(ambient_light == 1 && diffuse_light == 1 && specular_light == 0) gl_FragColor = ambient + diffuse;
		else if(ambient_light == 0 && diffuse_light == 1 && specular_light == 0) gl_FragColor = diffuse;
		else if(ambient_light == 0 && diffuse_light == 1 && specular_light == 1) gl_FragColor = diffuse + specular;
		else if(ambient_light == 1 && diffuse_light == 0 && specular_light == 1) gl_FragColor = ambient + specular;
		else if(ambient_light == 0 && diffuse_light == 0 && specular_light == 1) gl_FragColor = specular;
		else if(ambient_light == 0 && diffuse_light == 0 && specular_light == 0) gl_FragColor = vec4(0, 0, 0, 1);
		else gl_FragColor = ambient + diffuse + specular;
		
		
		//gl_FragColor = ambient + diffuse + specular;
	}
	else{
		gl_FragColor = texture2D(texture, texCoord);
	}
	
}
