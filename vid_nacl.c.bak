// vid_sdl.h -- sdl video driver

#include "quakedef.h"
#include "d_local.h"

#if defined(STANDALONE)
#include "native_client/common/standalone.h"
#else
#include <nacl/nacl_av.h>
#endif

viddef_t    vid;                // global video state
unsigned short  d_8to16table[256];

// default resolution
int BASEWIDTH = 800;
int BASEHEIGHT = 600;

// remap table for emulating palettes
unsigned int gPalette[256];
#define MAKE_RGBA(r, g, b, a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

int    VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes = 0;
byte    *VGA_pagebase;

struct NACL_Color
{
  char r;
  char g;
  char b;
  char unused;
};

struct NACL_Surface
{
  int w;
  int h;
  int pitch;
  int flags;
  void *pixels;
  struct NACL_Color palette[256];
};


static struct NACL_Surface *screen = NULL;
static unsigned int *pixels32 = NULL;

static qboolean mouse_avail;
static float   mouse_x, mouse_y;
static int mouse_oldbuttonstate = 0;

// No support for option menus
void (*vid_menudrawfn)(void) = NULL;
void (*vid_menukeyfn)(int key) = NULL;

void    VID_SetPalette (unsigned char *palette)
{
    int i;
    /* emulate palette in software */
    if (screen) {
          struct NACL_Color *screen_colors = screen->palette;
          for ( i = 0; i < 256; ++i ) {
              screen_colors[i].r = *palette++;
              screen_colors[i].g = *palette++;
              screen_colors[i].b = *palette++;
              screen_colors[i].unused = 255;
          }
    }
}

void    VID_ShiftPalette (unsigned char *palette)
{
    VID_SetPalette(palette);
}

void    VID_Init (unsigned char *palette)
{
    int pnum, chunk;
    byte *cache;
    int cachesize;
    unsigned char video_bpp;
    unsigned short video_w, video_h;
    unsigned int flags;

    // if this call succeeds, use width & height from embedded html
    {
      int width;
      int height;
      int r = nacl_multimedia_get_embed_size(&width, &height);
      if (0 == r) {
          BASEWIDTH = width;
          BASEHEIGHT = height;
      }
    }

    // Set up display mode (width and height)
    vid.width = BASEWIDTH;
    vid.height = BASEHEIGHT;
    vid.maxwarpwidth = WARP_WIDTH;
    vid.maxwarpheight = WARP_HEIGHT;
    nacl_video_init(vid.width, vid.height);

    // Set up a fake 8bit 'screen' that quake will draw into
    // It will be our job to manually blit 8bit -> 32bit
    screen = malloc(sizeof(struct NACL_Surface));
    memset(screen, 0, sizeof(struct NACL_Surface));
    screen->flags = 0;
    screen->w = BASEWIDTH;
    screen->h = BASEHEIGHT;
    screen->pitch = BASEWIDTH;
    screen->pixels = malloc(BASEWIDTH * BASEHEIGHT * sizeof(byte));
    memset(screen->pixels, 0, BASEWIDTH * BASEHEIGHT * sizeof(byte));

    // Set up a staging area of 32bit pixels that we'll use to up-convert
    // (in theory we could use SDL to do this, but we'd have to expose
    // more of the SDL API via trampoline)
    pixels32 = malloc(BASEWIDTH * BASEHEIGHT * sizeof(unsigned int));

    VID_SetPalette(palette);

    // now know everything we need to know about the buffer
    VGA_width = vid.conwidth = vid.width;
    VGA_height = vid.conheight = vid.height;
    vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
    vid.numpages = 1;
    vid.colormap = host_colormap;
    vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
    VGA_pagebase = vid.buffer = screen->pixels;
    VGA_rowbytes = vid.rowbytes = screen->pitch;
    vid.conbuffer = vid.buffer;
    vid.conrowbytes = vid.rowbytes;
    vid.direct = 0;

    // allocate z buffer and surface cache
    chunk = vid.width * vid.height * sizeof (*d_pzbuffer);
    cachesize = D_SurfaceCacheForRes (vid.width, vid.height);
    chunk += cachesize;
    d_pzbuffer = Hunk_HighAllocName(chunk, "video");
    if (d_pzbuffer == NULL)
        Sys_Error ("Not enough memory for video mode\n");

    // initialize the cache memory
        cache = (byte *) d_pzbuffer
                + vid.width * vid.height * sizeof (*d_pzbuffer);
    D_InitCaches (cache, cachesize);
}

static void VID_FreeSurface(struct NACL_Surface *surf) {
  if (NULL != surf) {
    if (surf->pixels) {
      free(surf->pixels);
    }
    free(surf);
  }
}

