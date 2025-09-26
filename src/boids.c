#include <GLES3/gl3.h>
#include <math.h>
#include <stdint.h>
#include <stddef.h>

#include "demo_app.h"

#define MAX_BOIDS 160
#define NEIGHBOR_RADIUS 80.0f
#define SEPARATION_RADIUS 90.0f
#define MAX_SPEED 600.0f
#define MAX_FORCE 180.0f
#define VELOCITY_DAMP_ACTIVE 0.985f
#define VELOCITY_DAMP_IDLE 0.9995f
#define EDGE_THRESHOLD 60.0f
#define EDGE_FORCE 320.0f

static int g_width = 0;
static int g_height = 0;
static int g_active = 0;

static GLuint g_program = 0;
static GLuint g_vao = 0;
static GLuint g_vbo = 0;
static GLint g_time_loc = -1;
static GLint g_resolution_loc = -1;

static float g_positions[MAX_BOIDS][2];
static float g_velocities[MAX_BOIDS][2];
static float g_mouse_x = 0.0f;
static float g_mouse_y = 0.0f;
static int g_mouse_present = 0;

static uint32_t g_rng = 1u;

static float frand(void) {
  g_rng = g_rng * 1664525u + 1013904223u;
  return (float)((g_rng >> 8) & 0xFFFFFFu) / (float)0x1000000u;
}

static float wrap_distance(float delta, float extent) {
  if (extent <= 0.0f) return delta;
  float half = extent * 0.5f;
  while (delta > half) delta -= extent;
  while (delta < -half) delta += extent;
  return delta;
}

static float wrap_mod(float value, float extent) {
  if (extent <= 0.0f) return value;
  float wrapped = fmodf(value, extent);
  if (wrapped < 0.0f) wrapped += extent;
  return wrapped;
}

static const char *VERT_SRC =
    "#version 300 es\n"
    "layout(location=0) in vec2 a_clip;\n"
    "uniform vec2 u_resolution;\n"
    "void main(){\n"
    "  gl_Position = vec4(a_clip, 0.0, 1.0);\n"
    "  gl_PointSize = 6.0;\n"
    "}\n";

static const char *FRAG_SRC =
    "#version 300 es\n"
    "precision highp float;\n"
    "uniform float u_time;\n"
    "out vec4 fragColor;\n"
    "void main(){\n"
    "  float r = 0.6 + 0.4 * sin(u_time * 1.7 + gl_FragCoord.x * 0.02);\n"
    "  float g = 0.6 + 0.4 * sin(u_time * 1.3 + gl_FragCoord.y * 0.02 + 1.7);\n"
    "  float b = 0.7 + 0.3 * sin(u_time * 1.1 + 3.1);\n"
    "  vec2 uv = (gl_PointCoord - 0.5) * 2.0;\n"
    "  float alpha = smoothstep(1.0, 0.2, dot(uv, uv));\n"
    "  fragColor = vec4(r, g, b, alpha);\n"
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
    printf("boids shader error: %s\n", log);
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
    printf("boids link error: %s\n", log);
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

static void reset_boids(void) {
  for (int i = 0; i < MAX_BOIDS; ++i) {
    g_positions[i][0] = frand() * g_width;
    g_positions[i][1] = frand() * g_height;
    float angle = frand() * 6.2831853f;
    float speed = 60.0f + frand() * 40.0f;
    g_velocities[i][0] = cosf(angle) * speed;
    g_velocities[i][1] = sinf(angle) * speed;
  }
}

