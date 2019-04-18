#version 330 core 
in vec3 fragNor;
in float colorScaleR;
in float colorScaleB;
in float colorScaleG;
out vec4 color;

void main()
{
	vec3 normal = normalize(fragNor);
	// Map normal in the range [-1, 1] to color in range [0, 1];
	vec3 Ncolor = 0.5*normal + 0.5;
	Ncolor.r *= colorScaleR;
	Ncolor.b *= colorScaleB;
	Ncolor.g *= colorScaleG;
	color = vec4(Ncolor, 1.0);
}
