#include <windows.h>

//Imgui headers for UI
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <sstream>
#include <fstream>

#include "DebugCallback.h"
#include "InitShader.h"    //Functions for loading shaders from text files
#include "LoadMesh.h"      //Functions for creating OpenGL buffers from mesh files
#include "LoadTexture.h"   //Functions for creating OpenGL textures from image files

namespace window
{
   const char* const title = USER_NAME " " PROJECT_NAME; //defined in project settings
   int size[2] = {1024, 1024};
   float clear_color[4] = {0.35f, 0.35f, 0.35f, 1.0f};
}

namespace scene
{
   const std::string asset_dir = "assets/";
   const std::string mesh_name = "Amago0.obj";
   const std::string tex_name = "AmagoT.bmp";

   MeshData mesh;
   GLuint tex_id = -1; //Texture map for mesh

   float angle = 0.0f;
   float scale = 1.0f;
   
   const std::string shader_dir = "shaders/";
   const std::string vertex_shader("lab2_vs.glsl");
   const std::string fragment_shader("lab2_fs.glsl");

   float cameraPos[] = { 0.0f, 0.0f, 1.0f };

   GLuint shader = -1;

   bool clear = true;

   std::string UniformDeets = "";

   float shiny = 1.0f;
}
//For an explanation of this program's structure see https://www.glfw.org/docs/3.3/quick.html 

class UniformDeets {
public:
    std::string name;
    int type;
    bool isInBlock = false;;
    int BlockIndex;
    int index;
};

class StaticVars {
public:
    std::string name;
    void* data;
};

std::vector<StaticVars> variables;

