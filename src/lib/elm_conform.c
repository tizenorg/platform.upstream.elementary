#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_conform.h"
#include "elm_widget_layout.h"

#define MY_CLASS ELM_CONFORMANT_CLASS

#define MY_CLASS_NAME "Elm_Conformant"
#define MY_CLASS_NAME_LEGACY "elm_conformant"

//TIZEN_ONLY(20160330): add processing properties of window
#define SWAP(x, y, t) ((t) = (x), (x) = (y), (y) = (t))
//

static char CONFORMANT_KEY[] = "_elm_conform_key";

#define ELM_CONFORM_INDICATOR_TIME 1.0

static const char INDICATOR_PART[] = "elm.swallow.indicator";
static const char VIRTUALKEYPAD_PART[] = "elm.swallow.virtualkeypad";
static const char CLIPBOARD_PART[] = "elm.swallow.clipboard";
static const char SOFTKEY_PART[] = "elm.swallow.softkey";

static const char SIG_VIRTUALKEYPAD_STATE_ON[] = "virtualkeypad,state,on";
static const char SIG_VIRTUALKEYPAD_STATE_OFF[] = "virtualkeypad,state,off";
static const char SIG_CLIPBOARD_STATE_ON[] = "clipboard,state,on";
static const char SIG_CLIPBOARD_STATE_OFF[] = "clipboard,state,off";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_VIRTUALKEYPAD_STATE_ON, ""},
   {SIG_VIRTUALKEYPAD_STATE_OFF, ""},
   {SIG_CLIPBOARD_STATE_ON, ""},
   {SIG_CLIPBOARD_STATE_OFF, ""},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"default", "elm.swallow.content"},
   {"icon", "elm.swallow.content"}, // TODO: deprecate this in elm 2.0
   {NULL, NULL}
};

// TIZEN_ONLY(20160218): Improve launching performance.
static Evas_Object *_precreated_conform_obj = NULL;
//

/* Example of env vars:
 * ILLUME_KBD="0, 0, 800, 301"
 * ILLUME_IND="0, 0, 800, 32"
 * ILLUME_STK="0, 568, 800, 32"
 */
static Eina_Bool
_conformant_part_geometry_get_from_env(const char *part,
                                       int *sx,
                                       int *sy,
                                       int *sw,
                                       int *sh)
{
   const char delimiters[] = " ,;";
   char *env_val, *token;
   char buf[PATH_MAX];
   int tsx, tsy, tsw;

   if (!(env_val = getenv(part))) return EINA_FALSE;

   /* strtok would modify env var if not copied to a buffer */
   strncpy(buf, env_val, sizeof(buf));
   buf[PATH_MAX - 1] = '\0';

   token = strtok(buf, delimiters);
   if (!token) return EINA_FALSE;
   tsx = atoi(token);

   token = strtok(NULL, delimiters);
   if (!token) return EINA_FALSE;
   tsy = atoi(token);

   token = strtok(NULL, delimiters);
   if (!token) return EINA_FALSE;
   tsw = atoi(token);

   token = strtok(NULL, delimiters);
   if (!token) return EINA_FALSE;
   *sh = atoi(token);

   *sx = tsx;
   *sy = tsy;
   *sw = tsw;

   return EINA_TRUE;
}

static void
_conformant_part_size_hints_set(Evas_Object *obj,
                                Evas_Object *sobj,
                                Evas_Coord sx,
                                Evas_Coord sy,
                                Evas_Coord sw,
                                Evas_Coord sh)
{
   Evas_Coord cx, cy, cw, ch;
   Evas_Coord part_height = 0, part_width = 0;

   evas_object_geometry_get(obj, &cx, &cy, &cw, &ch);

   /* Part overlapping with conformant */
   if ((cx < (sx + sw)) && ((cx + cw) > sx)
       && (cy < (sy + sh)) && ((cy + ch) > sy))
     {
        part_height = MIN((cy + ch), (sy + sh)) - MAX(cy, sy);
        part_width = MIN((cx + cw), (sx + sw)) - MAX(cx, sx);
     }

   evas_object_size_hint_min_set(sobj, part_width, part_height);
   evas_object_size_hint_max_set(sobj, part_width, part_height);
}

