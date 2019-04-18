#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float colorTime;
uniform int collected;
out vec3 fragNor;
out float colorScaleR;
out float colorScaleB;
out float colorScaleG;

void main()
{
	gl_Position = P * V * M * vertPos;
	fragNor = (M * vec4(vertNor, 0.0)).xyz;
	if (collected == 1)
	{
		colorScaleR = colorTime + 0.2;
		colorScaleB = (colorTime + 0.5) / 2.0 + 0.2;
		colorScaleG = (colorTime * 2.0) - 0.5 + 0.2;
	}
	else
	{
		colorScaleR = colorScaleB = colorScaleG = 1;
	}
}