std::string StringifyType(int type) {
    switch (type) {
    case GL_INT:
        return "Integer";
        break;
    case GL_BOOL:
        return "Boolean";
        break;
    case GL_FLOAT:
        return "Float";
        break;
    case GL_FLOAT_VEC2:
        return "2D Float Vector";
        break;
    case GL_FLOAT_VEC3:
        return "3D Float Vector";
        break;
    case GL_FLOAT_VEC4:
        return "4D Float Vector";
        break;
    case GL_SAMPLER_1D:
        return "1D Texture Sampler";
        break;
    case GL_SAMPLER_2D:
        return "2D Texture Sampler";
        break;
    case GL_SAMPLER_3D:
        return "3D Texture Sampler";
        break;
    case GL_FLOAT_MAT3:
        return "3D Float Matrix";
        break;
    case GL_FLOAT_MAT4:
        return "4D Float Matrix";
        break;
    default:
        return "Unknown Type";
    }
}

 std::vector<UniformDeets> GetUniforms() {
    GLint numUniforms = 0;
    glGetProgramInterfaceiv(scene::shader, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
    const GLenum properties[4] = { GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_OFFSET };

    std::vector<UniformDeets> UniformVector;

    for (int unif = 0; unif < numUniforms; unif++)
    {
        GLint values[4];
        glGetProgramResourceiv(scene::shader, GL_UNIFORM, unif, 4, properties, 4, NULL, values);

        std::vector<char> nameData(values[2]);
        UniformDeets temp;
        glGetProgramResourceName(scene::shader, GL_UNIFORM, unif, nameData.size(), NULL, &nameData[0]);
        std::string name(nameData.begin(), nameData.end() - 1);

        temp.index = unif;

        if (values[0] != -1) {
            temp.isInBlock = true;
            std::string BlockName(name.substr(0, name.rfind(".")));
            temp.BlockIndex = glGetUniformBlockIndex(scene::shader, BlockName.c_str());
            std::string last_element(name.substr(name.rfind(".") + 1));
            temp.name = last_element;
        }
        else {
            temp.isInBlock = false;
            temp.BlockIndex = -1;
        }

        temp.name = name;
        temp.type = values[1];

        UniformVector.push_back(temp);
    }

    return UniformVector;
}

std::string PrintUniforms() {
    std::string Uniforms = "";
    std::vector<UniformDeets> UniformVector = GetUniforms();

    for (int i = 0; i < UniformVector.size(); i++) {
        Uniforms += UniformVector[i].name + " (" + StringifyType(UniformVector[i].type) + ")\n";
    }

    return Uniforms;       
}

void CreateUniformWidgets() {

    ImGui::Begin("Uniform Variables");

    if (ImGui::Button("Print Uniform Variables")) {
        scene::UniformDeets = "Currently Active Uniform Variables - \n" + PrintUniforms();
    }

    ImGui::Text(scene::UniformDeets.c_str());

    std::vector<UniformDeets> UniformVector = GetUniforms();

    for (int i = 0; i < UniformVector.size(); i++) {
        GLuint* index;
        int offset[1];
        int type[1];
        GLubyte* blockBuffer = nullptr;
        GLint blockSize;
        GLint binding;
        GLint ubo_id;
        std::vector<char>* nameData;
        std::string SysVar(UniformVector[i].name.substr(0, UniformVector[i].name.rfind("_")));
        if (SysVar == "sys") {
            continue;
        }
        if (UniformVector[i].isInBlock) {

            std::string BlockName(UniformVector[i].name.substr(0, UniformVector[i].name.rfind(".")));

            //GLuint index[] = { UniformVector[i].BlockIndex + UniformVector[i].index };

            GLuint index[1];
            const GLchar* uniformName[] = { UniformVector[i].name.c_str() };
            glGetUniformIndices(scene::shader, 1, uniformName, index);

            glGetActiveUniformsiv(scene::shader, 1, index, GL_UNIFORM_OFFSET, offset);
            glGetActiveUniformsiv(scene::shader, 1, index, GL_UNIFORM_TYPE, type);

            //std::cout << "OFFSET - " << offset[0] << " | TYPE - " << type[0] << " | NAME - " << UniformVector[i].name << "\n";

            int size;
            glGetActiveUniformBlockiv(scene::shader, UniformVector[i].BlockIndex, GL_UNIFORM_BLOCK_NAME_LENGTH, &size);
            glGetActiveUniformBlockiv(scene::shader, UniformVector[i].BlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
            nameData = new std::vector<char> (size);
            glGetProgramResourceName(scene::shader, GL_UNIFORM_BLOCK, UniformVector[i].BlockIndex, nameData->size(), NULL, &(*nameData)[0]);
            std::string name(nameData->begin(), nameData->end() - 1);
            glGetActiveUniformBlockiv(scene::shader, UniformVector[i].BlockIndex, GL_UNIFORM_BLOCK_BINDING, &binding);
            glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, binding, &ubo_id);
            //std::cout << "BLOCK NAME - " << name << " | BLOCK INDEX - " << UniformVector[i].BlockIndex << " | BLOCK BINDING - " << binding << " | BUFFER NAME - " << ubo_id << "\n";
            if (ubo_id == 0) {
                //std::cout << "NAME - " << UniformVector[i].name << " | OFFSET - " << offset[0] << "\n";
                GLuint Buffer;
                glGenBuffers(1, &Buffer);
                glBindBuffer(GL_UNIFORM_BUFFER, Buffer);
                glNamedBufferData(Buffer, blockSize, NULL, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, binding, Buffer);
                glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, binding, &ubo_id);
            }
        }

        int variableIndex = -1;
        for (int var = 0; var < variables.size(); var++) {
            if (variables[var].name == UniformVector[i].name) {
                variableIndex = var;
            }
        }

        if ((UniformVector[i].type >= GL_FLOAT_MAT2 && UniformVector[i].type <= GL_FLOAT_MAT4) || 
            (UniformVector[i].type >= GL_DOUBLE_MAT2 && UniformVector[i].type <= GL_DOUBLE_MAT4)) {
            continue;
        }

        if ((UniformVector[i].type >= GL_FLOAT_VEC2 && UniformVector[i].type <= GL_FLOAT_VEC4)) {
            std::string last_element(UniformVector[i].name.substr(UniformVector[i].name.rfind("_") + 1));
            int numElements = UniformVector[i].type - GL_FLOAT_VEC2 + 2;
            int loc = glGetUniformLocation(scene::shader, UniformVector[i].name.c_str());
            if (last_element == "color") {
                float* col = nullptr;
                if (variableIndex > -1) {
                    col = static_cast<float*>(variables[variableIndex].data);
                }
                else {
                    col = new float[numElements];
                    for (int ind = 0; ind < numElements; ind++) {
                        col[ind] = 0.0f;
                    }
                    StaticVars v;
                    variableIndex = variables.size();
                    variables.push_back(v);
                    variables[variableIndex].data = col;
                    variables[variableIndex].name = UniformVector[i].name;
                }
                switch (numElements) {
                case 3:
                    ImGui::ColorEdit3(UniformVector[i].name.c_str(), col);
                    if (UniformVector[i].isInBlock) {
                        glNamedBufferSubData(ubo_id, offset[0], sizeof(float) * 3, col);
                    }
                    else {
                        glUniform3fv(loc, 1, col);
                    }
                    break;
                case 4:
                    ImGui::ColorEdit4(UniformVector[i].name.c_str(), col);
                    if (UniformVector[i].isInBlock) {
                        glNamedBufferSubData(ubo_id, offset[0], sizeof(float) * 4, col);
                    }
                    else {
                        glUniform4fv(loc, 1, col);
                    }
                    break;
                default:
                    std::cout << "No possible color picker for uniform\n";
                }
            }
            else {
                float* flt = nullptr;
                if (variableIndex > -1) {
                    flt = static_cast<float*>(variables[variableIndex].data);
                }
                else {
                    flt = new float[numElements];
                    for (int ind = 0; ind < numElements; ind++) {
                        flt[ind] = 0.0f;
                    }
                    StaticVars v;
                    variableIndex = variables.size();
                    variables.push_back(v);
                    variables[variableIndex].data = flt;
                    variables[variableIndex].name = UniformVector[i].name;
                }
                switch (numElements) {
                case 2:
                    ImGui::SliderFloat2(UniformVector[i].name.c_str(), flt, -10.0f, 10.0f);
                    if (UniformVector[i].isInBlock) {
                        glNamedBufferSubData(ubo_id, offset[0], sizeof(float)*2, flt);
                    }
                    else {
                        glUniform2fv(loc, 1, flt);
                    }
                    break;
                case 3:
                    ImGui::SliderFloat3(UniformVector[i].name.c_str(), flt, -10.0f, 10.0f);
                    if (UniformVector[i].isInBlock) {
                        glNamedBufferSubData(ubo_id, offset[0], sizeof(float) * 3, flt);
                    }
                    else {
                        glUniform3fv(loc, 1, flt);
                    }
                    break;
                case 4:
                    ImGui::SliderFloat4(UniformVector[i].name.c_str(), flt, -10.0f, 10.0f);
                    if (UniformVector[i].isInBlock) {
                        glNamedBufferSubData(ubo_id, offset[0], sizeof(float) * 4, flt);
                    }
                    else {
                        glUniform4fv(loc, 1, flt);
                    }
                    break;
                default:
                    std::cout << "No possible slider for uniform\n";
                }
            }
        }

        if (UniformVector[i].type == GL_BOOL) {
            int* check = nullptr;
            if (variableIndex > -1) {
                check = static_cast<int*>(variables[variableIndex].data);
            }
            else {
                StaticVars v;
                variableIndex = variables.size();
                variables.push_back(v);
                check = new int;
                variables[variableIndex].data = check;
                variables[variableIndex].name = UniformVector[i].name;
            }
            bool check_ptr = (bool)(*check);
            ImGui::Checkbox(UniformVector[i].name.c_str(), &check_ptr);
            *check = (int)check_ptr;
            if (UniformVector[i].isInBlock) {
                glNamedBufferSubData(ubo_id, offset[0], sizeof(int), check);
            }
            else {
                int loc = glGetUniformLocation(scene::shader, UniformVector[i].name.c_str());
                glUniform1i(loc, *check);
            }
        }

        if (UniformVector[i].type == GL_INT) {
            int* num = nullptr;
            if (variableIndex > -1) {
                num = static_cast<int*>(variables[variableIndex].data);
            }
            else {
                StaticVars v;
                variableIndex = variables.size();
                variables.push_back(v);
                num = new int;
                *num = 0;
                variables[variableIndex].data = num;
                variables[variableIndex].name = UniformVector[i].name;
            }
            //std::cout << "NAME - " << UniformVector[i].name << " | OFFSET - " << offset[0] << " | BLOCK SIZE - " << blockSize << " | TYPE - " << type[0] << " | GL_INT - " << GL_INT << "\n";
            ImGui::SliderInt(UniformVector[i].name.c_str(), num, -10, 10);
            if (UniformVector[i].isInBlock) {
                glNamedBufferSubData(ubo_id, offset[0], sizeof(int), num);
            }
            else {
                int loc = glGetUniformLocation(scene::shader, UniformVector[i].name.c_str());
                glUniform1i(loc, *num);
            }
        }

        if (UniformVector[i].type == GL_FLOAT) {
            float* num = nullptr;
            if (variableIndex > -1) {
                num = static_cast<float*>(variables[variableIndex].data);
            }
            else {
                StaticVars v;
                variableIndex = variables.size();
                variables.push_back(v);
                num = new float;
                *num = 0.0f;
                variables[variableIndex].data = num;
                variables[variableIndex].name = UniformVector[i].name;
            }
            ImGui::SliderFloat(UniformVector[i].name.c_str(), num, -10.0f, 10.0f);
            if (UniformVector[i].isInBlock) {
                glNamedBufferSubData(ubo_id, offset[0], sizeof(float), num);
            }
            else {
                int loc = glGetUniformLocation(scene::shader, UniformVector[i].name.c_str());
                glUniform1f(loc, *num);
            }
        }
    }

    ImGui::SliderFloat("Shininess", &scene::shiny, 0.0f, 100.0f);

    ImGui::End();
}