static void
_conformant_part_sizing_eval(Evas_Object *obj,
                             Conformant_Part_Type part_type)
{
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window zone = 0;
   Ecore_X_Window xwin;
#endif
// TIZEN_ONLY(20150707): implemented elm_win_conformant_set/get for wayland
#ifdef HAVE_ELEMENTARY_WAYLAND
   Ecore_Wl_Window *wlwin;
#endif
//
   Evas_Object *top;
   int sx = -1, sy = -1, sw = -1, sh = -1;

   ELM_CONFORMANT_DATA_GET(obj, sd);
   top = elm_widget_top_get(obj);

#ifdef HAVE_ELEMENTARY_X
   xwin = elm_win_xwindow_get(top);

   if (xwin)
     zone = ecore_x_e_illume_zone_get(xwin);
#endif

   if (part_type & ELM_CONFORMANT_INDICATOR_PART)
     {
#ifdef HAVE_ELEMENTARY_X
        if ((!_conformant_part_geometry_get_from_env
               ("ILLUME_IND", &sx, &sy, &sw, &sh)) && (xwin))
          {
             //No information of the indicator geometry, reset the geometry.
             if ((!zone) ||
                 (!ecore_x_e_illume_indicator_geometry_get
                   (zone, &sx, &sy, &sw, &sh)))
               sx = sy = sw = sh = 0;
          }
#endif
// TIZEN_ONLY(20150707): implemented elm_win_conformant_set/get for wayland
#ifdef HAVE_ELEMENTARY_WAYLAND
        wlwin = elm_win_wl_window_get(top);
        if (wlwin)
          ecore_wl_window_indicator_geometry_get(wlwin, &sx, &sy, &sw, &sh);
#endif
//
        if (((sd->rot == 90) || (sd->rot == 270)) && sd->landscape_indicator)
          _conformant_part_size_hints_set(obj, sd->landscape_indicator, sx, sy, sw, sh);
        else if (((sd->rot == 0) || (sd->rot == 180)) && sd->portrait_indicator)
          _conformant_part_size_hints_set(obj, sd->portrait_indicator, sx, sy, sw, sh);
     }

   if (part_type & ELM_CONFORMANT_VIRTUAL_KEYPAD_PART)
     {
#ifdef HAVE_ELEMENTARY_X
        if ((!_conformant_part_geometry_get_from_env
               ("ILLUME_KBD", &sx, &sy, &sw, &sh)) && (xwin))
          {
             //No information of the keyboard geometry, reset the geometry.
#ifdef __linux__
             DBG("[KEYPAD]:pid=%d, xwin=0x%x, zone=0x%x: no env value and check window property.", getpid(), xwin, zone);
#endif
             if (!ecore_x_e_illume_keyboard_geometry_get(xwin, &sx, &sy, &sw, &sh))
               {
                  DBG("[KEYPAD]:no window property, check zone property.");
                  if ((!zone) ||
                      (!ecore_x_e_illume_keyboard_geometry_get(zone, &sx, &sy, &sw, &sh)))
                    {
                       DBG("[KEYPAD]:no zone property, reset value.");
                       sx = sy = sw = sh = 0;
                    }
               }
          }
#endif
// TIZEN_ONLY(20150707): implemented elm_win_conformant_set/get for wayland
#ifdef HAVE_ELEMENTARY_WAYLAND
        wlwin = elm_win_wl_window_get(top);
        if (wlwin)
          ecore_wl_window_keyboard_geometry_get(wlwin, &sx, &sy, &sw, &sh);
        Evas_Coord tmp = 0;
        Evas_Coord ww = 0, wh = 0;
        elm_win_screen_size_get(top, NULL, NULL, &ww, &wh);
        if (sd->rot == 90)
          {
             SWAP(sx, sy, tmp);
             SWAP(sw, sh, tmp);
          }
        if (sd->rot == 180)
          {
             sy = wh - sh;
          }
        if (sd->rot == 270)
          {
             sy = ww - sw;
             SWAP(sw, sh, tmp);
          }
#endif
//
        DBG("[KEYPAD]: size(%d,%d, %dx%d).", sx, sy, sw, sh);
        if (sd->virtualkeypad)
          _conformant_part_size_hints_set
             (obj, sd->virtualkeypad, sx, sy, sw, sh);
     }

   if (part_type & ELM_CONFORMANT_SOFTKEY_PART)
     {
#ifdef HAVE_ELEMENTARY_X
        if ((!_conformant_part_geometry_get_from_env
               ("ILLUME_STK", &sx, &sy, &sw, &sh)) && (xwin))
          {
             //No information of the softkey geometry, reset the geometry.
             if ((!zone) ||
                 (!ecore_x_e_illume_softkey_geometry_get
                     (zone, &sx, &sy, &sw, &sh)))
               sx = sy = sw = sh = 0;
          }
#endif
        if (sd->softkey)
          _conformant_part_size_hints_set(obj, sd->softkey, sx, sy, sw, sh);
     }
   if (part_type & ELM_CONFORMANT_CLIPBOARD_PART)
     {
#ifdef HAVE_ELEMENTARY_X
        if ((!_conformant_part_geometry_get_from_env
               ("ILLUME_CB", &sx, &sy, &sw, &sh)) && (xwin))
          {
             //No information of the clipboard geometry, reset the geometry.
             if ((!zone) ||
                 (!ecore_x_e_illume_clipboard_geometry_get
                   (zone, &sx, &sy, &sw, &sh)))
               sx = sy = sw = sh = 0;
          }
#endif
        if (sd->clipboard)
          _conformant_part_size_hints_set(obj, sd->clipboard, sx, sy, sw, sh);
     }
}