void    VID_Shutdown (void)
{
    VID_FreeSurface(screen);
    if (pixels32) {
      free(pixels32);
    }
    screen = NULL;
    pixels32 = NULL;
    nacl_video_shutdown();
    nacl_multimedia_shutdown();
}

static void UpdateAll() {
  // just copy the whole darn thing
  int i;
  byte *src = screen->pixels;
  unsigned int *dst = pixels32;
  unsigned int colors[256];
  // convert palette to our real screen format
  for (i = 0; i < 256; ++i) {
    byte r = screen->palette[i].r;
    byte g = screen->palette[i].g;
    byte b = screen->palette[i].b;
    byte a = 255;
    colors[i] = MAKE_RGBA(r, g, b, a);
  }
  // convert quake's 8bit output to 32bit raw bgra pixel data
  for (i = 0; i < screen->w * screen->h; ++i) {
    byte b = *src++;
    unsigned int c = colors[b];
    *dst++ = c;
  }
  // call SDL via trampoline with raw bgra pixel data
  nacl_video_update(pixels32);
}

void    VID_Update (vrect_t *rects)
{
    UpdateAll();
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
    unsigned char *offset;


    if (!screen) return;
    if ( x < 0 ) x = screen->w+x-1;
    offset = (unsigned char *)screen->pixels + y*screen->pitch + x;
    while ( height-- )
    {
        memcpy(offset, pbitmap, width);
        offset += screen->pitch;
        pbitmap += width;
    }
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
    if (!screen) return;
    if (x < 0) x = screen->w+x-1;
    // update the whole screen
    UpdateAll();
}


/*
================
Sys_SendKeyEvents
================
*/

