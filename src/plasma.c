#include <GLES3/gl3.h>
#include <math.h>
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
    "out vec2 v_uv;\n"
    "void main(){\n"
    "  v_uv = a_pos * 0.5 + 0.5;\n"
    "  gl_Position = vec4(a_pos, 0.0, 1.0);\n"
    "}\n";

static const char *FRAG_SRC =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec2 v_uv;\n"
    "uniform float u_time;\n"
    "uniform float u_aspect;\n"
    "out vec4 fragColor;\n"
    "void main(){\n"
    "  vec2 uv = v_uv * 2.0 - 1.0;\n"
    "  uv.x *= u_aspect;\n"
    "  float t = u_time * 0.4;\n"
    "  mat2 rot = mat2(cos(t * 0.7), -sin(t * 0.7), sin(t * 0.7), cos(t * 0.7));\n"
    "  vec2 p = rot * uv;\n"
    "  float waves = sin(p.x * 3.5 + t * 1.2) + sin(p.y * 4.5 - t * 1.7);\n"
    "  vec2 swirlBase = uv + 0.35 * vec2(sin(t * 0.9 + uv.y * 6.0), cos(t * 0.6 + uv.x * 6.0));\n"
    "  float swirl = sin(swirlBase.x * swirlBase.y * 8.0 + t * 2.0);\n"
    "  float rings = sin(length(uv * 3.2 + vec2(sin(t), cos(t * 0.8))) - t * 1.3);\n"
    "  float v = waves * 0.35 + swirl * 0.4 + rings * 0.25;\n"
    "  vec3 col = 0.5 + 0.5 * cos(vec3(0.0, 2.0, 4.0) + v * 3.4 + t * 0.7);\n"
    "  fragColor = vec4(col, 1.0);\n"
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
      -1.0f, -1.0f,
       3.0f, -1.0f,
      -1.0f,  3.0f,
  };

  glGenVertexArrays(1, &g_vao);
  glBindVertexArray(g_vao);

  glGenBuffers(1, &g_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);

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
  float aspect = (g_height > 0) ? ((float)g_width / (float)g_height) : 1.0f;

  glDisable(GL_DEPTH_TEST);
  glClearColor(0.02f, 0.03f, 0.05f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(g_program);
  glUniform1f(g_time_loc, t);
  glUniform1f(g_aspect_loc, aspect);

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