static void
_conformant_parts_swallow(Evas_Object *obj)
{
   Evas *e;
   ELM_CONFORMANT_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   e = evas_object_evas_get(obj);

   sd->scroller = NULL;

   //Virtual Keyboard
   if (edje_object_part_exists(wd->resize_obj, VIRTUALKEYPAD_PART))
     {
        if (!sd->virtualkeypad)
          {
             sd->virtualkeypad = evas_object_rectangle_add(e);
             elm_widget_sub_object_add(obj, sd->virtualkeypad);
             evas_object_size_hint_max_set(sd->virtualkeypad, -1, 0);
          }
        else
          _conformant_part_sizing_eval(obj, ELM_CONFORMANT_VIRTUAL_KEYPAD_PART);

        evas_object_color_set(sd->virtualkeypad, 0, 0, 0, 0);
        elm_layout_content_set(obj, VIRTUALKEYPAD_PART, sd->virtualkeypad);
     }
   else
     ELM_SAFE_FREE(sd->virtualkeypad, evas_object_del);

   //Clipboard
   if (edje_object_part_exists(wd->resize_obj, CLIPBOARD_PART))
     {
        if (!sd->clipboard)
          {
             sd->clipboard = evas_object_rectangle_add(e);
             evas_object_size_hint_min_set(sd->clipboard, -1, 0);
             evas_object_size_hint_max_set(sd->clipboard, -1, 0);
          }
        else
          _conformant_part_sizing_eval(obj, ELM_CONFORMANT_CLIPBOARD_PART);

        evas_object_color_set(sd->clipboard, 0, 0, 0, 0);
        elm_layout_content_set(obj, CLIPBOARD_PART, sd->clipboard);
     }
   else
     ELM_SAFE_FREE(sd->clipboard, evas_object_del);

   //Softkey
   if (edje_object_part_exists(wd->resize_obj, SOFTKEY_PART))
     {
        if (!sd->softkey)
          {
             sd->softkey = evas_object_rectangle_add(e);
             evas_object_size_hint_min_set(sd->softkey, -1, 0);
             evas_object_size_hint_max_set(sd->softkey, -1, 0);
          }
        else
          _conformant_part_sizing_eval(obj, ELM_CONFORMANT_SOFTKEY_PART);

        evas_object_color_set(sd->softkey, 0, 0, 0, 0);
        elm_layout_content_set(obj, SOFTKEY_PART, sd->softkey);
     }
   else
     ELM_SAFE_FREE(sd->softkey, evas_object_del);
}

