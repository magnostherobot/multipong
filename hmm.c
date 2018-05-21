#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define debug(...) fprintf(stderr, __VA_ARGS__)

#define vec2(t) struct { \
  t x; \
  t y; \
}

typedef vec2(float) fvec2;

typedef struct sllnode {
  struct sllnode* next;
  void* content;
} sllnode;

typedef struct paddle {
  fvec2 pos;
  int player_id;
} paddle;

typedef struct ball {
  fvec2 pos;
  fvec2 vel;
} ball;

bool running = true;

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 600;

sllnode* windows = NULL;

sllnode* balls = NULL;
sllnode* paddles = NULL;

bool init_sdl_video() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    debug("error initialising: %s\n", SDL_GetError());
    return false;
  } else {
    return true;
  }
}

SDL_Window* init_window(int x, int y, int w, int h, char* n) {
  SDL_Window* window = SDL_CreateWindow(
      n, x, y, w, h,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
  );
  if (!window) {
    debug("error creating window: %s\n", SDL_GetError());
    return NULL;
  }

  SDL_Renderer* rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!rend) {
    debug("error creating renderer for %p: %s\n", window, SDL_GetError());
    return NULL;
  }

  return window;
}

void draw_entity(SDL_Window* wind, fvec2 pos) {
  SDL_Renderer* rend = SDL_GetRenderer(wind);

  int winx, winy;
  SDL_GetWindowPosition(wind, &winx, &winy);

  SDL_Rect draw = {
    (int)pos.x - winx,
    (int)pos.y - winy,
    10,
    10
  };

  SDL_RenderFillRect(rend, &draw);
}

void draw_ball(SDL_Window* wind, ball* b) {
  draw_entity(wind, b->pos);
}

void draw_paddle(SDL_Window* wind, paddle* p) {
  draw_entity(wind, p->pos);
}

int draw_loop(void* data) {
  while (running) {
    for (sllnode* n = windows; n; n = n->next) {
      SDL_Window* wind = n->content;
      SDL_Renderer* rend = SDL_GetRenderer(wind);

      SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
      SDL_RenderClear(rend);

      SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);

      for (sllnode* m = balls; m; m = m->next) {
        draw_ball(wind, m->content);
      }
      for (sllnode* m = paddles; m; m = m->next) {
        draw_paddle(wind, m->content);
      }

      SDL_RenderPresent(rend);
    }
  }
}

void move_ball(ball* b, double dt) {
  float posx = ((float)(b->pos.x) + ((double)(b->vel.x) * dt));
  float posy = ((float)(b->pos.y) + ((double)(b->vel.y) * dt));
  if (posx > SCREEN_WIDTH) {
    posx = SCREEN_WIDTH - (SCREEN_WIDTH - posx);
    b->vel.x *= -1.;
  }
  if (posy > SCREEN_HEIGHT) {
    posy = SCREEN_HEIGHT - (SCREEN_HEIGHT - posy);
    b->vel.y *= -1.;
  }

  debug("x: %f, y: %f\n", posx, posy);

  b->pos.x = posx;
  b->pos.y = posy;
}

int physics_loop(void* data) {
  static unsigned long old_frame_time;

  while (running) {
    unsigned long new_frame_time = SDL_GetTicks();
    unsigned long dt = new_frame_time - old_frame_time;
    old_frame_time = new_frame_time;

    for (sllnode* n = balls; n; n = n->next) {
      move_ball(n->content, (double)dt / 1000.);
    }

    SDL_Delay(10);
  }
}

int event_loop(void* data) {
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      ;
    }
  }
}

sllnode* make_windows(int n) {
  sllnode* next = NULL;
  // goes in reverse!
  for (int i = 0; i < n; ++i) {
    sllnode* lln = malloc(sizeof(*lln));
    lln->content = init_window(20, 20, 400, 400, "HMM");
    lln->next = next;
    next = lln;
  }
  return next;
}

sllnode* make_balls() {
  sllnode* only = malloc(sizeof(*only));
  only->content = malloc(sizeof(ball));

  ball* ent = only->content;

  ent->pos.x = 200;
  ent->pos.y = 200;

  ent->vel.x = 100;
  ent->vel.y = 30;

  only->next = NULL;
  return only;
}

int main(int argc, char* args[]) {
  if (!init_sdl_video()) return 1;

  windows = make_windows(5);
  balls = make_balls();

  SDL_CreateThread(draw_loop, "trh_draw_loop", NULL);
  SDL_CreateThread(physics_loop, "trh_physics_loop", NULL);
  event_loop(NULL);
}
