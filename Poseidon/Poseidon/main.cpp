#define WIN32_LEAN_AND_MEAN
#define STB_IMAGE_IMPLEMENTATION
#define INT_SIZE 32

#include <iostream>

#include "OpenGLImports.h"

#include "ShaderProgram.h"
#include "Texture.h"

#include "Debug.h"

// --------------------------------------------------------
// FUNCTION DECLARATIONS

int main(void);

// callback functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// setup functions
void setUpLibraries();
void setCallbackFunctions();
void init_VAO_and_shaders();
void init_textures();

// fft functions
void create_h0k_h0minusk_textures();
void create_butterfly_texture();
void create_fourier_components();
void fft();
void inversion();

// loop functions
void render();

// help functions 
unsigned int reverseBits(unsigned int n);

// cleanup functions
void cleanUp();

// --------------------------------------------------------
// USER DEFINED VARIABLES

// window settings
GLFWwindow* window = nullptr;
const int window_width = 800;
const int window_height = 800;
const char* window_title = "Poseidon";

// fft ocean parameters
float A = 10;
int L = 1000;
glm::vec2 windDirection = glm::vec2(1.0f, 1.0f);
float windSpeed = 40;
bool choppy = true;

// width and height of grid
const int N = 256;
const int M = 256;

// --------------------------------------------------------
// GLOBAL VARIABLES

// shaders
ShaderProgram programTildeHCompute;
ShaderProgram programRender;
ShaderProgram programButterflyTextureCompute;
ShaderProgram programFourierComponentCompute;
ShaderProgram programButterflyCompute;
ShaderProgram programInversionCompute;
unsigned int VAO, VBO;


//textures
Texture texture_random_noise_1;
Texture texture_random_noise_2;
Texture texture_random_noise_3;
Texture texture_random_noise_4;

Texture texture_tilde_h0k;
Texture texture_tilde_h0minusk;

Texture texture_butterfly;

Texture texture_fourier_component_dx;
Texture texture_fourier_component_dy;
Texture texture_fourier_component_dz;

Texture texture_pingpong_0;
Texture texture_pingpong_1;

Texture texture_displacement_of_points_on_grid;

// time
float lastRefreshTime = 0.0f;
float currTime = glfwGetTime();
float t = 0.0f;

// misc
int* bitReversedIndices;
int pingpong_index = 0;
const int log_2_N = (int)(log(N) / log(2));
char pressed = ' ';

// --------------------------------------------------------
// STRUCTS
struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoord;
};

// --------------------------------------------------------
// MAIN
int main(void)
{
    setUpLibraries();
    setCallbackFunctions();

    init_VAO_and_shaders();
    init_textures();

    create_h0k_h0minusk_textures();
    create_butterfly_texture();
    create_fourier_components();
    fft();
    inversion();
    

    // CONNECT TO FRAGMENT SHADER
    // bind the resulting textures to fragment shader
    glBindTextureUnit(0, texture_tilde_h0k.getID());
    glBindTextureUnit(1, texture_tilde_h0minusk.getID());

    // bind resulting butterfly texture to fragment shader
    glBindTextureUnit(2, texture_butterfly.getID());

    // bind resulting dx, dy, dz fourier component textures  to fragment shader
    glBindTextureUnit(3, texture_fourier_component_dx.getID());
    glBindTextureUnit(4, texture_fourier_component_dy.getID());
    glBindTextureUnit(5, texture_fourier_component_dz.getID());

    // bind ping pong textures to fragment shader
    glBindTextureUnit(6, texture_displacement_of_points_on_grid.getID());
    glBindTextureUnit(7, texture_pingpong_1.getID());

    //render loop
    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_BLEND);

        currTime = glfwGetTime();
        if (currTime - lastRefreshTime > 1 / 30)
        {
            t += 0.25;
            create_fourier_components();
            fft();
            inversion();
            lastRefreshTime = currTime;
        }

        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUp();

    glfwTerminate();
    return 0;
}