static Eina_Bool
_port_indicator_connect_cb(void *data)
{
   const char   *indicator_serv_name;
   ELM_CONFORMANT_DATA_GET(data, sd);
   int rot;

   if (!sd) return ECORE_CALLBACK_CANCEL;
   if (sd->indmode != ELM_WIN_INDICATOR_SHOW)
     {
        sd->port_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   rot = (intptr_t) evas_object_data_get(sd->portrait_indicator, CONFORMANT_KEY);
   indicator_serv_name = elm_config_indicator_service_get(rot);
   if (!indicator_serv_name)
     {
        DBG("Conformant cannot find indicator service name: Rotation=%d\n",rot);
        sd->port_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   if (strchr(indicator_serv_name, '/'))
     {
        sd->port_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   if (elm_plug_connect(sd->portrait_indicator, indicator_serv_name, 0, EINA_FALSE))
     {
        DBG("Conformant connect to server[%s]\n", indicator_serv_name);
        sd->port_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_land_indicator_connect_cb(void *data)
{
   const char   *indicator_serv_name;
   ELM_CONFORMANT_DATA_GET(data, sd);
   int rot;

   if (!sd) return ECORE_CALLBACK_CANCEL;
   if (sd->indmode != ELM_WIN_INDICATOR_SHOW)
     {
        sd->land_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   rot = (intptr_t) evas_object_data_get(sd->landscape_indicator, CONFORMANT_KEY);
   indicator_serv_name = elm_config_indicator_service_get(rot);
   if (!indicator_serv_name)
     {
        DBG("Conformant cannot find indicator service name: Rotation=%d\n",rot);
        sd->land_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   if (strchr(indicator_serv_name, '/'))
     {
        sd->port_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   if (elm_plug_connect(sd->landscape_indicator, indicator_serv_name, 0, EINA_FALSE))
     {
        DBG("Conformant connect to server[%s]\n", indicator_serv_name);
        sd->land_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_land_indicator_disconnected(void *data,
                             Eo *obj EINA_UNUSED,
                             const Eo_Event_Description *desc EINA_UNUSED,
                             void *event_info EINA_UNUSED)
{
   Evas_Object *conform = data;

   ELM_CONFORMANT_DATA_GET(conform, sd);

   sd->land_indi_timer = ecore_timer_add(ELM_CONFORM_INDICATOR_TIME,
                                         _land_indicator_connect_cb, conform);
   return EINA_TRUE;
}

static Eina_Bool
_port_indicator_disconnected(void *data,
                             Eo *obj EINA_UNUSED,
                             const Eo_Event_Description *desc EINA_UNUSED,
                             void *event_info EINA_UNUSED)
{
   Evas_Object *conform = data;

   ELM_CONFORMANT_DATA_GET(conform, sd);

   sd->port_indi_timer = ecore_timer_add(ELM_CONFORM_INDICATOR_TIME,
                                         _port_indicator_connect_cb, conform);
   return EINA_TRUE;
}

static Evas_Object *
_create_portrait_indicator(Evas_Object *obj)
{
   Evas_Object *port_indicator = NULL;
   const char *port_indicator_serv_name;

   ELM_CONFORMANT_DATA_GET(obj, sd);

   port_indicator_serv_name = elm_config_indicator_service_get(sd->rot);
   if (!port_indicator_serv_name)
     {
        DBG("Conformant cannot get portrait indicator service name\n");
        return NULL;
     }
   if (strchr(port_indicator_serv_name, '/'))
     {
        return NULL;
     }

   port_indicator = elm_plug_add(obj);
   if (!port_indicator)
     {
        DBG("Conformant cannot create plug to server[%s]\n", port_indicator_serv_name);
        return NULL;
     }

   if (!elm_plug_connect(port_indicator, port_indicator_serv_name, 0, EINA_FALSE))
     {
        DBG("Conformant cannot connect to server[%s]\n", port_indicator_serv_name);
        sd->port_indi_timer = ecore_timer_add(ELM_CONFORM_INDICATOR_TIME,
                                          _port_indicator_connect_cb, obj);
     }

   elm_widget_sub_object_add(obj, port_indicator);
   eo_do(port_indicator, eo_event_callback_add
     (ELM_PLUG_EVENT_IMAGE_DELETED, _port_indicator_disconnected, NULL));
   evas_object_size_hint_min_set(port_indicator, -1, 0);
   evas_object_size_hint_max_set(port_indicator, -1, 0);

   return port_indicator;
}

static Evas_Object *
_create_landscape_indicator(Evas_Object *obj)
{
   Evas_Object *land_indicator = NULL;
   const char *land_indicator_serv_name;

   ELM_CONFORMANT_DATA_GET(obj, sd);

   land_indicator_serv_name = elm_config_indicator_service_get(sd->rot);
   if (!land_indicator_serv_name)
     {
        DBG("Conformant cannot get portrait indicator service name\n");
        return NULL;
     }
   if (strchr(land_indicator_serv_name, '/'))
     {
        return NULL;
     }

   land_indicator = elm_plug_add(obj);
   if (!land_indicator)
     {
        DBG("Conformant cannot create plug to server[%s]\n", land_indicator_serv_name);
        return NULL;
     }

   if (!elm_plug_connect(land_indicator, land_indicator_serv_name, 0, EINA_FALSE))
     {
        DBG("Conformant cannot connect to server[%s]\n", land_indicator_serv_name);
        sd->land_indi_timer = ecore_timer_add(ELM_CONFORM_INDICATOR_TIME,
                                          _land_indicator_connect_cb, obj);
     }

   elm_widget_sub_object_add(obj, land_indicator);
   eo_do(land_indicator, eo_event_callback_add
     (ELM_PLUG_EVENT_IMAGE_DELETED, _land_indicator_disconnected, NULL));
   evas_object_size_hint_min_set(land_indicator, -1, 0);
   evas_object_size_hint_max_set(land_indicator, -1, 0);
   return land_indicator;
}

static void
_indicator_mode_set(Evas_Object *conformant, Elm_Win_Indicator_Mode indmode)
{
   Evas_Object *old_indi = NULL;
   ELM_CONFORMANT_DATA_GET(conformant, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(conformant, wd);

   sd->indmode = indmode;

   if (!edje_object_part_exists(wd->resize_obj, INDICATOR_PART))
     return;

   if (indmode == ELM_WIN_INDICATOR_SHOW)
     {
        old_indi = elm_layout_content_get(conformant, INDICATOR_PART);

        //create new indicator
        if (!old_indi)
          {
             if ((sd->rot == 90)||(sd->rot == 270))
               {
                  if (!sd->landscape_indicator)
                    sd->landscape_indicator = _create_landscape_indicator(conformant);

                  if (!sd->landscape_indicator) return;
                  elm_layout_content_set(conformant, INDICATOR_PART, sd->landscape_indicator);
               }
             else
               {
                  if (!sd->portrait_indicator)
                    sd->portrait_indicator = _create_portrait_indicator(conformant);

                  if (!sd->portrait_indicator) return;
                  elm_layout_content_set(conformant, INDICATOR_PART, sd->portrait_indicator);
               }

          }
        elm_object_signal_emit(conformant, "elm,state,indicator,show", "elm");
     }
   else
     elm_object_signal_emit(conformant, "elm,state,indicator,hide", "elm");
}

static void
_indicator_opacity_set(Evas_Object *conformant, Elm_Win_Indicator_Opacity_Mode ind_o_mode)
{
   ELM_CONFORMANT_DATA_GET(conformant, sd);
   sd->ind_o_mode = ind_o_mode;
   //TODO: opacity change
}

static Eina_Bool
_on_indicator_mode_changed(void *data,
                           Eo *obj, const Eo_Event_Description *desc EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Evas_Object *conformant = data;
   Evas_Object *win = obj;

   Elm_Win_Indicator_Mode indmode;
   Elm_Win_Indicator_Opacity_Mode ind_o_mode;

   ELM_CONFORMANT_DATA_GET(conformant, sd);

   indmode = elm_win_indicator_mode_get(win);
   ind_o_mode = elm_win_indicator_opacity_get(win);
   if (indmode != sd->indmode)
     _indicator_mode_set(conformant, indmode);
   if (ind_o_mode != sd->ind_o_mode)
     _indicator_opacity_set(conformant, ind_o_mode);
   return EINA_TRUE;
}

static Eina_Bool
_on_rotation_changed(void *data,
                     Eo *obj, const Eo_Event_Description *desc EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   int rot = 0;
   Evas_Object *win = obj;
   Evas_Object *conformant = data;
   Evas_Object *old_indi = NULL;

   ELM_CONFORMANT_DATA_GET(data, sd);

   rot = elm_win_rotation_get(win);

   if (rot == sd->rot) return EINA_TRUE;

   sd->rot = rot;
   old_indi = elm_layout_content_unset(conformant, INDICATOR_PART);
   /* this means ELM_WIN_INDICATOR_SHOW never be set.we don't need to change indicator type*/
   if (!old_indi) return EINA_TRUE;
   evas_object_hide(old_indi);

   if ((rot == 90) || (rot == 270))
     {
        if (!sd->landscape_indicator)
          sd->landscape_indicator = _create_landscape_indicator(conformant);

        if (!sd->landscape_indicator) return EINA_TRUE;

        evas_object_show(sd->landscape_indicator);
        evas_object_data_set(sd->landscape_indicator, CONFORMANT_KEY, (void *) (intptr_t) rot);
        elm_layout_content_set(conformant, INDICATOR_PART, sd->landscape_indicator);
     }
   else
     {
        if (!sd->portrait_indicator)
          sd->portrait_indicator = _create_portrait_indicator(conformant);

        if (!sd->portrait_indicator) return EINA_TRUE;

        evas_object_show(sd->portrait_indicator);
        evas_object_data_set(sd->portrait_indicator, CONFORMANT_KEY, (void *) (intptr_t) rot);
        elm_layout_content_set(conformant, INDICATOR_PART, sd->portrait_indicator);
     }
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_conformant_elm_widget_theme_apply(Eo *obj, Elm_Conformant_Data *_pd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   _conformant_parts_swallow(obj);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

// unused now - but meant to be for making sure the focused widget is always
// visible when the vkbd comes and goes by moving the conformant obj (and thus
// its children) to  show the focused widget (and if focus changes follow)
/*
   static Evas_Object *
   _focus_object_get(const Evas_Object *obj)
   {
   Evas_Object *win, *foc;

   win = elm_widget_top_get(obj);
   if (!win) return NULL;
   foc = elm_widget_top_get(win);
   }

   static void
   _focus_object_region_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
   {
   evas_object_geometry_get(obj, x, y, w, h);
   }

   static void
   _focus_change_del(void *data, Evas_Object *obj, void *event_info)
   {
   // called from toplevel when the focused window shanges
   }

   static void
   _autoscroll_move(Evas_Object *obj)
   {
   // move conformant edje by delta to show focused widget
   }

   static void
   _autoscroll_mode_enable(Evas_Object *obj)
   {
   // called when autoscroll mode should be on - content area smaller than
   // its min size
   // 1. get focused object
   // 2. if not in visible conformant area calculate delta needed to
   //    get it in
   // 3. store delta and call _autoscroll_move() which either asanimates
   //    or jumps right there
   }

   static void
   _autoscroll_mode_disable(Evas_Object *obj)
   {
   // called when autoscroll mode should be off - set delta to 0 and
   // call _autoscroll_move()
   }
 */

static void
_move_resize_cb(void *data EINA_UNUSED,
                Evas *e EINA_UNUSED,
                Evas_Object *obj,
                void *event_info EINA_UNUSED)
{
   Conformant_Part_Type part_type;
   ELM_CONFORMANT_DATA_GET(obj, sd);
   Elm_Win_Keyboard_Mode mode;

   part_type = (ELM_CONFORMANT_INDICATOR_PART |
                ELM_CONFORMANT_SOFTKEY_PART |
                ELM_CONFORMANT_CLIPBOARD_PART);

   mode = elm_win_keyboard_mode_get(sd->win);
   if (mode == ELM_WIN_KEYBOARD_ON)
     part_type |= ELM_CONFORMANT_VIRTUAL_KEYPAD_PART;

   _conformant_part_sizing_eval(obj, part_type);
}

static void
_show_region_job(void *data)
{
   Evas_Object *focus_obj;

   ELM_CONFORMANT_DATA_GET(data, sd);

   focus_obj = elm_widget_focused_object_get(data);
   if (focus_obj)
     {
        Evas_Coord x, y, w, h;

        elm_widget_focus_region_get(focus_obj, &x, &y, &w, &h);

        if (h < _elm_config->finger_size)
          h = _elm_config->finger_size;

        elm_widget_show_region_set(focus_obj, x, y, w, h, EINA_TRUE);
     }

   sd->show_region_job = NULL;
}

// showing the focused/important region.
static void
_on_content_resize(void *data,
                   Evas *e EINA_UNUSED,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   ELM_CONFORMANT_DATA_GET(data, sd);

#ifdef HAVE_ELEMENTARY_X
   if ((sd->vkb_state == ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF) &&
       (sd->clipboard_state == ECORE_X_ILLUME_CLIPBOARD_STATE_OFF))
     return;
#endif

   ecore_job_del(sd->show_region_job);
   sd->show_region_job = ecore_job_add(_show_region_job, data);
}

static void
_autoscroll_objects_update(void *data)
{
   Evas_Object *sub, *top_scroller = NULL;

   ELM_CONFORMANT_DATA_GET(data, sd);

   sub = elm_widget_focused_object_get(data);
   //Look up for top most scroller in the focus object hierarchy
   //inside Conformant.

   while (sub)
     {
        if (eo_isa(sub, ELM_CONFORMANT_CLASS)) break;

        if (eo_isa(sub, ELM_SCROLLER_CLASS) || eo_isa(sub, ELM_GENLIST_CLASS))
          top_scroller = sub;

        sub = elm_object_parent_widget_get(sub);
     }

   //If the scroller got changed by app, replace it.
   if (top_scroller != sd->scroller)
     {
        if (sd->scroller)
          evas_object_event_callback_del
            (sd->scroller, EVAS_CALLBACK_RESIZE, _on_content_resize);
        sd->scroller = top_scroller;

        if (sd->scroller)
          evas_object_event_callback_add
            (sd->scroller, EVAS_CALLBACK_RESIZE, _on_content_resize, data);
     }
}

#ifdef HAVE_ELEMENTARY_X
static void
_virtualkeypad_state_change(Evas_Object *obj, Ecore_X_Event_Window_Property *ev)
{
   ELM_CONFORMANT_DATA_GET(obj, sd);

   Ecore_X_Window zone = ecore_x_e_illume_zone_get(ev->win);
   Ecore_X_Virtual_Keyboard_State state =
      ecore_x_e_virtual_keyboard_state_get(ev->win);

   DBG("[KEYPAD]:window's state win=0x%x, state=%d.", ev->win, state);
   if (state == ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN)
     {
        if (zone) state = ecore_x_e_virtual_keyboard_state_get(zone);
        DBG("[KEYPAD]:zone's state zone=0x%x, state=%d.", zone, state);
     }

   if (sd->vkb_state == state) return;
   sd->vkb_state = state;

   if (state == ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF)
     {
        DBG("[KEYPAD]:ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF");
        evas_object_size_hint_min_set(sd->virtualkeypad, -1, 0);
        evas_object_size_hint_max_set(sd->virtualkeypad, -1, 0);
        _conformant_part_sizing_eval(obj, ELM_CONFORMANT_VIRTUAL_KEYPAD_PART);
        if (!sd->clipboard_state)
          elm_widget_display_mode_set(obj, EVAS_DISPLAY_MODE_NONE);
        eo_do(obj, eo_event_callback_call(
                 ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_OFF, NULL));
     }
   else if (state == ECORE_X_VIRTUAL_KEYBOARD_STATE_ON)
     {
        DBG("[KEYPAD]:ECORE_X_VIRTUAL_KEYBOARD_STATE_ON");
        _conformant_part_sizing_eval(obj, ELM_CONFORMANT_VIRTUAL_KEYPAD_PART);
        elm_widget_display_mode_set(obj, EVAS_DISPLAY_MODE_COMPRESS);
        _autoscroll_objects_update(obj);
        eo_do(obj, eo_event_callback_call(
                 ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_ON, NULL));
     }
}

static void
_clipboard_state_change(Evas_Object *obj, Ecore_X_Event_Window_Property *ev)
{
   ELM_CONFORMANT_DATA_GET(obj, sd);

   Ecore_X_Window zone = ecore_x_e_illume_zone_get(ev->win);
   Ecore_X_Illume_Clipboard_State state =
      ecore_x_e_illume_clipboard_state_get(ev->win);

   DBG("[CLIPBOARD]:window's state win=0x%x, state=%d.", ev->win, state);

   if (state == ECORE_X_ILLUME_CLIPBOARD_STATE_UNKNOWN)
     {
        state = ecore_x_e_illume_clipboard_state_get(ev->win);
        DBG("[CLIPBOARD]:zone's state zone=0x%x, state=%d.", zone, state);
     }

   if (sd->clipboard_state == state) return;
   sd->clipboard_state = state;

   if (state == ECORE_X_ILLUME_CLIPBOARD_STATE_OFF)
     {
        evas_object_size_hint_min_set(sd->clipboard, -1, 0);
        evas_object_size_hint_max_set(sd->clipboard, -1, 0);
        if (!sd->vkb_state)
          elm_widget_display_mode_set(obj, EVAS_DISPLAY_MODE_NONE);
        eo_do(obj, eo_event_callback_call(
                 ELM_CONFORMANT_EVENT_CLIPBOARD_STATE_OFF, NULL));
     }
   else if (state == ECORE_X_ILLUME_CLIPBOARD_STATE_ON)
     {
        elm_widget_display_mode_set(obj, EVAS_DISPLAY_MODE_COMPRESS);
        _autoscroll_objects_update(obj);
        eo_do(obj, eo_event_callback_call(
                 ELM_CONFORMANT_EVENT_CLIPBOARD_STATE_ON, NULL));
     }
}

static Eina_Bool
_on_prop_change(void *data,
                int type EINA_UNUSED,
                void *event)
{
   Ecore_X_Event_Window_Property *ev = event;

   int pid = 0;

#ifdef __linux__
   pid = (int)getpid();
#endif

   if (ev->atom == ECORE_X_ATOM_E_ILLUME_ZONE)
     {
        DBG("pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_ZONE.\n", pid, ev->win);
        Conformant_Part_Type part_type;

        part_type = (ELM_CONFORMANT_INDICATOR_PART |
                     ELM_CONFORMANT_SOFTKEY_PART |
                     ELM_CONFORMANT_VIRTUAL_KEYPAD_PART |
                     ELM_CONFORMANT_CLIPBOARD_PART);

        _conformant_part_sizing_eval(data, part_type);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY)
     {
        DBG("pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY.", pid, ev->win);
        _conformant_part_sizing_eval(data, ELM_CONFORMANT_INDICATOR_PART);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY)
     {
        DBG("pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY.", pid, ev->win);
        _conformant_part_sizing_eval(data, ELM_CONFORMANT_SOFTKEY_PART);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY)
     {
        DBG("[KEYPAD]:pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY.", pid, ev->win);
        _conformant_part_sizing_eval(data, ELM_CONFORMANT_VIRTUAL_KEYPAD_PART);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_CLIPBOARD_GEOMETRY)
     {
        DBG("pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_CLIPBOARD_GEOMETRY.", pid, ev->win);
        _conformant_part_sizing_eval(data, ELM_CONFORMANT_CLIPBOARD_PART);
     }
   else if (ev->atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE)
     {
        DBG("[KEYPAD]:pid=%d, win=0x%x, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE.", pid, ev->win);
        _virtualkeypad_state_change(data, ev);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_CLIPBOARD_STATE)
     {
        DBG("pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_CLIPBOARD_STATE.", pid, ev->win);
        _clipboard_state_change(data, ev);
     }

   return ECORE_CALLBACK_PASS_ON;
}

#endif

// TIZEN_ONLY(20150707): implemented elm_win_conformant_set/get for wayland
static void
_on_conformant_changed(void *data,
                     Evas_Object *obj,
                     void *event_info)
{
   Conformant_Part_Type part_type;
   Conformant_Property property = (Conformant_Property) event_info;
   Elm_Win_Keyboard_Mode mode;

   part_type = (ELM_CONFORMANT_INDICATOR_PART |
                ELM_CONFORMANT_VIRTUAL_KEYPAD_PART);

   ELM_CONFORMANT_DATA_GET(data, sd);

   /* object is already freed */
   if (!sd) return;

   mode = elm_win_keyboard_mode_get(obj);

   if (property & CONFORMANT_KEYBOARD_STATE)
     {
        if (mode == ELM_WIN_KEYBOARD_ON)
          {
             _conformant_part_sizing_eval(data, ELM_CONFORMANT_VIRTUAL_KEYPAD_PART);
             elm_widget_display_mode_set(data, EVAS_DISPLAY_MODE_COMPRESS);
             _autoscroll_objects_update(data);
             eo_do(data, eo_event_callback_call(
                   ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_ON, NULL));
          }
        else if (mode == ELM_WIN_KEYBOARD_OFF)
          {
             evas_object_size_hint_min_set(sd->virtualkeypad, -1, 0);
             evas_object_size_hint_max_set(sd->virtualkeypad, -1, 0);
             elm_widget_display_mode_set(data, EVAS_DISPLAY_MODE_NONE);
             eo_do(data, eo_event_callback_call(
                   ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_OFF, NULL));
          }
     }
   if ((property & CONFORMANT_KEYBOARD_GEOMETRY) &&
       (mode == ELM_WIN_KEYBOARD_ON))
     {
        _conformant_part_sizing_eval(data, ELM_CONFORMANT_VIRTUAL_KEYPAD_PART);
     }
}
//

EOLIAN static void
_elm_conformant_evas_object_smart_add(Eo *obj, Elm_Conformant_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   if (!elm_layout_theme_set
       (obj, "conformant", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   _conformant_parts_swallow(obj);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _move_resize_cb, obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOVE, _move_resize_cb, obj);

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_conformant_evas_object_smart_del(Eo *obj, Elm_Conformant_Data *sd)
{
#ifdef HAVE_ELEMENTARY_X
   ecore_event_handler_del(sd->prop_hdl);
#endif

   ecore_job_del(sd->show_region_job);
   ecore_timer_del(sd->port_indi_timer);
   ecore_timer_del(sd->land_indi_timer);
   evas_object_del(sd->portrait_indicator);
   evas_object_del(sd->landscape_indicator);

   evas_object_data_set(sd->win, "\377 elm,conformant", NULL);

   eo_do(sd->win,
         eo_event_callback_del(ELM_WIN_EVENT_INDICATOR_PROP_CHANGED,
            _on_indicator_mode_changed, obj),
         eo_event_callback_del(ELM_WIN_EVENT_ROTATION_CHANGED,
            _on_rotation_changed, obj));
   // TIZEN_ONLY(20150707): implemented elm_win_conformant_set/get for wayland
   evas_object_smart_callback_del_full
     (sd->win, "conformant,changed", _on_conformant_changed, obj);
   //

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_conformant_elm_widget_parent_set(Eo *obj, Elm_Conformant_Data *sd, Evas_Object *parent)
{
#ifdef HAVE_ELEMENTARY_X
   Evas_Object *top = elm_widget_top_get(parent);
   Ecore_X_Window xwin = elm_win_xwindow_get(parent);

   if ((xwin) && (!elm_win_inlined_image_object_get(top)))
     {

        sd->prop_hdl = ecore_event_handler_add
            (ECORE_X_EVENT_WINDOW_PROPERTY, _on_prop_change, obj);
        sd->vkb_state = ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF;
        sd->clipboard_state = ECORE_X_ILLUME_CLIPBOARD_STATE_OFF;
     }
   // FIXME: get kbd region prop
#endif
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_conformant_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Conformant_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

// TIZEN_ONLY(20160218): Improve launching performance.
EAPI void
elm_conformant_precreated_object_set(Evas_Object *obj)
{
   INF("Set precreated obj(%p).", obj);
   _precreated_conform_obj = obj;
}

EAPI Evas_Object *
elm_conformant_precreated_object_get(void)
{
   INF("Get precreated obj(%p).", _precreated_conform_obj);
   return _precreated_conform_obj;
}
//

EAPI Evas_Object *
elm_conformant_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
// TIZEN_ONLY(20160218): Improve launching performance.
   if (_precreated_conform_obj)
     {
        Evas_Object *par_obj = elm_widget_parent_get(_precreated_conform_obj);

        if (par_obj == parent)
          {
             Evas_Object *above_obj = evas_object_above_get(_precreated_conform_obj);
             if (above_obj)
               evas_object_raise(_precreated_conform_obj);

             Evas_Object *tmp = _precreated_conform_obj;
             _precreated_conform_obj = NULL;
             INF("Return precreated obj(%p).", tmp);
             return tmp;
          }
     }
//
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static Eo *
_elm_conformant_eo_base_constructor(Eo *obj, Elm_Conformant_Data *sd)
{
   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_FILLER));

   sd->win = elm_widget_top_get(obj);
   _on_indicator_mode_changed(obj, sd->win, NULL, NULL);
   _on_rotation_changed(obj, sd->win, NULL, NULL);

   sd->indmode = elm_win_indicator_mode_get(sd->win);
   sd->ind_o_mode = elm_win_indicator_opacity_get(sd->win);
   sd->rot = elm_win_rotation_get(sd->win);
   evas_object_data_set(sd->win, "\377 elm,conformant", obj);

   eo_do(sd->win,
         eo_event_callback_add(ELM_WIN_EVENT_INDICATOR_PROP_CHANGED,
            _on_indicator_mode_changed, obj),
         eo_event_callback_add(ELM_WIN_EVENT_ROTATION_CHANGED,
            _on_rotation_changed, obj));
   // TIZEN_ONLY(20150707): implemented elm_win_conformant_set/get for wayland
   evas_object_smart_callback_add
     (sd->win, "conformant,changed", _on_conformant_changed, obj);
   //

   return obj;
}

static void
_elm_conformant_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_conformant.eo.c"