void demo_app_init(int width, int height) {
  g_width = width;
  g_height = height;
  g_active = 0;
  g_rng = 0x1234ABCDu ^ (uint32_t)(width * 131u + height);

  GLuint vs = compile_shader(GL_VERTEX_SHADER, VERT_SRC);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, FRAG_SRC);
  g_program = link_program(vs, fs);
  g_time_loc = glGetUniformLocation(g_program, "u_time");
  g_resolution_loc = glGetUniformLocation(g_program, "u_resolution");

  glGenVertexArrays(1, &g_vao);
  glBindVertexArray(g_vao);

  glGenBuffers(1, &g_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
  glBufferData(GL_ARRAY_BUFFER, MAX_BOIDS * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

  reset_boids();
  demo_app_resize(width, height);
}

void demo_app_resize(int width, int height) {
  g_width = width;
  g_height = height;
  glViewport(0, 0, g_width, g_height);
}

void demo_app_frame(double time_sec, double dt_sec) {
  if (!g_active) return;
  float dt = (float)dt_sec;
  if (dt > 0.05f) dt = 0.05f;

  for (int i = 0; i < MAX_BOIDS; ++i) {
    float px = g_positions[i][0];
    float py = g_positions[i][1];
    float px_screen = wrap_mod(px, (float)g_width);
    float py_screen = wrap_mod(py, (float)g_height);
    float vx = g_velocities[i][0];
    float vy = g_velocities[i][1];

    float align_x = 0.f, align_y = 0.f;
    float cohesion_x = 0.f, cohesion_y = 0.f;
    float separation_x = 0.f, separation_y = 0.f;
    int neighbors = 0;

    for (int j = 0; j < MAX_BOIDS; ++j) {
      if (i == j) continue;
      float dx = wrap_distance(g_positions[j][0] - px, (float)g_width);
      float dy = wrap_distance(g_positions[j][1] - py, (float)g_height);

      float dist2 = dx * dx + dy * dy;
      if (dist2 < NEIGHBOR_RADIUS * NEIGHBOR_RADIUS) {
        align_x += g_velocities[j][0];
        align_y += g_velocities[j][1];
        cohesion_x += px + dx;
        cohesion_y += py + dy;
        if (dist2 < SEPARATION_RADIUS * SEPARATION_RADIUS && dist2 > 0.0001f) {
          separation_x -= dx / dist2;
          separation_y -= dy / dist2;
        }
        neighbors++;
      }
    }

    float accel_x = 0.f;
    float accel_y = 0.f;

    if (neighbors > 0) {
      float inv = 1.0f / neighbors;
      align_x = (align_x * inv - vx) * 0.1f;
      align_y = (align_y * inv - vy) * 0.1f;

      cohesion_x = ((cohesion_x * inv) - px) * 0.08f;
      cohesion_y = ((cohesion_y * inv) - py) * 0.08f;

      separation_x *= 0.35f;
      separation_y *= 0.35f;

      accel_x += align_x + cohesion_x + separation_x;
      accel_y += align_y + cohesion_y + separation_y;
    }

    if (g_mouse_present) {
      float dxm = g_mouse_x - px_screen;
      float dym = g_mouse_y - py_screen;
      float distm2 = dxm * dxm + dym * dym;
      if (distm2 > 25.0f) {
        float inv = 1.0f / sqrtf(distm2);
        accel_x += dxm * inv * 160.0f;
        accel_y += dym * inv * 160.0f;
      }
    }

    if (g_width > 0) {
      float left_dist = px_screen;
      if (left_dist < EDGE_THRESHOLD) {
        float t = (EDGE_THRESHOLD - left_dist) * (1.0f / EDGE_THRESHOLD);
        accel_x += EDGE_FORCE * t;
      }
      float right_dist = (float)g_width - px_screen;
      if (right_dist < EDGE_THRESHOLD) {
        float t = (EDGE_THRESHOLD - right_dist) * (1.0f / EDGE_THRESHOLD);
        accel_x -= EDGE_FORCE * t;
      }
    }
    if (g_height > 0) {
      float top_dist = py_screen;
      if (top_dist < EDGE_THRESHOLD) {
        float t = (EDGE_THRESHOLD - top_dist) * (1.0f / EDGE_THRESHOLD);
        accel_y += EDGE_FORCE * t;
      }
      float bottom_dist = (float)g_height - py_screen;
      if (bottom_dist < EDGE_THRESHOLD) {
        float t = (EDGE_THRESHOLD - bottom_dist) * (1.0f / EDGE_THRESHOLD);
        accel_y -= EDGE_FORCE * t;
      }
    }

    float speed = sqrtf(vx * vx + vy * vy);
    if (speed > 0.0001f) {
      accel_x += (vx / speed) * 6.0f;
      accel_y += (vy / speed) * 6.0f;
    }

    float acc_mag = sqrtf(accel_x * accel_x + accel_y * accel_y);
    if (acc_mag > MAX_FORCE) {
      float scale = MAX_FORCE / acc_mag;
      accel_x *= scale;
      accel_y *= scale;
    }

    vx += accel_x * dt;
    vy += accel_y * dt;
    float new_speed = sqrtf(vx * vx + vy * vy);
    if (new_speed > MAX_SPEED) {
      float scale = MAX_SPEED / new_speed;
      vx *= scale;
      vy *= scale;
    }

    float damp = g_mouse_present ? VELOCITY_DAMP_ACTIVE : VELOCITY_DAMP_IDLE;
    vx *= damp;
    vy *= damp;

    if (!g_mouse_present) {
      float cruise = 80.0f;
      float speed_after = sqrtf(vx * vx + vy * vy);
      if (speed_after < cruise) {
        if (speed_after > 0.0001f) {
          float scale = cruise / speed_after;
          vx *= scale;
          vy *= scale;
        } else {
          // Re-inject a tiny random push to keep idle motion alive.
          float angle = frand() * 6.2831853f;
          vx = cosf(angle) * cruise;
          vy = sinf(angle) * cruise;
        }
      }
    }

    px += vx * dt;
    py += vy * dt;

    g_positions[i][0] = px;
    g_positions[i][1] = py;
    g_velocities[i][0] = vx;
    g_velocities[i][1] = vy;
  }

  float verts[MAX_BOIDS * 2];
  float inv_w = g_width > 0 ? 1.0f / g_width : 0.0f;
  float inv_h = g_height > 0 ? 1.0f / g_height : 0.0f;
  for (int i = 0; i < MAX_BOIDS; ++i) {
    float screen_x = wrap_mod(g_positions[i][0], (float)g_width);
    float screen_y = wrap_mod(g_positions[i][1], (float)g_height);
    float x = screen_x * inv_w * 2.0f - 1.0f;
    float y = 1.0f - screen_y * inv_h * 2.0f;
    verts[i * 2 + 0] = x;
    verts[i * 2 + 1] = y;
  }

  glDisable(GL_DEPTH_TEST);
  glUseProgram(g_program);
  glUniform1f(g_time_loc, (float)time_sec);
  if (g_resolution_loc >= 0) {
    glUniform2f(g_resolution_loc, (float)g_width, (float)g_height);
  }

  glBindVertexArray(g_vao);
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
  glDrawArrays(GL_POINTS, 0, MAX_BOIDS);
}

void demo_app_set_active(int active) {
  g_active = active ? 1 : 0;
}

void demo_app_update_mouse(float x, float y, int present) {
  g_mouse_x = x;
  g_mouse_y = y;
  g_mouse_present = present;
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
    case 4: if (pressed) reset_boids(); break; // Z
    default: (void)pressed; break;
  }
}