//Draw the ImGui user interface
void draw_gui(GLFWwindow* window)
{
   //Begin ImGui Frame
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   //Draw Gui
   ImGui::Begin("Debug window");
   if (ImGui::Button("Quit"))
   {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
   }

   //Lab 2: Uncomment these 
   ImGui::SliderFloat("Rotation angle", &scene::angle, -glm::pi<float>(), +glm::pi<float>());
   ImGui::SliderFloat("Scale", &scene::scale, -10.0f, +10.0f);
   ImGui::SliderFloat3("Camera Position", scene::cameraPos, -10.0f, +10.0f);

   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

   ImGui::Checkbox("Clearing", &scene::clear);
   ImGui::End();

   CreateUniformWidgets();

   static bool show_test = false;
   if(show_test)
   {
      ImGui::ShowDemoWindow(&show_test);
   }

   //End ImGui Frame
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


// This function gets called every time the scene gets redisplayed
void display(GLFWwindow* window)
{
   //Clear the screen to the color previously specified in the glClearColor(...) call.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glm::mat4 M = glm::rotate(scene::angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(scene::scale * scene::mesh.mScaleFactor));
   glm::mat4 V = glm::lookAt(glm::vec3(scene::cameraPos[0], scene::cameraPos[1], scene::cameraPos[2]), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
   glm::mat4 P = glm::perspective(glm::pi<float>()/2.0f, 1.0f, 0.1f, 100.0f);

   glUseProgram(scene::shader);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, scene::tex_id);
   int tex_loc = glGetUniformLocation(scene::shader, "diffuse_tex");
   if (tex_loc != -1)
   {
      glUniform1i(tex_loc, 0); // we bound our texture to texture unit 0
   }

   //Get location for shader uniform variable
   int PVM_loc = glGetUniformLocation(scene::shader, "PVM");
   if (PVM_loc != -1)
   {
      glm::mat4 PVM = P * V * M;
      //Set the value of the variable at a specific location
      glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
   }

   int cameraPos_loc = glGetUniformLocation(scene::shader, "sys_cameraPos");
   if (cameraPos_loc != -1)
   {
       //Set the value of the variable at a specific location
       glUniform3fv(cameraPos_loc, 1, scene::cameraPos);
   }

   int shiny_loc = glGetUniformLocation(scene::shader, "sys_shininess");
   if (shiny_loc != -1)
   {
       //Set the value of the variable at a specific location
       glUniform1f(shiny_loc, scene::shiny);
   }

   glBindVertexArray(scene::mesh.mVao);
   scene::mesh.DrawMesh();

   draw_gui(window);

   // Swap front and back buffers
   glfwSwapBuffers(window);
}

void idle()
{
   float time_sec = static_cast<float>(glfwGetTime());
   //Pass time_sec value to the shaders
   int time_loc = glGetUniformLocation(scene::shader, "time");
   if (time_loc != -1)
   {
      glUniform1f(time_loc, time_sec);
   }
}

void reload_shader()
{
    std::string vs = scene::shader_dir + scene::vertex_shader;
    std::string fs = scene::shader_dir + scene::fragment_shader;

    GLuint new_shader = InitShader(vs.c_str(), fs.c_str());

    if (new_shader == -1) // loading failed
    {
        glClearColor(1.0f, 0.0f, 1.0f, 0.0f); //change clear color if shader can't be compiled
    }
    else
    {
        glClearColor(window::clear_color[0], window::clear_color[1], window::clear_color[2], window::clear_color[3]);

        if (scene::shader != -1)
        {
            glDeleteProgram(scene::shader);
        }
        scene::shader = new_shader;
    }
}

//This function gets called when a key is pressed
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   std::cout << "key : " << key << ", " << char(key) << ", scancode: " << scancode << ", action: " << action << ", mods: " << mods << std::endl;

   if (action == GLFW_PRESS)
   {
      switch (key)
      {
      case 'r':
      case 'R':
         reload_shader();
         break;

      case GLFW_KEY_ESCAPE:
         glfwSetWindowShouldClose(window, GLFW_TRUE);
         break;
      }
   }
}