void Sys_SendKeyEvents(void)
{
    union NaClMultimediaEvent event;
    int sym, state;
    int modstate;
    while (0 == nacl_video_poll_event(&event))
    {
        switch (event.type) {

            case NACL_EVENT_KEY_DOWN:
            case NACL_EVENT_KEY_UP:
                sym = event.key.keysym.sym;
                state = event.key.state;
                modstate = event.key.keysym.mod;

                //printf("sym: %d, state: %d, modstate: %d\n", sym, state, modstate);
                //printf("up: %d, down: %d, left: %d, right: %d\n", NACL_KEY_UP, NACL_KEY_DOWN, NACL_KEY_LEFT, NACL_KEY_RIGHT);

                switch(sym)
                {
                   case NACL_KEY_DELETE: sym = K_DEL; break;
                   case NACL_KEY_BACKSPACE: sym = K_BACKSPACE; break;
                   case NACL_KEY_F1: sym = K_F1; break;
                   case NACL_KEY_F2: sym = K_F2; break;
                   case NACL_KEY_F3: sym = K_F3; break;
                   case NACL_KEY_F4: sym = K_F4; break;
                   case NACL_KEY_F5: sym = K_F5; break;
                   case NACL_KEY_F6: sym = K_F6; break;
                   case NACL_KEY_F7: sym = K_F7; break;
                   case NACL_KEY_F8: sym = K_F8; break;
                   case NACL_KEY_F9: sym = K_F9; break;
                   case NACL_KEY_F10: sym = K_F10; break;
                   case NACL_KEY_F11: sym = K_F11; break;
                   case NACL_KEY_F12: sym = K_F12; break;
                   case NACL_KEY_BREAK:
                   case NACL_KEY_PAUSE: sym = K_PAUSE; break;
                   case NACL_KEY_UP: sym = K_UPARROW; break;
                   case NACL_KEY_DOWN: sym = K_DOWNARROW; break;
                   case NACL_KEY_RIGHT: sym = K_RIGHTARROW; break;
                   case NACL_KEY_LEFT: sym = K_LEFTARROW; break;
                   case NACL_KEY_INSERT: sym = K_INS; break;
                   case NACL_KEY_HOME: sym = K_HOME; break;
                   case NACL_KEY_END: sym = K_END; break;
                   case NACL_KEY_PAGEUP: sym = K_PGUP; break;
                   case NACL_KEY_PAGEDOWN: sym = K_PGDN; break;
                   case NACL_KEY_RSHIFT:
                   case NACL_KEY_LSHIFT: sym = K_SHIFT; break;
                   case NACL_KEY_RCTRL:
                   case NACL_KEY_LCTRL: sym = K_CTRL; break;
                   case NACL_KEY_RALT:
                   case NACL_KEY_LALT: sym = K_ALT; break;
                   case NACL_KEY_KP0:
                       if(modstate & NACL_KEYMOD_NUM) sym = K_INS;
                       else sym = NACL_KEY_0;
                       break;
                   case NACL_KEY_KP1:
                       if(modstate & NACL_KEYMOD_NUM) sym = K_END;
                       else sym = NACL_KEY_1;
                       break;
                   case NACL_KEY_KP2:
                       if(modstate & NACL_KEYMOD_NUM) sym = K_DOWNARROW;
                       else sym = NACL_KEY_2;
                       break;
                   case NACL_KEY_KP3:
                       if(modstate & NACL_KEYMOD_NUM) sym = K_PGDN;
                       else sym = NACL_KEY_3;
                       break;
                   case NACL_KEY_KP4:
                       if(modstate & NACL_KEYMOD_NUM) sym = K_LEFTARROW;
                       else sym = NACL_KEY_4;
                       break;
                   case NACL_KEY_KP5: sym = NACL_KEY_5; break;
                   case NACL_KEY_KP6:
                       if(modstate & NACL_KEYMOD_NUM) sym = K_RIGHTARROW;
                       else sym = NACL_KEY_6;
                       break;
                   case NACL_KEY_KP7:
                       if(modstate & NACL_KEYMOD_NUM) sym = K_HOME;
                       else sym = NACL_KEY_7;
                       break;
                   case NACL_KEY_KP8:
                       if(modstate & NACL_KEYMOD_NUM) sym = K_UPARROW;
                       else sym = NACL_KEY_8;
                       break;
                   case NACL_KEY_KP9:
                       if(modstate & NACL_KEYMOD_NUM) sym = K_PGUP;
                       else sym = NACL_KEY_9;
                       break;
                   case NACL_KEY_KP_PERIOD:
                       if(modstate & NACL_KEYMOD_NUM) sym = K_DEL;
                       else sym = NACL_KEY_PERIOD;
                       break;
                   case NACL_KEY_KP_DIVIDE: sym = NACL_KEY_SLASH; break;
                   case NACL_KEY_KP_MULTIPLY: sym = NACL_KEY_ASTERISK; break;
                   case NACL_KEY_KP_MINUS: sym = NACL_KEY_MINUS; break;
                   case NACL_KEY_KP_PLUS: sym = NACL_KEY_PLUS; break;
                   case NACL_KEY_KP_ENTER: sym = NACL_KEY_RETURN; break;
                   case NACL_KEY_KP_EQUALS: sym = NACL_KEY_EQUALS; break;
                }
                // If we're not directly handled and still above 255
                // just force it to 0
                if(sym > 255) sym = 0;
                Key_Event(sym, state);
                break;

            case NACL_EVENT_MOUSE_MOTION:
                if ( (event.motion.x != (vid.width/2)) ||
                     (event.motion.y != (vid.height/2)) ) {
                    mouse_x = event.motion.xrel*10;
                    mouse_y = event.motion.yrel*10;
                    if ( (event.motion.x < ((vid.width/2)-(vid.width/4))) ||
                         (event.motion.x > ((vid.width/2)+(vid.width/4))) ||
                         (event.motion.y < ((vid.height/2)-(vid.height/4))) ||
                         (event.motion.y > ((vid.height/2)+(vid.height/4))) ) {
                        /* NaCl has no equivalent to:                */
                        /* SDL_WarpMouse(vid.width/2, vid.height/2); */
                    }
                }
                break;

            case NACL_EVENT_QUIT:
                CL_Disconnect ();
                Host_ShutdownServer(false);
                Sys_Quit ();
                break;
            default:
                break;
        }
    }
}

void IN_Init (void)
{
    if ( COM_CheckParm ("-nomouse") )
        return;
    mouse_x = mouse_y = 0.0;
    mouse_avail = 1;
}

void IN_Shutdown (void)
{
    mouse_avail = 0;
}

void IN_Commands (void)
{
    /* mouse events are processed in the event loop */
}

void IN_Move (usercmd_t *cmd)
{
    if (!mouse_avail)
        return;

    mouse_x *= sensitivity.value;
    mouse_y *= sensitivity.value;

    if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
        cmd->sidemove += m_side.value * mouse_x;
    else
        cl.viewangles[YAW] -= m_yaw.value * mouse_x;
    if (in_mlook.state & 1)
        V_StopPitchDrift ();

    if ( (in_mlook.state & 1) && !(in_strafe.state & 1)) {
        cl.viewangles[PITCH] += m_pitch.value * mouse_y;
        if (cl.viewangles[PITCH] > 80)
            cl.viewangles[PITCH] = 80;
        if (cl.viewangles[PITCH] < -70)
            cl.viewangles[PITCH] = -70;
    } else {
        if ((in_strafe.state & 1) && noclip_anglehack)
            cmd->upmove -= m_forward.value * mouse_y;
        else
            cmd->forwardmove -= m_forward.value * mouse_y;
    }
    mouse_x = mouse_y = 0.0;
}

/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
    return 0;
}
