// libtsprites demo - libtsprites, 2024, M64

#include "../libtsprites/src/include/tscolors.hpp"
#include "../libtsprites/src/include/tscreen.hpp"
#include "../libtsprites/src/include/tseffects.hpp"
#include "../libtsprites/src/include/tsprites.hpp"
#include "../libtsprites/src/include/tsrender.hpp"
#include "../libtsprites/src/include/tsscener.hpp"
#include "../libtsprites/src/include/tsutils.hpp"
#include "math.h"
#include "resid-dmpplayer.h"
#include "resid.h"
#include "resources/zoom64-2.sid_dmp.h"
#include "sdl-audio.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>

#define FPS 60
#define KDEBOUNCE_TICKS 8

//  NOTE: -- PLASMA GHOST ------------------------------------------------------

typedef enum GhostDirection_e {
  Ghost_Left = 0,
  Ghost_Right = 1
} GhostDirection_t;

typedef struct GhostPlayer_s {
  GhostDirection_t direction;
} GhostPlayer_t;

GhostPlayer_t PlasmaGhost;

//  NOTE: -- ReSID / Audio Setup  ----------------------------------------------

SDL_AudioDeviceID SDL_Audio_DevID;
SDL_AudioSpec SDL_Audio_Spec;

ReSID R;
ReSIDPbData D;
ReSIDDmpPlayer DP(&R, &D);

//  NOTE: -- ReSID / Audio THREAD  ---------------------------------------------

void audioprocessing_tread() {
  while (D.play) {
    if (DP.update()) {
      DP.dmp_idx = 0;
    }
    SDL_Delay(10);
  }
}

//  NOTE: -- main  -------------------------------------------------------------

