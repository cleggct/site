#include <emscripten.h>
#include <emscripten/html5.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "demo_app.h"

static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE g_ctx = 0;
static int g_active = 0;
static double g_prev_time = 0.0;
static int g_width = 0;
static int g_height = 0;
static float g_mouse_x = 0.0f;
static float g_mouse_y = 0.0f;
static int g_mouse_present = 0;

EM_JS(char *, runtime_acquire_selector, (), {
  var selector = Module['__canvasSelector'] || '#canvas';
  var length = lengthBytesUTF8(selector) + 1;
  var ptr = _malloc(length);
  stringToUTF8(selector, ptr, length);
  return ptr;
});

static void ensure_context_current(void) {
  if (g_ctx) {
    emscripten_webgl_make_context_current(g_ctx);
  }
}

static EM_BOOL handle_key_event(int type, const EmscriptenKeyboardEvent *ev, void *userData) {
  (void)userData;
  if (!g_active) return EM_FALSE;
  int pressed = (type == EMSCRIPTEN_EVENT_KEYDOWN) ? 1 : 0;
  const char *code = ev->code;
  int key = -1;
  if (!strcmp(code, "ArrowLeft")) key = 0;
  else if (!strcmp(code, "ArrowRight")) key = 1;
  else if (!strcmp(code, "ArrowUp")) key = 2;
  else if (!strcmp(code, "ArrowDown")) key = 3;
  else if (!strcmp(code, "KeyZ")) key = 4;
  else if (!strcmp(code, "KeyX")) key = 5;
  if (key >= 0) {
    demo_app_handle_key(key, pressed);
    return EM_TRUE;
  }
  return EM_FALSE;
}

static void frame(void) {
  ensure_context_current();
  double now = emscripten_get_now() * 0.001;
  double dt = (g_prev_time > 0.0) ? (now - g_prev_time) : 0.0;
  g_prev_time = now;
  if (!g_active) return;
  demo_app_frame(now, dt);
}

EMSCRIPTEN_KEEPALIVE
void set_active(int active) {
  ensure_context_current();
  g_active = active ? 1 : 0;
  demo_app_set_active(g_active);
  if (!g_active) {
    g_prev_time = emscripten_get_now() * 0.001;
  }
}

EMSCRIPTEN_KEEPALIVE
void resize_canvas(int width, int height) {
  ensure_context_current();
  g_width = width;
  g_height = height;
  demo_app_resize(width, height);
}

EMSCRIPTEN_KEEPALIVE
void update_mouse(float x, float y, int present) {
  g_mouse_x = x;
  g_mouse_y = y;
  g_mouse_present = present;
  demo_app_update_mouse(x, y, present);
}

int main(void) {
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.majorVersion = 2;
  attr.minorVersion = 0;
  attr.alpha = EM_FALSE;
  attr.depth = EM_FALSE;
  attr.stencil = EM_FALSE;
  attr.antialias = EM_TRUE;
  attr.enableExtensionsByDefault = EM_TRUE;

  char *selector = runtime_acquire_selector();
  g_ctx = emscripten_webgl_create_context(selector, &attr);
  free(selector);
  if (g_ctx <= 0) {
    return 1;
  }
  ensure_context_current();

  emscripten_webgl_get_drawing_buffer_size(g_ctx, &g_width, &g_height);
  demo_app_init(g_width, g_height);
  demo_app_set_active(0);

  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 1, handle_key_event);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 1, handle_key_event);

  g_prev_time = emscripten_get_now() * 0.001;
  emscripten_set_main_loop(frame, 0, 1);
  return 0;
}
