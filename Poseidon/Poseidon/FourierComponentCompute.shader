#version 450 core
#define PI 3.1415
#define GRAVITY 9.81

layout(local_size_x = 1, local_size_y = 1) in;


// read textures
layout(binding = 0, rgba32f) readonly uniform image2D tilde_h0k;
layout(binding = 1, rgba32f) readonly uniform image2D tilde_h0minusk;

// write textures
layout(binding = 2, rgba32f)  writeonly uniform image2D fourier_component_dx;
layout(binding = 3, rgba32f)  writeonly uniform image2D fourier_component_dy;
layout(binding = 4, rgba32f)  writeonly uniform image2D fourier_component_dz;

uniform int N;
uniform int L;
uniform float time;

struct ComplexNumber {
	float real;
	float imaginary;
};

ComplexNumber multiply(ComplexNumber c0, ComplexNumber c1) {
	ComplexNumber product;
	product.real = c0.real * c1.real - c0.imaginary * c1.imaginary;
	product.imaginary = c0.real * c1.imaginary + c0.imaginary * c1.real;
	return product;
};

ComplexNumber add(ComplexNumber c0, ComplexNumber c1) {
	ComplexNumber sum;
	sum.real = c0.real + c1.real;
	sum.imaginary = c0.imaginary + c1.imaginary;
	return sum;
}

ComplexNumber conjugate(ComplexNumber c) {
	ComplexNumber conjugate_of_c = ComplexNumber(c.real, -c.imaginary);
	return conjugate_of_c;
}


void main()
{

	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);

	vec2 x = ivec2(gl_GlobalInvocationID.xy) - float(N) / 2.0;
	vec2 k = vec2(2.0 * PI * x.x / L, 2.0 * PI * x.y / L);

	float magnitude_of_k = length(k);
	if (magnitude_of_k < 0.00001) 
	{ 
		magnitude_of_k = 0.00001;
	};

	float w = sqrt(GRAVITY * magnitude_of_k);

	vec2 tilde_h0k_values = imageLoad(tilde_h0k, texel).rg;
	ComplexNumber fourier_component = ComplexNumber(tilde_h0k_values.x, tilde_h0k_values.y);

	vec2 tilde_h0minusk_values = imageLoad(tilde_h0minusk, texel).rg;
	ComplexNumber fourier_component_conj = conjugate(ComplexNumber(tilde_h0minusk_values.x, tilde_h0minusk_values.y));

	float cos_w_t = cos(w * time);
	float sin_w_t = sin(w * time);

	// using euler e^(iwt) = cos(wt) + i*sin(wt)
	ComplexNumber exp_iwt = ComplexNumber(cos_w_t, sin_w_t);
	ComplexNumber exp_iwt_inv = ComplexNumber(cos_w_t, -sin_w_t);

	// find projections of spectrum onto each axis 

	// dy
	ComplexNumber hkt_dy = add(multiply(fourier_component, exp_iwt), multiply(fourier_component_conj, exp_iwt_inv));

	// dx 
	ComplexNumber dx = ComplexNumber(0.0, -k.x / magnitude_of_k);
	ComplexNumber hkt_dx = multiply(dx, hkt_dy);

	// dz 
	ComplexNumber dy = ComplexNumber(0.0, -k.y / magnitude_of_k);
	ComplexNumber hkt_dz = multiply(dy, hkt_dy);

	// store components in textures
	imageStore(fourier_component_dy, texel, vec4(hkt_dy.real, hkt_dy.imaginary, 0, 1));
	imageStore(fourier_component_dx, texel, vec4(hkt_dx.real, hkt_dx.imaginary, 0, 1));
	imageStore(fourier_component_dz, texel, vec4(hkt_dz.real, hkt_dz.imaginary, 0, 1));

}