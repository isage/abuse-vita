/*
 *  Abuse - dark 2D side-scrolling platform game
 *  Copyright (c) 1995 Crack dot Com
 *  Copyright (c) 2005-2011 Sam Hocevar <sam@hocevar.net>
 *  Copyright (c) 2016 Antonio Radojkovic <antonior.software@gmail.com>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include "cache.h"
#include "common.h"
#include "demo.h"
#include "dev.h"
#include "dprint.h"
#include "fonts.h"
#include "game.h"
#include "gui.h"
#include "id.h"
#include "input.h"
#include "jwindow.h"
#include "lisp.h"
#include "specs.h"

#include <string.h>

// AR
#include "sdlport/setup.h"
extern Settings settings;
extern int get_key_binding(char const *dir, int i);
//

extern void *save_order; // load from "saveordr.lsp", contains a list ordering the save games

extern JCFont *console_font;

#define MAX_SAVE_GAMES 15
#define MAX_SAVE_LINES 5
int last_save_game_number = 0;

int save_buts[MAX_SAVE_GAMES * 3];

void load_number_icons()
{
  for (int i = 0; i < MAX_SAVE_GAMES * 3; i++)
  {
    char name[100];
    sprintf(name, "nums%04d.pcx", i + 1);
    save_buts[i] = cache.reg("art/icons.spe", name, SPEC_IMAGE, 1);
  }
}

void last_savegame_name(char *buf)
{
  printf("last_savegame_name()\n");
  sprintf(buf, "%ssave%04d.spe", get_save_filename_prefix(),
          (last_save_game_number + MAX_SAVE_GAMES - 1) % MAX_SAVE_GAMES + 1);
}

Jwindow *create_num_window(int mx, int total_saved, int lines, image **thumbnails)
{
  ico_button *buts[MAX_SAVE_GAMES];
  int y = 0, x = 0, i;
  int iw    = cache.img(save_buts[0])->Size().x;
  int ih    = cache.img(save_buts[0])->Size().y;
  int maxih = ih, maxiw = iw;
  int n = 0;
  for (i = 0; i < total_saved; i++, y += ih)
  {
    maxih = Max(ih, maxih);
    maxiw = Max(iw, maxiw);
    if (y >= lines * ih)
    {
      y = 0;
      x += iw;
    }
    if (thumbnails)
    {
      while (!thumbnails[n])
        n++;
    }
    buts[i] = new ico_button(x, y, ID_LOAD_GAME_NUMBER + n, save_buts[n * 3 + 0], save_buts[n * 3 + 0],
                             save_buts[n * 3 + 1], save_buts[n * 3 + 2], NULL);
    buts[i]->set_act_id(ID_LOAD_GAME_PREVIEW + n);
    n++;
  }

  for (i = 0; i < total_saved - 1; i++)
    buts[i]->next = buts[i + 1];

  Jwindow *l_win = wm->CreateWindow(ivec2(mx, yres / 2 - (Jwindow::top_border() + maxih * 5) / 2), ivec2(-1), buts[0]);

  return l_win;
}

int get_save_spot()
{
  int i = MAX_SAVE_GAMES, last_free = 0;
  for (; i > 0;)
  {
    char name[20];
    sprintf(name, "%ssave%04d.spe", get_save_filename_prefix(), i);
    FILE *fp = open_FILE(name, "rb");
    if (fp)
      i = 0;
    else
    {
      last_free = i;
      i--;
    }
    fclose(fp);
  }

  if (last_free)
    return last_free; // if there are any slots not created yet...

  int w  = cache.img(save_buts[0])->Size().x;
  int mx = last_demo_mpos.x - w / 2;
  if (mx + w + 10 > xres)
    mx = xres - w - 10;
  if (mx < 0)
    mx = 0;

  Jwindow *l_win = create_num_window(mx, MAX_SAVE_GAMES, MAX_SAVE_LINES, NULL);
  Event ev;
  int got_level = 0;
  int quit      = 0;
  do
  {
    wm->flush_screen();
    wm->get_event(ev);
    if (ev.type == EV_MESSAGE && ev.message.id >= ID_LOAD_GAME_NUMBER && ev.message.id < ID_LOAD_GAME_PREVIEW)
      got_level = ev.message.id - ID_LOAD_GAME_NUMBER + 1;

    if (ev.type == EV_CLOSE_WINDOW && ev.window == l_win)
      quit = 1;
  } while (!got_level && !quit);

  wm->close_window(l_win);
  the_game->reset_keymap();
  return got_level;
}

void get_savegame_name(char *buf) // buf should be at least 50 bytes
{
  sprintf(buf, "save%04d.spe", (last_save_game_number++) % MAX_SAVE_GAMES + 1);
  /*  FILE *fp=open_FILE("lastsave.lsp","wb");
    if (fp)
    {
      fprintf(fp,"(setq last_save_game %d)\n",last_save_game_number%MAX_SAVE_GAMES);
      fclose(fp);
    } else dprintf("Warning unable to open lastsave.lsp for writing\n"); */
}

