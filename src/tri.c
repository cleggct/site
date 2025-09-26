#include <GLES3/gl3.h>
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "demo_app.h"

static GLuint g_program = 0;
static GLuint g_vao = 0;
static GLuint g_vbo = 0;
static GLint g_time_loc = -1;
static GLint g_aspect_loc = -1;
static int g_width = 0;
static int g_height = 0;
static int g_active = 0;

static const char *VERT_SRC =
    "#version 300 es\n"
    "layout(location=0) in vec2 a_pos;\n"
    "layout(location=1) in vec3 a_color;\n"
    "out vec3 v_color;\n"
    "uniform float u_time;\n"
    "uniform float u_aspect;\n"
    "void main(){\n"
    "  float angle = u_time * 0.5;\n"
    "  mat2 rot = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));\n"
    "  vec2 p = rot * a_pos;\n"
    "  p.x *= u_aspect;\n"
    "  gl_Position = vec4(p, 0.0, 1.0);\n"
    "  v_color = a_color;\n"
    "}\n";

static const char *FRAG_SRC =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec3 v_color;\n"
    "uniform float u_time;\n"
    "out vec4 fragColor;\n"
    "void main(){\n"
    "  float glow = 0.5 + 0.5 * sin(u_time * 3.14159);\n"
    "  vec3 neon = mix(v_color, vec3(1.0, 0.3, 1.0), glow);\n"
    "  vec3 bright = clamp(neon * (1.15 + 0.65 * glow), 0.0, 1.0);\n"
    "  fragColor = vec4(bright, 1.0);\n"
    "}\n";

static GLuint compile_shader(GLenum type, const char *src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  GLint ok = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[512];
    glGetShaderInfoLog(shader, sizeof log, NULL, log);
#ifdef DEBUG
    printf("shader compile error: %s\n", log);
#endif
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

static GLuint link_program(GLuint vs, GLuint fs) {
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glLinkProgram(prog);
  GLint ok = 0;
  glGetProgramiv(prog, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[512];
    glGetProgramInfoLog(prog, sizeof log, NULL, log);
#ifdef DEBUG
    printf("program link error: %s\n", log);
#endif
    glDeleteProgram(prog);
    prog = 0;
  }
  glDetachShader(prog, vs);
  glDetachShader(prog, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);
  return prog;
}

void demo_app_init(int width, int height) {
  g_width = width;
  g_height = height;
  g_active = 0;

  GLuint vs = compile_shader(GL_VERTEX_SHADER, VERT_SRC);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, FRAG_SRC);
  g_program = link_program(vs, fs);
  g_time_loc = glGetUniformLocation(g_program, "u_time");
  g_aspect_loc = glGetUniformLocation(g_program, "u_aspect");

  const GLfloat verts[] = {
      0.0f,  0.6f,  1.0f, 0.4f, 0.4f,
     -0.6f, -0.4f,  0.4f, 0.8f, 0.4f,
      0.6f, -0.4f,  0.4f, 0.4f, 1.0f,
  };

  glGenVertexArrays(1, &g_vao);
  glBindVertexArray(g_vao);

  glGenBuffers(1, &g_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

  demo_app_resize(width, height);
}

void demo_app_resize(int width, int height) {
  g_width = width;
  g_height = height;
  glViewport(0, 0, g_width, g_height);
}

void demo_app_frame(double time_sec, double dt_sec) {
  (void)dt_sec;
  if (!g_active) return;

  float t = (float)time_sec;
  float aspect = (g_height > 0) ? ((float)g_height / (float)g_width) : 1.0f;

  glDisable(GL_DEPTH_TEST);
  glClearColor(0.05f, 0.08f, 0.12f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(g_program);
  if (g_time_loc >= 0) {
    glUniform1f(g_time_loc, t);
  }
  if (g_aspect_loc >= 0) {
    glUniform1f(g_aspect_loc, aspect);
  }

  glBindVertexArray(g_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void demo_app_set_active(int active) {
  g_active = active ? 1 : 0;
}

void demo_app_shutdown(void) {
  if (g_vbo) {
    glDeleteBuffers(1, &g_vbo);
    g_vbo = 0;
  }
  if (g_vao) {
    glDeleteVertexArrays(1, &g_vao);
    g_vao = 0;
  }
  if (g_program) {
    glDeleteProgram(g_program);
    g_program = 0;
  }
}

void demo_app_handle_key(int key, int pressed) {
  (void)key;
  (void)pressed;
}

void demo_app_update_mouse(float x, float y, int present) {
  (void)x; (void)y; (void)present;
}
