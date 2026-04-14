#include "STB/stb.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <array>
#include <cstdlib>
#include <cstring>
#include <error.h>
#include <print>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "text_reader.hpp"
static void APIENTRY opengl_error(GLenum source,
                                  GLenum type,
                                  GLuint id,
                                  GLenum severity,
                                  GLsizei length,
                                  const GLchar* message,
                                  const void* userParam) {
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        std::println(
            "ID: {:d}, Severity: {:d}, Source: {:d}, Type: {:d}, Message:\n\t{:4}",
            id,
            severity,
            source,
            type,
            message);
    } else {
        // log_str.append(str);
    }
}

struct window {
    GLFWmonitor* monitor = NULL;
    GLFWwindow* win = NULL;
    GLFWvidmode* window_specs = NULL;
    int width;
    int height;
    float aspect_ratio = 0;
    bool error_bool = false;

    static void destroy() {
        glfwTerminate();
    }
    static void make(window* win) {
        std::println("Creating window");
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
        int chck = glfwInit();
        if (!chck) {
            std::println("GLFW problem");
            exit(EXIT_FAILURE);
        }

        win->monitor = glfwGetPrimaryMonitor();
        win->window_specs = (GLFWvidmode*)glfwGetVideoMode(win->monitor);
        win->width = win->window_specs->width;
        win->height = win->window_specs->height;
        win->aspect_ratio = (float)win->width / (float)win->height;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_REFRESH_RATE, win->window_specs->refreshRate);
        glfwWindowHint(GLFW_SAMPLES, 128);

        win->win =
            glfwCreateWindow(win->width, win->height, "▓▓▓▒▒▒░░░", win->monitor, NULL);
        if (!win->win) {
            const char* desc;
            glfwGetError(&desc);
            std::println("Window failed: {}", desc);
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwSetWindowAttrib(win->win, GLFW_DECORATED, GLFW_FALSE);
        glfwMakeContextCurrent(win->win);
        glfwSwapInterval(0);
        glfwSetWindowUserPointer(win->win, win);

        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            std::println("GLEW Error: {}", (const char*)glewGetErrorString(err));
            std::abort();
        }

        glfwSetWindowSizeCallback(win->win, [](GLFWwindow* w, int width, int height) {
            glViewport(0, 0, width, height);
            window* wptr = (window*)glfwGetWindowUserPointer(w);
            wptr->width = width;
            wptr->height = height;
            wptr->aspect_ratio = (float)width / height;
        });

        std::println("Resolution x:{:d} y:{:d}", win->width, win->height);
    }

    int should_close() {
        return glfwWindowShouldClose(this->win);
    }
    void poll_events() {
        glfwPollEvents();
    }
    void swap_buffers() {
        glfwSwapBuffers(this->win);
    }
};

#define CLEAR_BUFF()                                                                     \
    {                                                                                    \
        glClearColor(0.1, 0.2, 0.2, 1.);                                                 \
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);      \
    }
#define RESET_BUFF()                                                                     \
    {                                                                                    \
        glBindFramebuffer(GL_FRAMEBUFFER, 0);                                            \
        glUseProgram(0);                                                                 \
    }

void glew_load() {
    // std::println("Loading OpenGL Extensions");
    // if (glewInit() != GLEW_OK) {
    //     std::println("GLEW problem");
    //     std::abort();
    // }
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);

    // OpenGL Error handling
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(opengl_error, NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

namespace shader {
    struct state {
        GLuint id;
        GLint type;
        static state
        make(GLint type, const char* path, uint64_t txtsize = 0, bool txtload = true) {
            (txtload) ? std::println("Creating Shader State: {0}", path)
                      : std::println("Creating Shader State");

            unsigned int id;
            const char* src = NULL;
            uint64_t size = 0;
            if (txtload) {
                readfile(path, &src, &size);
                if (src == NULL) {
                    std::println("Failed loading shader state");
                    std::abort();
                }
            } else {
                src = path;
                size = txtsize;
            }

            id = glCreateShader(type);
            const int s = size;
            glShaderSource(id, 1, &src, &s);
            glCompileShader(id);

            free((void*)src);

            return {id, type};
        }
    };
    struct prog {
        GLuint id;
        template <typename... Args>
        static prog make(Args... args) {
            const auto states = std::array<state, sizeof...(Args)>{args...};
            prog p = {glCreateProgram()};
            std::println("Creating Shader Program with states {:d} and ID: {:d}",
                         states.size(),
                         p.id);
            for (const auto& arg : states) {
                glAttachShader(p.id, arg.id);
            }
            glLinkProgram(p.id);

            for (const auto& arg : states) {
                glDetachShader(p.id, arg.id);
                glDeleteShader(arg.id);
            }
            return p;
        }
        static unsigned int uniloc(prog* p, const char* var) {
            return glGetUniformLocation(p->id, var);
        }
        static void use(prog& p) {
            glUseProgram(p.id);
        }
    };
} // namespace shader

[[gnu::const]] inline unsigned char coord2pixel(const float f) {
    return (f + 1) * (255.0f / 2.0f);
}
struct gltexture {
    unsigned char* ptr = nullptr;
    unsigned int ID;
    unsigned int width, height;
    int type;
    int type2;
    int format;
    uint slot = 0;

    gltexture(unsigned int width,
              unsigned int height,
              int type = GL_RGBA32F,
              int type2 = GL_RGBA,
              int format = GL_FLOAT) {
        glGenTextures(1, &this->ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type2, format, NULL);
        this->height = height;
        this->width = width;
        this->type = type;
        this->type2 = type2;
        this->format = format;
    }
    gltexture(const char* path) {
        int width, height, channels;
        unsigned char* t = stbi_load(path, &width, &height, &channels, 0);
        int type = GL_RGB;
        int type2 = GL_RGB;
        int format = GL_UNSIGNED_BYTE;
        glGenTextures(1, &this->ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type2, format, t);
        this->height = height;
        this->width = width;
        this->type = type;
        this->type2 = type2;
        this->format = format;
        this->ptr = t;
    }
    void bind(unsigned int slot) {
        this->slot = slot;
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, ID);
        GLenum tt = type;
        if (format == GL_UNSIGNED_BYTE)
            tt = GL_RGBA8UI;
        glBindImageTexture(slot, ID, 0, GL_FALSE, 0, GL_READ_WRITE, tt);
    }

    std::vector<glm::vec4> img() {
        std::vector<glm::vec4> p;
        p.reserve(width * height);
        glGetTextureImage(ID,
                          0,
                          type2,
                          format,
                          width * height * sizeof(glm::vec4),
                          p.data());
        return p;
    }
    void exportimg(const char* outname) {
        std::vector<glm::vec4> fpixels = img();
        unsigned char* const pixels = new unsigned char[width * height * 4];
        for (size_t i = 0; i < width * height; i++) {
            pixels[i * 4 + 0] = coord2pixel(fpixels[i].x);
            pixels[i * 4 + 1] = coord2pixel(fpixels[i].y);
            pixels[i * 4 + 2] = coord2pixel(fpixels[i].z);
            pixels[i * 4 + 3] = 255;
        }

        stbi_write_png(outname, width, height, 4, pixels, width * 4);

        delete[] pixels;
    }
};

