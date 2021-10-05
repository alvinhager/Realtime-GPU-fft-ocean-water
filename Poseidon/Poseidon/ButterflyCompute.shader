#version 450 core
#define PI 3.1415

layout(local_size_x = 1, local_size_y = 1) in;

layout(binding = 0, rgba32f) readonly uniform image2D butterfly_texture;
layout(binding = 1, rgba32f) uniform image2D pingpong_0;
layout(binding = 2, rgba32f) uniform image2D pingpong_1;

uniform int stage;
uniform int pingpong_index;
uniform int direction;

struct ComplexNumber 
{
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

void horizontalButterflies()
{
	ComplexNumber H;
	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);

	if (pingpong_index == 0)
	{
		vec4 data = imageLoad(butterfly_texture, ivec2(stage, texel.x)).rgba;
		vec2 p_ = imageLoad(pingpong_0, ivec2(data.z, texel.y)).rg;
		vec2 q_ = imageLoad(pingpong_0, ivec2(data.w, texel.y)).rg;
		vec2 w_ = vec2(data.x, data.y);

		ComplexNumber p = ComplexNumber(p_.x, p_.y);
		ComplexNumber q = ComplexNumber(q_.x, q_.y);
		ComplexNumber w = ComplexNumber(w_.x, w_.y);

		H = add(p, multiply(w, q));

		imageStore(pingpong_1, texel, vec4(H.real, H.imaginary, 0, 1));
	}
	else if (pingpong_index == 1)
	{
		vec4 data = imageLoad(butterfly_texture, ivec2(stage, texel.x)).rgba;
		vec2 p_ = imageLoad(pingpong_1, ivec2(data.z, texel.y)).rg;
		vec2 q_ = imageLoad(pingpong_1, ivec2(data.w, texel.y)).rg;
		vec2 w_ = vec2(data.x, data.y);

		ComplexNumber p = ComplexNumber(p_.x, p_.y);
		ComplexNumber q = ComplexNumber(q_.x, q_.y);
		ComplexNumber w = ComplexNumber(w_.x, w_.y);

		H = add(p, multiply(w, q));

		imageStore(pingpong_0, texel, vec4(H.real, H.imaginary, 0, 1));
	}
}

void verticalButterflies()
{
	ComplexNumber H;
	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);

	if (pingpong_index == 0)
	{
		vec4 data = imageLoad(butterfly_texture, ivec2(stage, texel.y)).rgba;
		vec2 p_ = imageLoad(pingpong_0, ivec2(texel.x, data.z)).rg;
		vec2 q_ = imageLoad(pingpong_0, ivec2(texel.x, data.w)).rg;
		vec2 w_ = vec2(data.x, data.y);

		ComplexNumber p = ComplexNumber(p_.x, p_.y);
		ComplexNumber q = ComplexNumber(q_.x, q_.y);
		ComplexNumber w = ComplexNumber(w_.x, w_.y);

		H = add(p, multiply(w, q));

		imageStore(pingpong_1, texel, vec4(H.real, H.imaginary, 0, 1));
	}
	else if (pingpong_index == 1)
	{
		vec4 data = imageLoad(butterfly_texture, ivec2(stage, texel.y)).rgba;
		vec2 p_ = imageLoad(pingpong_1, ivec2(texel.x, data.z)).rg;
		vec2 q_ = imageLoad(pingpong_1, ivec2(texel.x, data.w)).rg;
		vec2 w_ = vec2(data.x, data.y);

		ComplexNumber p = ComplexNumber(p_.x, p_.y);
		ComplexNumber q = ComplexNumber(q_.x, q_.y);
		ComplexNumber w = ComplexNumber(w_.x, w_.y);

		H = add(p, multiply(w, q));

		imageStore(pingpong_0, texel, vec4(H.real, H.imaginary, 0, 1));
	}
}

void main() {

	if (direction == 0)
	{
		horizontalButterflies();
	}
	else if (direction == 1)
	{
		verticalButterflies();
	}
}