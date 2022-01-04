//OpenGL libraries
#include "glad.h"
#include <GLFW/glfw3.h>

//Used for debug/output
#include <iostream>

//Used for reading vertex and fragment shaders
#include <fstream>
#include <streambuf>
#include <cstring>

//Used for loading 1D gradient images as arrays
//and then later using them for coloring the fractal
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cmath>

int screenWidth = 1000;
int screenHeight = 1000;

//Graph area displayed by the program
double minX = -2.5, minY = -2.0; //lower-left corner
double maxX = 1.5, maxY = 2.0; //upper-right corner

//Variable number of iterations
int maxIterations = 430;

//Check if mouse left button is pressed
//Used for panning

//Gradient

std::string readSource(const char* filename) {
  std::ifstream source(filename);
  std::string str((std::istreambuf_iterator<char>(source)),
                  std::istreambuf_iterator<char>());
  return str;
}

void resizeMandel(const int width, const int height) {
  maxY = minY + (maxX - minX) * (float)height / (float)width;
  screenWidth = width, screenHeight = height;
}

//Resize window callback function
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  //Update displayed area (both screen size and graph area)
  resizeMandel(width, height);
  glViewport(0, 0, width, height);
}

//Maximaze window callback function
void window_maximize_callback(GLFWwindow* window, int maximized) {
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  resizeMandel(width, height);
}

static void cursor_position_callback(GLFWwindow* window, double new_x, double new_y) {
  static double x = -1, y =- 1;
  if (x != -1 && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    double xdiff = maxX - minX;
    double ydiff = maxY - minY;
    double xdelta = (x - new_x) / screenWidth * xdiff;
    double ydelta = -(y - new_y) / screenHeight * ydiff;
    minX += xdelta, maxX += xdelta;
    minY += ydelta, maxY += ydelta;
  }
  x = new_x, y = new_y;
}


bool checkShaderCompilation(const unsigned int shader, char infoLog[512]) {
  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success) return true;
  glGetShaderInfoLog(shader, 512, nullptr, infoLog);
  return false;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action != GLFW_PRESS)
    return;

  if (key == GLFW_KEY_ESCAPE)
    glfwSetWindowShouldClose(window, true);
  else if (key == GLFW_KEY_UP)
    maxIterations = int(maxIterations * 1.2f);
  else if (key == GLFW_KEY_DOWN)
    maxIterations = std::max(5, int(maxIterations * 0.8f));
  std::cout << "Max iterations: " << maxIterations << std::endl;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  double cursorX, cursorY;
  glfwGetCursorPos(window, &cursorX, &cursorY);

  double xdiff = maxX - minX;
  double ydiff = maxY - minY;

  //yoffset = 1 (scroll up) or -1 (scroll down)
  minX += yoffset / 10 * cursorX / screenWidth * xdiff;
  maxX -= yoffset / 10 * (1 - cursorX / screenWidth) * xdiff;

  minY += yoffset / 10 * (1 - cursorY / screenHeight) * ydiff;
  maxY -= yoffset / 10 * cursorY / screenHeight * ydiff;
}

bool checkProgramCompilation(const unsigned int program, char infoLog[512]) {
  int success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (success) return true;
  glGetProgramInfoLog(program, 512, nullptr, infoLog);
  return false;
}

void processInput(GLFWwindow* window) {

}

void unloadGradient(unsigned int& texture) {
  glDeleteTextures(1, &texture);
}

void loadGradient(unsigned int& texture, const char* imageName) {
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_1D, texture);

  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(imageName, &width, &height, &nrChannels, 0);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, width, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_1D);

  stbi_image_free(data);
}

