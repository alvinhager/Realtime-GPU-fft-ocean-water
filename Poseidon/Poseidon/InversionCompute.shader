#version 450 core

layout (local_size_x = 1, local_size_y = 1) in;

layout (binding = 0, rgba32f) uniform image2D displacement;

layout (binding = 1, rgba32f) uniform image2D pingpong_0;
layout (binding = 2, rgba32f) uniform image2D pingpong_1;

uniform int pingpong;
uniform int N;

void main(void)
{
	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
	bool isEven = int(mod((int(texel.x + texel.y)), 2)) == 0;
	float multiplier = isEven ? 1.0 : -1.0;
	
	if(pingpong == 0)
	{
		float h = imageLoad(pingpong_0, texel).r;
		imageStore(displacement, texel, vec4(multiplier * (h / float(N * N)), 
											 multiplier * (h / float(N * N)), 
											 multiplier * (h / float(N * N)), 
											 1));
	}
	else if(pingpong == 1)
	{
		float h = imageLoad(pingpong_1, texel).r;
		imageStore(displacement, texel, vec4(multiplier * (h / float(N * N)), 
											 multiplier * (h / float(N * N)), 
											 multiplier * (h / float(N * N)), 
											 1));
	}
}