int main() {
    window w;

    window::make(&w);

    glew_load();

    shader::prog noise = shader::prog::make(
        shader::state::make(GL_COMPUTE_SHADER, "./Shaders/compute/noise.comp.glsl"));

    const constexpr size_t mesh_res = 1024 / 8;
    const constexpr size_t tsize = 4096;
    gltexture t1 = gltexture(tsize, tsize);
    t1.bind(0);
    shader::prog::use(noise);
    glDispatchCompute(tsize, tsize, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    // t1.exportimg("t1.png");
    shader::prog mprog = shader::prog::make(
        shader::state::make(GL_VERTEX_SHADER, "./Shaders/model/model.glsl.vert"),
        shader::state::make(GL_FRAGMENT_SHADER, "./Shaders/model/model.glsl.frag"));

    std::println("We need a VAO and VBO because the OpenGL gods demand it");
    float v = 0;
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), &v, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glm::mat4 view = glm::lookAt(glm::vec3(0, 1, 1), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::vec3 pos = glm::vec3(0, -10, 0);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1, 0));
    float near = 0.1f;
    float far = 100.0f;
    glm::mat4 proj = glm::perspective(glm::radians(65.0f), w.aspect_ratio, near, far);

    gltexture ground = gltexture("./Textures/Grass003_2K-JPG/Grass003_2K-JPG_Color.jpg");
    gltexture rock = gltexture("./Textures/Rock028_2K-JPG/Rock028_2K-JPG_Color.jpg");
    unsigned int posloc = shader::prog::uniloc(&mprog, "mpos");
    unsigned int modelloc = shader::prog::uniloc(&mprog, "model");
    unsigned int viewloc = shader::prog::uniloc(&mprog, "view");
    unsigned int projloc = shader::prog::uniloc(&mprog, "proj");
    unsigned int timeloc = shader::prog::uniloc(&mprog, "time");
    unsigned int t1loc = shader::prog::uniloc(&mprog, "height");
    unsigned int groundloc = shader::prog::uniloc(&mprog, "ground");
    unsigned int rockloc = shader::prog::uniloc(&mprog, "rock");
    unsigned int indexloc = shader::prog::uniloc(&mprog, "index");
    unsigned int stripindexloc = shader::prog::uniloc(&mprog, "sindex");
    unsigned int planeresloc = shader::prog::uniloc(&mprog, "planeres");

    std::println("Rendering");
    glEnable(GL_CULL_FACE);
    double time = glfwGetTime();
    double dtime = 0;

    shader::prog::use(mprog);
    t1.bind(0);
    ground.bind(3);
    rock.bind(4);

    glUniform1i(t1loc, t1.slot);
    glUniform1i(groundloc, ground.slot);
    glUniform1i(rockloc, rock.slot);
    glUniform1i(planeresloc, mesh_res);

    constexpr auto rad = glm::radians(90.0f);
    while (!w.should_close()) {

        const double ntime = glfwGetTime();
        dtime = ntime - time;
        time = ntime;

        CLEAR_BUFF();
        RESET_BUFF();

        shader::prog::use(mprog);
        proj = glm::perspective(rad, w.aspect_ratio, 0.1f, 1000.0f);
        view = glm::rotate(view, 0.15f * (float)dtime, glm::vec3(0, 1, 0));

        glUniform3fv(posloc, 1, glm::value_ptr(pos));
        glUniformMatrix4fv(modelloc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewloc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projloc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniform1f(timeloc, time);

        for (int i = 0; i < 9; i++) {
            glUniform1i(indexloc, i);
            for (int si = 0; si < mesh_res; si++) {
                glUniform1i(stripindexloc, si);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, (mesh_res * 2 + 2));
            }
        }

        w.poll_events();
        w.swap_buffers();
    }

    window::destroy();
    return 0;
}
