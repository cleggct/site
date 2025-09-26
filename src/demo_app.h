#ifndef DEMO_APP_H
#define DEMO_APP_H

void demo_app_init(int width, int height);
void demo_app_resize(int width, int height);
void demo_app_frame(double time_sec, double dt_sec);
void demo_app_set_active(int active);
void demo_app_handle_key(int key, int pressed);
void demo_app_update_mouse(float x, float y, int present);
void demo_app_shutdown(void);

#endif /* DEMO_APP_H */
