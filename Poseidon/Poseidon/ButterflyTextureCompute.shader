#version 450 core
#define PI 3.1415

layout(local_size_x = 1, local_size_y = 16) in;

// assigns the contents of the reverseIndicesBuffer to an array. binding index = 0, as defined in main
layout(std430, binding = 0)  buffer reverseIndicesBuffer
{
	int reverseIndices[]; 
} bit_reversed;

layout(binding = 1, rgba32f) uniform writeonly image2D butterfly_texture;


struct ComplexNumber
{
	float real;
	float imaginary;
};

uniform int N;

void main()
{
	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
	float k = mod(texel.y * (float(N) / pow(2, texel.x + 1)), N);
	ComplexNumber twiddle = ComplexNumber(cos((2.0 * PI * k) / float(N)), sin((2.0 * PI * k) / float(N)));
	
	int butterflyspan = int(pow(2, texel.x));

	int butterflywing = 0;

	if (mod(texel.y, pow(2, texel.x + 1)) < pow(2, texel.x))
	{
		butterflywing = 1;
	}

	if (texel.x == 0)
	{
		if (butterflywing == 1)
		{
			imageStore(butterfly_texture, texel, vec4(twiddle.real, twiddle.imaginary, bit_reversed.reverseIndices[texel.y], bit_reversed.reverseIndices[texel.y + 1]));
		}
		else
		{
			imageStore(butterfly_texture, texel, vec4(twiddle.real, twiddle.imaginary, bit_reversed.reverseIndices[texel.y - 1], bit_reversed.reverseIndices[texel.y]));
		}
	}
	else
	{
		if (butterflywing == 1)
		{
			imageStore(butterfly_texture, texel, vec4(twiddle.real, twiddle.imaginary, texel.y, texel.y + butterflyspan));
		}
		else
		{
			imageStore(butterfly_texture, texel, vec4(twiddle.real, twiddle.imaginary, texel.y - butterflyspan, texel.y));
		}
	}

}