//This function gets called when the mouse moves over the window.
void mouse_cursor(GLFWwindow* window, double x, double y)
{
   //std::cout << "cursor pos: " << x << ", " << y << std::endl;
}

//This function gets called when a mouse button is pressed.
void mouse_button(GLFWwindow* window, int button, int action, int mods)
{
   //std::cout << "button : "<< button << ", action: " << action << ", mods: " << mods << std::endl;
}

//Initialize OpenGL state. This function only gets called once.
void init()
{
   glewInit();
   RegisterDebugCallback();

   std::ostringstream oss;
   //Get information about the OpenGL version supported by the graphics driver.	
   oss << "Vendor: "       << glGetString(GL_VENDOR)                    << std::endl;
   oss << "Renderer: "     << glGetString(GL_RENDERER)                  << std::endl;
   oss << "Version: "      << glGetString(GL_VERSION)                   << std::endl;
   oss << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << std::endl;

   //Output info to console
   std::cout << oss.str();

   //Output info to file
   std::fstream info_file("info.txt", std::ios::out | std::ios::trunc);
   info_file << oss.str();
   info_file.close();

   //Set the color the screen will be cleared to when glClear is called
   glClearColor(window::clear_color[0], window::clear_color[1], window::clear_color[2], window::clear_color[3]);

   glEnable(GL_DEPTH_TEST);

   reload_shader();
   scene::mesh = LoadMesh(scene::asset_dir + scene::mesh_name);
   scene::tex_id = LoadTexture(scene::asset_dir + scene::tex_name);
}


// C++ programs start executing in the main() function.
int main(void)
{
   GLFWwindow* window;

   // Initialize the library
   if (!glfwInit())
   {
      return -1;
   }

#ifdef _DEBUG
   glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

   // Create a windowed mode window and its OpenGL context
   window = glfwCreateWindow(window::size[0], window::size[1], window::title, NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      return -1;
   }

   //Register callback functions with glfw. 
   glfwSetKeyCallback(window, keyboard);
   glfwSetCursorPosCallback(window, mouse_cursor);
   glfwSetMouseButtonCallback(window, mouse_button);

   // Make the window's context current
   glfwMakeContextCurrent(window);

   init();

   // New in Lab 2: Init ImGui
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("#version 150");

   // Loop until the user closes the window 
   while (!glfwWindowShouldClose(window))
   {
      idle();
      display(window);

      // Poll for and process events 
      glfwPollEvents();
   }

   // New in Lab 2: Cleanup ImGui
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();

   glfwTerminate();
   return 0;
}