// initializes vertex buffers and all shaderprograms
void init_VAO_and_shaders()
{
    // create shaders
    programRender = ShaderProgram("VertexShader.shader", "FragmentShader.shader");
    programTildeHCompute = ShaderProgram("TildeHCompute.shader");
    programButterflyTextureCompute = ShaderProgram("ButterflyTextureCompute.shader");
    programFourierComponentCompute = ShaderProgram("FourierComponentCompute.shader");
    programButterflyCompute = ShaderProgram("ButterflyCompute.shader");
    programInversionCompute = ShaderProgram("InversionCompute.shader");

    // create vertex objects
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // setup vertex array
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (void*)(sizeof(glm::vec3)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // setup vertex buffer ( mapping from coordinates (x,y) to texture coord (u,v)
    Vertex vertices[] = {
        //    x           y       z      u  v
        { { -1.0f,    -1.0f,    0 },{ 0, 0 } }, // bottom-left
        { { +1.0f,    -1.0f,    0 },{ 1, 0 } }, // bottom-right
        { { +1.0f,    +1.0f,    0 },{ 1, 1 } }, // top-right
        { { -1.0f,    +1.0f,    0 },{ 0, 1 } }, // top-left
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

// initializes all used textures
void init_textures()
{
    texture_tilde_h0k = Texture(false, N, N);
    texture_tilde_h0minusk = Texture(false, N, N);
    texture_random_noise_1 = Texture(true, N, N);
    texture_random_noise_2 = Texture(true, N, N);
    texture_random_noise_3 = Texture(true, N, N);
    texture_random_noise_4 = Texture(true, N, N);

    texture_butterfly = Texture(false, log_2_N, N);

    texture_fourier_component_dx = Texture(false, N, N);
    texture_fourier_component_dy = Texture(false, N, N);
    texture_fourier_component_dz = Texture(false, N, N);

    texture_pingpong_0 = Texture(false, N, N);
    texture_pingpong_1 = Texture(false, N, N);

    texture_displacement_of_points_on_grid = Texture(false, N, N);

}

// create tilde h0k and tilde h0minusk textures
void create_h0k_h0minusk_textures() {

    // bind image units of tilde_h0k and h0minusk
    glBindImageTexture(0, texture_tilde_h0k.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texture_tilde_h0minusk.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    // bind image units of noise textures 
    glBindImageTexture(2, texture_random_noise_1.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(3, texture_random_noise_2.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(4, texture_random_noise_3.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(5, texture_random_noise_4.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);

    // set uniform variables of the shader
    programTildeHCompute.SetUniform1i("N", N);
    programTildeHCompute.SetUniform1i("L", L);
    programTildeHCompute.SetUniform1f("A", A);
    programTildeHCompute.SetUniform1f("windSpeed", windSpeed);
    programTildeHCompute.SetUniform1fv("windDirection", windDirection);
    programTildeHCompute.SetUniform1i("choppy", choppy);

    // run the tildeHCompute shader to write to textures
    programTildeHCompute.compute(N, N);

}

// create butterfly texture and reversed bit indices
void create_butterfly_texture() {

    //generate reversed bit indices
    bitReversedIndices = new int[N];
    int bits = (log(N) / log(2));
    for (int i = 0; i < N; i++)
    {
        unsigned int x = reverseBits(i);
        x = (x << bits) | (x >> (INT_SIZE - bits));
        bitReversedIndices[i] = x;
    }

    // create the shader storage buffer that passes bitReversedIndices
    unsigned int reverseIndicesSSBO;
    glGenBuffers(1, &reverseIndicesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, reverseIndicesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * N, bitReversedIndices, GL_STATIC_DRAW);
    delete[] bitReversedIndices;
    // buffer assigned to binding index 0
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, reverseIndicesSSBO);
    // unbind buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // bind output texture we are writing to 
    glBindImageTexture(1, texture_butterfly.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    programButterflyTextureCompute.SetUniform1i("N",N);

    // run the butterfly compute shader to write to butterfly texture
    programButterflyTextureCompute.compute(N, N);
}

// create fourier components dx, dy and dz textures
void create_fourier_components()
{
    // bind the h0k and h0minusk to image unit 0 and 1
    glBindImageTexture(0, texture_tilde_h0k.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texture_tilde_h0minusk.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);

    // bind image units used in fourier component compute shader to dx dy dz write textures
    glBindImageTexture(2, texture_fourier_component_dx.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(3, texture_fourier_component_dy.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(4, texture_fourier_component_dz.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // set uniform variables of the shader
    programFourierComponentCompute.SetUniform1f("time", t);
    programFourierComponentCompute.SetUniform1i("N", N);
    programFourierComponentCompute.SetUniform1i("L", L);

    // run the shader to write to dx, dy, dz textures
    programFourierComponentCompute.compute(N, N);
}

// butterfly fourier compututation
void fft()
{
    // bind image units used in butterfly texture compute shader
    glBindImageTexture(0, texture_butterfly.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texture_fourier_component_dy.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, texture_pingpong_1.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // one dimensional FFT in horizontal direction 
    for (int stage = 0; stage < log_2_N; stage++)
    {
        programButterflyCompute.updateButterflyComputeUniforms(pingpong_index, 0, stage);
        programButterflyCompute.compute(N, N);
        glFinish();
        pingpong_index = !pingpong_index;
    }

    // one dimensional FFT in vertical direction   
    for (int stage = 0; stage < log_2_N; stage++)
    {
        programButterflyCompute.updateButterflyComputeUniforms(pingpong_index, 1, stage);
        programButterflyCompute.compute(N, N);
        glFinish();
        pingpong_index = !pingpong_index;
    }
}

// inversion and permutation compute shader
void inversion()
{
    // bind image units used in inversion compute shader, texture pinpong1 is an empty texture, the dy component serves as texture pingpong0
    glBindImageTexture(0, texture_displacement_of_points_on_grid.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texture_fourier_component_dy.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(2, texture_pingpong_1.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);

    // set uniform variables of the shader
    programInversionCompute.SetUniform1i("pingpong", pingpong_index);
    programInversionCompute.SetUniform1i("N", N);

    // run the shader to write to the pingpong textures
    programInversionCompute.compute(N, N);
}

// set up OpenGL, GLFW, GLEW
void setUpLibraries()
{
    // initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
    }

    // create window
    window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
    }

    // set OpenGL version 4.5
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwMakeContextCurrent(window); 

    // initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
    }
}

// setting calback functions
void setCallbackFunctions(void) {
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

// runs renderProgram clears buffer
void render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // render quad
    programRender.bind();
    glBindVertexArray(VAO);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);
    programRender.unbind();
}

// calls all destructors
void cleanUp()
{
    // delete all objects
    glDeleteProgram(programTildeHCompute.getID());
    glDeleteProgram(programRender.getID());
    glDeleteProgram(programButterflyTextureCompute.getID());
    glDeleteProgram(programFourierComponentCompute.getID());
    glDeleteProgram(programButterflyCompute.getID());
    glDeleteProgram(programInversionCompute.getID());

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    texture_random_noise_1.deleteTexture();
    texture_random_noise_2.deleteTexture();
    texture_random_noise_3.deleteTexture();
    texture_random_noise_4.deleteTexture();

    texture_tilde_h0k.deleteTexture();
    texture_tilde_h0minusk.deleteTexture();
    texture_butterfly.deleteTexture();

    texture_pingpong_0.deleteTexture();
    texture_pingpong_1.deleteTexture();

    texture_displacement_of_points_on_grid.deleteTexture();
}

// is called when keys are pressed
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_RELEASE) 
    { 
        return; 
    }

    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        break;
    case GLFW_KEY_C:
        choppy = !choppy;
        break;
    case GLFW_KEY_A:                // amplitude
        pressed = 'A';
        break;
    case GLFW_KEY_L:                // L
        pressed = 'L';
        break;
    case GLFW_KEY_S:                // windspeed
        pressed = 'S';
        break;


    case GLFW_KEY_RIGHT_BRACKET:    // german keyboard: +
        switch (pressed)
        {
        case 'A':
            A++;
            break;
        case 'L':
            L+=10;
            break;
        case 'S':
            windSpeed++;
            break;
        }
        break;

    case GLFW_KEY_SLASH:            // german keyboard: -
        switch (pressed)
        {
        case 'A':
            A--;
            break;
        case 'L':
            L-=10;
            break;
        case 'S':
            windSpeed--;
            break;
        }
        break;
    }

}

// is call when one scrolls
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){}

// reverses the bits of a given unsigned int 
unsigned int reverseBits(unsigned int num)
{
    unsigned int  NO_OF_BITS = sizeof(num) * 8;
    unsigned int reverse_num = 0;
    int i;
    for (i = 0; i < NO_OF_BITS; i++)
    {
        if ((num & (1 << i)))
            reverse_num |= 1 << ((NO_OF_BITS - 1) - i);
    }
    return reverse_num;
}