int main(int argc, char **argv) {
  Scene_t scene1;
  Scene_t scene2;
  Scene_t scene3;
  Scene_t scene4;
  Scene_t scene5;

  scene_init(&scene1, 1, scene_seconds_to_ticks(3.0, FPS));
  scene2 = scene_then(&scene1, scene_seconds_to_ticks(8.0, FPS));
  scene3 = scene_then(&scene2, scene_seconds_to_ticks(8.0, FPS));
  scene4 = scene_then(&scene3, scene_seconds_to_ticks(2.0, FPS));
  scene5 = scene_then(&scene4, scene_seconds_to_ticks(2.0, FPS));

  int stop = 0;
  SDL_Event event;
  int CHAPTER = 0;
  tick_t TICK = 0;
  tick_t TICK_ONGOING = 0;
  unsigned int KEY;
  int KDEBOUNCE_CTR = 0;

  if (term_columns() < 128) {
    printf("[MAIN ] Please increase your terminal width to 128!\n");
    exit(1);
  }

  FPSCtx_t ctx;
  fps_init(FPS, &ctx); // 60 FPS

  TSRenderEngineTopDown engine;
  TScreen Screen(120, 40);
  Screen.bg_color = {0x10, 0x10, 0x10};
  Screen.SetRenderEngine(&engine);
  Screen.SetScreenMode(SCREEN_BGCOLOR);
  Screen.SetXY(4, 2);

  sdl_audio_init(&SDL_Audio_DevID, &SDL_Audio_Spec, &D);

  // NOTE: -- GENERAL SETUP ---------------------------------------------------

  TSFXPlasmaCTX_t plasma_ctx;
  TSFXPlasmaCTX_t plasma2_ctx;

  TSprite SprM64;
  TSprite SprM642;
  TSprite SprPlasma;
  TSprite SprScrollText1;
  TSprite SprScrollText2;
  TSprite SprGhost1;

  if (SprGhost1.ImportFromPNGFile((char *)"resources/ghostleftright.png"))
    return 1;
  SprGhost1.VSplitFixedW(SprGhost1.fs.frames[0], 18);
  render_surface_copy(SprGhost1.fs.frames[1]->out_surface,
                      SprGhost1.out_surface);
  TSpriteAnimation_t *ghost_ani_left = SprGhost1.NewAnimation(1, 6, 1);
  ghost_ani_left->tick_divider = 4;
  SprGhost1.StartAnimation(Ghost_Left);

  TSpriteAnimation_t *ghost_ani_right = SprGhost1.NewAnimation(7, 6, 1);
  ghost_ani_right->tick_divider = 4;
  SprGhost1.StartAnimation(Ghost_Right);
  PlasmaGhost.direction = Ghost_Right;

  // charset->VSplitFixedW(charset->fs.frames[0], slicewidth);

  if (SprM64.ImportFromPNGFile((char *)"resources/m64logo_pl-010101.png"))
    return 1;
  if (SprM642.ImportFromPNGFile((char *)"resources/m64logo_tr-010203.png"))
    return 1;
  if (SprPlasma.ImportFromPNGFile((char *)"resources/100x52.png"))
    return 1;
  if (SprScrollText1.ImportFromPNGFile((char *)"resources/mainscroll.png"))
    return 1;
  if (SprScrollText2.ImportFromPNGFile((char *)"resources/subscroll.png"))
    return 1;

  // SprPlasma.SetXY(20, 26); // 80x40
  RenderSurface_t backup_surface;
  render_surface_init(&backup_surface, SprPlasma.w, SprPlasma.h, {0, 0, 0});

  char *spr_txts[] = {(char *)"[       ]", (char *)"[  UP   ]",
                      (char *)"[ RIGHT ]", (char *)"[ DOWN  ]",
                      (char *)"[ LEFT  ]", (char *)"[  ESC  ]"};

  // --

  RGBColor_t color_replace1 = {0x01, 0x01, 0x01};
  RGBColor_t color_replace2 = {0x02, 0x02, 0x02};
  RGBColor_t color_replace3 = {0x03, 0x03, 0x03};
  RGBColor_t color_fill = {0x00, 0x00, 0x40};
  RGBColor_t fade_color1;
  RGBColor_t fade_color2;
  RGBPalette_t *pal_tmp_fade1;
  RGBPalette_t *pal_tmp_fade2;
  RGBColor_t plasma_color_replace = {0x01, 0x01, 0x01};

  RGBPalette_t *pal_fade1;
  RGBPalette_t *pal_fade2;
  RGBPalette_t *pal_fade3;

  // -- color fading tables for logo
  fade_color1 = {0x80, 0x20, 0x80};
  fade_color2 = {0x00, 0x60, 0xa0};
  pal_tmp_fade1 = CreatePaletteFadeColors(&fade_color1, &fade_color2, 16);
  pal_tmp_fade2 = CreatePaletteFadeColors(&fade_color2, &fade_color1, 16);
  pal_fade1 = PaletteAppend(pal_tmp_fade1, pal_tmp_fade2);

  fade_color2 = {0x00, 0x20, 0x80};
  fade_color1 = {0x00, 0x60, 0xa0};
  pal_tmp_fade1 = CreatePaletteFadeColors(&fade_color1, &fade_color2, 16);
  pal_tmp_fade2 = CreatePaletteFadeColors(&fade_color2, &fade_color1, 16);
  pal_fade2 = PaletteAppend(pal_tmp_fade1, pal_tmp_fade2);

  fade_color1 = {0x00, 0x20, 0x80};
  fade_color2 = {0x00, 0x60, 0xa0};
  pal_tmp_fade1 = CreatePaletteFadeColors(&fade_color1, &fade_color2, 16);
  pal_tmp_fade2 = CreatePaletteFadeColors(&fade_color2, &fade_color1, 16);
  pal_fade3 = PaletteAppend(pal_tmp_fade1, pal_tmp_fade2);

  // -- plasma

  tsfx_plasma_init(&plasma_ctx, SprM64.w, SprM64.h, 256, 3,
                   plasma_color_replace, SprM64.restore_surface);

  tsfx_plasma_init(&plasma2_ctx, SprPlasma.w, SprPlasma.h, 256, 3,
                   plasma_color_replace, SprPlasma.restore_surface);

  // --
  SSprite SSpr_direction(spr_txts, 6);
  SSpr_direction.x = 55;
  SSpr_direction.y = 39 * 2 + 1;

  SSprite SSpr_XY((char *)"[ X: 0, Y: 0 ]");
  SSpr_XY.x = 2;
  SSpr_XY.y = 39 * 2 + 1;
  char SSpr_XY_txtbuf[32];
  sprintf(SSpr_XY_txtbuf, "[ X: %d, Y: %d ]", SprM64.x, SprM64.y);
  SSpr_XY.SetText(SSpr_XY_txtbuf);

  SSprite SSpr_FCTX((char *)"[ dT: 0 us ]");
  SSpr_FCTX.x = 70;
  SSpr_FCTX.y = 39 * 2 + 1;
  char SSpr_FCTX_txtbuf[128];
  sprintf(SSpr_FCTX_txtbuf, "[ dT: 0 us ]");
  SSpr_FCTX.SetText(SSpr_FCTX_txtbuf);

  Screen.AddSprite(&SprGhost1);
  Screen.AddSprite(&SprM64);
  Screen.AddSprite(&SprScrollText1);
  Screen.AddSprite(&SprScrollText2);
  Screen.AddSprite(&SprPlasma);

  Screen.AddSprite(&SSpr_direction);
  Screen.AddSprite(&SSpr_XY);
  Screen.AddSprite(&SSpr_FCTX);

  int logo_x;
  int logo_y;

  float time_x = 10 * 12;

  // NOTE: -- CHAPTER 0 ---------------------------------- MAIN LOOP INIT ------

  CHAPTER = 0;
  DP.setDmp(siddmp, siddmp_len);
  DP.dmp_idx = 0;
  // NOTE: -- start audio processing thread --

  DP.play();
  std::thread audioThread(audioprocessing_tread);

  TICK = 0;
  stop = 0;
  KDEBOUNCE_CTR = 0;

  term_clear();
  term_echo(E_DISABLE);

  KBCtx_t kb_ctx;
  term_kbd_init(&kb_ctx);

  SprPlasma.SetXY(15, 13);
  SprPlasma.SetXY(10, 13);
  SprPlasma.SetXY(10, 3);
  SprScrollText1.SetXY(Screen.Width() + 140, 8);
  SprScrollText2.SetXY(Screen.Width() + 1200, 48);
  render_surface_copy(SprPlasma.out_surface, &backup_surface);

  // NOTE: -- CHAPTER 0 ---------------------------------- MAIL LOOP -----------

  while ((CHAPTER == 0) && (!stop)) {
    fps_begin_frame(&ctx);
    TICK++;
    TICK_ONGOING++;

    // -- chk keyboard --

    if (KDEBOUNCE_CTR)
      KDEBOUNCE_CTR--;
    if (!KDEBOUNCE_CTR) {
      // clear direction display
      SSpr_direction.frame_idx = 0;
    }

    term_nonblock(NB_ENABLE, &kb_ctx);
    if (term_kbhit()) {
      KEY = term_readkey();
      // printf("-- KEYs: %08x\n", KEY);
      switch (KEY) {
      case 'q':
      case KBD_KEY_SPACE:
      case KBD_KEY_ESC:
        SSpr_direction.frame_idx = 5;
        KDEBOUNCE_CTR = KDEBOUNCE_TICKS;
        stop = 1;
        break;
      case KBD_KEY_UP:
        SprGhost1.out_surface->y = SprGhost1.out_surface->y - 1;
        SprGhost1.y = SprGhost1.out_surface->y;
        SSpr_direction.frame_idx = 1;
        KDEBOUNCE_CTR = KDEBOUNCE_TICKS;
        break;
      case KBD_KEY_DOWN:
        SprGhost1.out_surface->y = SprGhost1.out_surface->y + 1;
        SprGhost1.y = SprGhost1.out_surface->y;
        SSpr_direction.frame_idx = 3;
        KDEBOUNCE_CTR = KDEBOUNCE_TICKS;
        break;
      case KBD_KEY_LEFT:
        SprGhost1.out_surface->x = SprGhost1.out_surface->x - 1;
        SprGhost1.x = SprGhost1.out_surface->x;
        SSpr_direction.frame_idx = 4;
        KDEBOUNCE_CTR = KDEBOUNCE_TICKS;
        if (PlasmaGhost.direction != Ghost_Left) {
          PlasmaGhost.direction = Ghost_Left;
          SprGhost1.StopAnimation();
          SprGhost1.StartAnimation(Ghost_Left);
        }
        break;
      case KBD_KEY_RIGHT:
        SprGhost1.out_surface->x = SprGhost1.out_surface->x + 1;
        SprGhost1.x = SprGhost1.out_surface->x;
        SSpr_direction.frame_idx = 2;
        KDEBOUNCE_CTR = KDEBOUNCE_TICKS;
        if (PlasmaGhost.direction != Ghost_Right) {
          PlasmaGhost.direction = Ghost_Right;
          SprGhost1.StopAnimation();
          SprGhost1.StartAnimation(Ghost_Right);
        }
        break;
      case 'k':
        SprGhost1.out_surface->y = SprGhost1.out_surface->y - 1;
        SprGhost1.y = SprGhost1.out_surface->y;
        SSpr_direction.frame_idx = 1;
        KDEBOUNCE_CTR = KDEBOUNCE_TICKS;
        break;
      case 'j':
        SprGhost1.out_surface->y = SprGhost1.out_surface->y + 1;
        SprGhost1.y = SprGhost1.out_surface->y;
        SSpr_direction.frame_idx = 3;
        KDEBOUNCE_CTR = KDEBOUNCE_TICKS;
        break;
      case 'h':
        SprGhost1.out_surface->x = SprGhost1.out_surface->x - 1;
        SprGhost1.x = SprGhost1.out_surface->x;
        SSpr_direction.frame_idx = 4;
        KDEBOUNCE_CTR = KDEBOUNCE_TICKS;
        if (PlasmaGhost.direction != Ghost_Left) {
          PlasmaGhost.direction = Ghost_Left;
          SprGhost1.StopAnimation();
          SprGhost1.StartAnimation(Ghost_Left);
        }
        break;
      case 'l':
        SprGhost1.out_surface->x = SprGhost1.out_surface->x + 1;
        SprGhost1.x = SprGhost1.out_surface->x;
        SSpr_direction.frame_idx = 2;
        KDEBOUNCE_CTR = KDEBOUNCE_TICKS;
        if (PlasmaGhost.direction != Ghost_Right) {
          PlasmaGhost.direction = Ghost_Right;
          SprGhost1.StopAnimation();
          SprGhost1.StartAnimation(Ghost_Right);
        }
        break;
      case 'r':
        ctx.min = 9999999999;
        ctx.max = 0;
        ctx.overruns = 0;
      default:
        break;
      }
      sprintf(SSpr_XY_txtbuf, "[ X: %d, Y: %d ]", SprM64.x, SprM64.y);
    }
    term_nonblock(NB_DISABLE, &kb_ctx);

    // -- ctrl-c handler, play music, wait 4 end of frame --
    if (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        // term_close();
        cursor_on();
        printf("\n[MAIN ] ctrl-c hit, exitting ...\n");
        stop = 1;
        break;
      default:
        break;
      }
    }

    logo_x =
        lrint((double)20.0 *
                  (double)sin((double)(TICK_ONGOING % (int)time_x) /
                              (double)time_x * (double)M_PI * (double)2.0) +
              (double)20.0 + 0.5);

    SprGhost1.AnimationTick();

    plasma2_ctx.time_offset++;

    //  NOTE: -- scene1 --
    if (scene_check_at_scene_start(&scene1, TICK) ||
        scene_check_in_scene(&scene1, TICK)) {
      tsfx_outline_rotor_tick(SprM64.restore_surface, 0, 0, TS_DIRECTION_LEFT);
      tsfx_dim_bgcolor(SprM64.restore_surface, TICK, scene1.duration,
                       SprM64.out_surface, Screen.bg_color);

      render_surface_copy(&backup_surface, SprPlasma.out_surface);
      tsfx_plasma_generate_plasma(&plasma2_ctx, SprPlasma.out_surface);
      tsfx_dim_direct(SprPlasma.out_surface, TICK, scene1.duration);
      SprGhost1.SetXY(40, 100 - ts_ease_in_out(TICK * 2, scene1.duration, 70));
    }

    //  NOTE: -- scene2 --
    if (scene_check_at_scene_start(&scene2, TICK)) {
      render_surface_copy(SprM64.restore_surface, SprM64.out_surface);
    }

    if (scene_check_in_scene_full(&scene2, TICK)) {
      tsfx_outline_rotor_tick(SprM64.out_surface, 0, 0, TS_DIRECTION_LEFT);
      plasma_ctx.time_offset++;
      tsfx_plasma_generate_plasma(&plasma_ctx, SprM64.out_surface);
      tsfx_plasma_generate_plasma(&plasma2_ctx, SprPlasma.out_surface);
    }

    //  NOTE: -- scene3 --
    if (scene_check_at_scene_start(&scene3, TICK)) {
    }

    if (scene_check_in_scene_full(&scene3, TICK)) {
      tsfx_outline_rotor_tick(SprM64.out_surface, 0, 0, TS_DIRECTION_RIGHT);
      color_fill = pal_fade1->colors[(TICK >> 0) % pal_fade1->num_colors];
      tsfx_color_fill(SprM642.restore_surface, &color_replace1, &color_fill,
                      SprM64.out_surface);

      color_fill = pal_fade2->colors[(TICK >> 0) % pal_fade2->num_colors];
      tsfx_color_fill(SprM642.restore_surface, &color_replace2, &color_fill,
                      SprM64.out_surface);

      color_fill = pal_fade3->colors[(TICK >> 0) % pal_fade3->num_colors];
      tsfx_color_fill(SprM642.restore_surface, &color_replace3, &color_fill,
                      SprM64.out_surface);
      tsfx_plasma_generate_plasma(&plasma2_ctx, SprPlasma.out_surface);
    }

    if (scene_check_at_scene_end(&scene3, TICK)) {
      TICK = scene2.begin; // skip scene2 start !
    }
    //  NOTE: -- ------ --

    if (logo_x == 0)
      logo_x = 1;
    logo_y = 56;
    SprM64.SetXY(logo_x + 15, logo_y + 1);

    SprScrollText1.SetXY(SprScrollText1.x - 1, 8 - 8 + 0);
    if (SprScrollText1.x < (-1 * SprScrollText1.w - 200))
      SprScrollText1.x = Screen.Width() + 40;

    SprScrollText2.SetXY(SprScrollText2.x - 2, 48 - 8 - 4 + 1);
    if (SprScrollText2.x < (-1 * SprScrollText2.w - 200))
      SprScrollText2.x = Screen.Width();

    sprintf(SSpr_XY_txtbuf, "[ X: %d, Y: %d ]", SprM64.x, SprM64.y);
    Screen.Render();

    fps_end_frame(&ctx);
    sprintf(SSpr_FCTX_txtbuf, "[ orr: %d, min/max: %lu / %lu, dT: %lu ]",
            ctx.overruns, ctx.min, ctx.max, ctx.ts2 - ctx.ts1);
  }

  // NOTE: -----------------------------------------------------------   END   -

  DP.stop();
  audioThread.join();

  term_echo(E_ENABLE);
  cursor_on();
  term_close();
  printf("[MAIN ] %s\n", SSpr_XY_txtbuf);
  printf("[MAIN ] END @ %lu.\n", TICK);

  return 0;
}