int show_load_icon()
{
  int i;
  for (i = 0; i < MAX_SAVE_GAMES; i++)
  {
    char nm[255];
    sprintf(nm, "%ssave%04d.spe", get_save_filename_prefix(), i + 1);
    bFILE *fp = open_file(nm, "rb");
    if (fp->open_failure())
    {
      delete fp;
    }
    else
    {
      delete fp;
      return 1;
    }
  }
  return 0;
}

int load_game(int show_all,
              char const *title) // return 0 if the player escapes, else return the number of the game to load
{
  // AR this creates the small load/save game window
  // and takes complete control of the program until it leaves the loop
  // it is called via clisp.cpp (case 263)

  int total_saved = 0;
  image *thumbnails[MAX_SAVE_GAMES];
  int start_num = 0;
  int max_w = 160, max_h = 100;
  memset(thumbnails, 0, sizeof(thumbnails));

  image *first = NULL;

  for (start_num = 0; start_num < MAX_SAVE_GAMES; start_num++)
  {
    char name[255];
    int fail = 0;

    sprintf(name, "%ssave%04d.spe", get_save_filename_prefix(), start_num + 1);
    bFILE *fp = open_file(name, "rb");
    if (fp->open_failure())
    {
      fail = 1;
    }
    else
    {
      spec_directory sd(fp);
      spec_entry *se = sd.find("thumb nail");
      if (se && se->type == SPEC_IMAGE)
      {
        thumbnails[start_num] = new image(fp, se);
        if (thumbnails[start_num]->Size().x > max_w)
          max_w = thumbnails[start_num]->Size().x;
        if (thumbnails[start_num]->Size().y > max_h)
          max_h = thumbnails[start_num]->Size().y;
        if (!first)
          first = thumbnails[start_num];
        total_saved++;
      }
      else
        fail = 1;
    }
    if (fail && show_all)
    {
      thumbnails[start_num] = new image(ivec2(160, 100));
      thumbnails[start_num]->clear();
      console_font->PutString(thumbnails[start_num], ivec2(0), symbol_str("no_saved"));
      total_saved++;
      if (!first)
        first = thumbnails[start_num];
    }
    delete fp;
  }

  if (!total_saved)
    return 0;
  if (total_saved > MAX_SAVE_GAMES)
    total_saved = MAX_SAVE_GAMES;

  int i;
  /*  int ih=cache.img(save_buts[0])->Size().y;
    ico_button *buts[MAX_SAVE_GAMES];
    int y=0;


    for (i=0; i<total_saved; i++,y+=ih)
    {
      buts[i]=new ico_button(0,y,ID_LOAD_GAME_NUMBER+i,
                 save_buts[i*3+1],save_buts[i*3+1],save_buts[i*3+0],save_buts[i*3+2],NULL);
      buts[i]->set_act_id(ID_LOAD_GAME_PREVIEW+i);
    }

    for (i=0; i<total_saved-1; i++)
      buts[i]->next=buts[i+1];
  */

  // Create thumbnail window 5 pixels to the right of the list window
  Jwindow *l_win   = create_num_window(0, total_saved, MAX_SAVE_LINES, thumbnails);
  Jwindow *preview = wm->CreateWindow(l_win->m_pos + ivec2(l_win->m_size.x + 5, 0), ivec2(max_w, max_h), NULL, title);

  preview->m_surf->PutImage(first, ivec2(preview->x1(), preview->y1()));

  // AR let me know we are stuck here
  the_game->ar_stateold = the_game->ar_state;
  the_game->ar_state    = AR_LOADSAVE;

  // AR controller ui movement, number icon size 30x25
  static int button_w = 30;
  static int button_h = 25;
  int mx, my; // mouse position

  int old_mx = wm->GetMousePos().x;
  int old_my = wm->GetMousePos().y;

  // AR initial position of the mouse in the window for controller use
  if (settings.ctr_aim)
  {
    mx = l_win->m_pos.x + button_w / 2;
    my = l_win->m_pos.y + button_h / 2;
    wm->SetMousePos(ivec2(mx, my));
  }
  //

  Event ev;
  int got_level = 0;
  int quit      = 0;
  int old_draw_num = 0;
  do
  {
    wm->flush_screen();
    wm->get_event(ev);
    if (ev.type == EV_MESSAGE && ev.message.id >= ID_LOAD_GAME_NUMBER && ev.message.id < ID_LOAD_GAME_PREVIEW)
      got_level = ev.message.id - ID_LOAD_GAME_NUMBER + 1;

    if (ev.type == EV_MESSAGE && ev.message.id >= ID_LOAD_GAME_PREVIEW && ev.message.id < ID_LOAD_PLAYER_GAME)
    {
      int draw_num = ev.message.id - ID_LOAD_GAME_PREVIEW;
      if (old_draw_num != draw_num)
      {
        old_draw_num = draw_num;
        preview->clear();
        preview->m_surf->PutImage(thumbnails[draw_num], ivec2(preview->x1(), preview->y1()));
      }
    }

    if ((ev.type == EV_CLOSE_WINDOW) || (ev.type == EV_KEY && ev.key == JK_ESC))
      quit = 1;

    // AR move cursor over icons
    if (ev.type == EV_KEY)
    {
      if ((ev.key == get_key_binding("left", 0) || ev.key == get_key_binding("left2", 0)))
      {
        if (mx - button_w > l_win->m_pos.x)
          mx -= button_w;
        wm->SetMousePos(ivec2(mx, my));
      }
      if ((ev.key == get_key_binding("right", 0) || ev.key == get_key_binding("right2", 0)))
      {
        if (mx + button_w < l_win->m_pos.x + l_win->m_size.x)
          mx += button_w;
        wm->SetMousePos(ivec2(mx, my));
      }
      if ((ev.key == get_key_binding("up", 0) || ev.key == get_key_binding("up2", 0)))
      {
        if (my - button_h > l_win->m_pos.y)
          my -= button_h;
        wm->SetMousePos(ivec2(mx, my));
      }
      if ((ev.key == get_key_binding("down", 0) || ev.key == get_key_binding("down2", 0)))
      {
        if (my + button_h < l_win->m_pos.y + l_win->m_size.y)
          my += button_h;
        wm->SetMousePos(ivec2(mx, my));
      }
    }
    //

  } while (!got_level && !quit);

  // AR let me know we leaving
  the_game->ar_state = the_game->ar_stateold;
  if (settings.ctr_aim)
    wm->SetMousePos(ivec2(old_mx, old_my)); // put mouse where it was on entering
  //

  wm->close_window(l_win);
  wm->close_window(preview);

  for (i = 0; i < total_saved; i++)
    delete thumbnails[i];

  wm->ResetMouseButtons();

  return got_level;
}