int main() {
  //Initialize GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 // glfwWindowHint(GLFW_SAMPLES, 9);
  //Create a window
  GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "mandel", nullptr, nullptr);
  if (!window) {
    std::cout << "Couldn't create a window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetWindowMaximizeCallback(window, window_maximize_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);

  //Load GLAD function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Couldn't load GLAD" << std::endl;
    glfwTerminate();
    return -1;
  }

  //Enable MSAA 4x
  //glEnable(GL_MULTISAMPLE);

  //Create & compile the vertex shader
  unsigned int vertexShader;

  std::string vertexSource = readSource("vert.glsl");
  const char* vertexSourceChr = vertexSource.c_str();
  std::cout << vertexSource << std::endl;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSourceChr, nullptr);
  glCompileShader(vertexShader);

  //Check if compilation was successful
  char infoLog[512];
  if (!checkShaderCompilation(vertexShader, infoLog)) {
    std::cout << "Vertex shader compilation has failed" << std::endl;
    std::cout << infoLog << std::endl;
    glfwTerminate();
    return -1;
  }

  //Create & assign the fragment shader
  unsigned int fragmentShader;
  std::string fragmentSource = readSource("frag.glsl");
  const char* fragmentSourceChr = fragmentSource.c_str();
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSourceChr, nullptr);
  glCompileShader(fragmentShader);


  //Check if compilation was successful
  if (!checkShaderCompilation(fragmentShader, infoLog)) {
    std::cout << "Fragment shader compilation has failed" << std::endl;
    std::cout << infoLog << std::endl;
    glfwTerminate();
    return -1;
  }

  //Create & assign the shader program
  //aka link shaders
  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  //Check if compilation was successful
  if (!checkProgramCompilation(shaderProgram, infoLog)) {
    std::cout << "Shader program compilation has failed" << std::endl;
    std::cout << infoLog << std::endl;
    glfwTerminate();
    return -1;
  }

  //Delete shader objects - they're already linked
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  //Draw on entire window area
  //The window is comprised of 2 right triangles
  GLfloat vertices[] = {
      -1, 1, 0,
      -1, -1, 0,
      1, -1, 0,
      1, 1, 0
  };
  GLuint indices[] = {
      0, 1, 2,
      0, 3, 2
  };

  //Create & setup VAO, VBO and EBO
  unsigned int VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  //Link vertex attributes - essentially tell OpenGL
  //how to interpret vertex data
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
  glEnableVertexAttribArray(0);

  //Unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  //Load default color scheme (gradient)
  unsigned int gradient;
  loadGradient(gradient, "wikipedia.png");

  //Main render loop
  double time0, time1;
  time0 = glfwGetTime();
  int frames = 0;
  while (!glfwWindowShouldClose(window)) {
    glGenTextures(1, &gradient);
    glActiveTexture(GL_TEXTURE0);
    //Set "uniforms" aka shared global variables between CPU/GPU
    int lowerLeftID = glGetUniformLocation(shaderProgram, "lowerLeft");
    int upperRightID = glGetUniformLocation(shaderProgram, "upperRight");
    int maxIterationsID = glGetUniformLocation(shaderProgram, "maxIterations");
    int viewportDimensionsID = glGetUniformLocation(shaderProgram, "viewportDimensions");
    int gradientID = glGetUniformLocation(shaderProgram, "gradient");
    glUseProgram(shaderProgram);
    glUniform2f(lowerLeftID, minX, minY);
    glUniform2f(upperRightID, maxX, maxY);
    glUniform1i(maxIterationsID, maxIterations);
    glUniform2f(viewportDimensionsID, screenWidth, screenHeight);
    glUniform1i(gradientID, 0);

    //Draw mandelbrot
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    //Redraw image
    glfwSwapBuffers(window);
    ++frames;
    if (glfwGetTime() - time0 >= 1) {

      std::cout << frames / (glfwGetTime() - time0) << std::endl;
      frames = 0;
      time0 = glfwGetTime();
    }

    //Listen for keyboard/mouse interaction
    glfwPollEvents();
    processInput(window);
  }

  //Exit program
  glfwTerminate();
  return 0;
}
