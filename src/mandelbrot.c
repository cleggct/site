#include <GLES3/gl3.h>
#include <math.h>
#include <stddef.h>
#include <string.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "demo_app.h"

static GLuint g_program = 0;
static GLuint g_vao = 0;
static GLuint g_vbo = 0;
static GLint g_time_loc = -1;
static GLint g_aspect_loc = -1;
static GLint g_center_loc = -1;
static GLint g_scale_loc = -1;
static int g_width = 0;
static int g_height = 0;

static float g_center_x = -0.5f;
static float g_center_y = 0.0f;
static float g_scale = 1.8f;
static int g_active = 0;
static int g_key_left = 0, g_key_right = 0, g_key_up = 0, g_key_down = 0;
static int g_key_zoom_in = 0, g_key_zoom_out = 0;

static const char *VERT_SRC =
    "#version 300 es\n"
    "layout(location=0) in vec2 a_pos;\n"
    "out vec2 v_pos;\n"
    "void main(){\n"
    "  v_pos = a_pos;\n"
    "  gl_Position = vec4(a_pos, 0.0, 1.0);\n"
    "}\n";

static const char *FRAG_SRC =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec2 v_pos;\n"
    "uniform float u_time;\n"
    "uniform float u_aspect;\n"
    "uniform vec2 u_center;\n"
    "uniform float u_scale;\n"
    "out vec4 fragColor;\n"
    "vec3 palette(float t){\n"
    "  return vec3(0.5 + 0.5 * cos(6.2831 * (t + vec3(0.0, 0.33, 0.67))));\n"
    "}\n"
    "void main(){\n"
    "  vec2 uv = v_pos;\n"
    "  uv.x *= u_aspect;\n"
    "  vec2 c = u_center + uv * u_scale;\n"
    "  vec2 z = vec2(0.0);\n"
    "  float m = 0.0;\n"
    "  const int ITER = 150;\n"
    "  for (int i = 0; i < ITER; ++i){\n"
    "    z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;\n"
    "    if (dot(z,z) > 4.0){\n"
    "      float nu = float(i) - log2(log2(dot(z,z))) + 4.0;\n"
    "      m = clamp(nu / float(ITER), 0.0, 1.0);\n"
    "      break;\n"
    "    }\n"
    "  }\n"
    "  float hue = fract(m + 0.15 * sin(u_time * 0.3));\n"
    "  vec3 col = (m == 0.0) ? vec3(0.05, 0.06, 0.08) : palette(hue);\n"
    "  fragColor = vec4(col, 1.0);\n"
    "}\n";

static GLuint compile_shader(GLenum type, const char *src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  GLint ok = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
#ifdef DEBUG
    char log[512];
    glGetShaderInfoLog(shader, sizeof log, NULL, log);
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
#ifdef DEBUG
    char log[512];
    glGetProgramInfoLog(prog, sizeof log, NULL, log);
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
  g_center_loc = glGetUniformLocation(g_program, "u_center");
  g_scale_loc = glGetUniformLocation(g_program, "u_scale");

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
  if (!g_active) return;

  float aspect = (g_height > 0) ? ((float)g_width / (float)g_height) : 1.0f;
  float pan_speed = g_scale * 0.6f;
  if (g_key_left) g_center_x -= pan_speed * (float)dt_sec;
  if (g_key_right) g_center_x += pan_speed * (float)dt_sec;
  if (g_key_up) g_center_y += pan_speed * (float)dt_sec;
  if (g_key_down) g_center_y -= pan_speed * (float)dt_sec;

  float zoom_rate = 1.6f;
  if (g_key_zoom_in) g_scale *= expf(-zoom_rate * (float)dt_sec);
  if (g_key_zoom_out) g_scale *= expf(zoom_rate * (float)dt_sec);
  if (g_scale < 0.0002f) g_scale = 0.0002f;
  if (g_scale > 4.0f) g_scale = 4.0f;

  glDisable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(g_program);
  if (g_aspect_loc >= 0) glUniform1f(g_aspect_loc, aspect);
  if (g_time_loc >= 0) glUniform1f(g_time_loc, (float)time_sec);
  if (g_center_loc >= 0) glUniform2f(g_center_loc, g_center_x, g_center_y);
  if (g_scale_loc >= 0) glUniform1f(g_scale_loc, g_scale);

  glBindVertexArray(g_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void demo_app_set_active(int active) {
  g_active = active ? 1 : 0;
  if (!g_active) {
    g_key_left = g_key_right = g_key_up = g_key_down = 0;
    g_key_zoom_in = g_key_zoom_out = 0;
  }
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
  switch (key) {
    case 0: g_key_left = pressed; break;
    case 1: g_key_right = pressed; break;
    case 2: g_key_up = pressed; break;
    case 3: g_key_down = pressed; break;
    case 4: g_key_zoom_in = pressed; break;
    case 5: g_key_zoom_out = pressed; break;
    default: break;
  }
}

void demo_app_update_mouse(float x, float y, int present) {
  (void)x; (void)y; (void)present;
}
