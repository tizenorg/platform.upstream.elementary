#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _Testitem
{
   Elm_Object_Item *item;
   int mode, onoff;
} Testitem;

static int rotate_with_resize = 0;
static Eina_Bool fullscreen = EINA_FALSE;

static void
my_bt_38_alpha_on(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   evas_object_hide(bg);
   elm_win_alpha_set(win, EINA_TRUE);
}

static void
my_bt_38_alpha_off(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   evas_object_show(bg);
   elm_win_alpha_set(win, EINA_FALSE);
}

static Eina_Bool
_activate_timer_cb(void *data)
{
   printf("Activate window\n");
   elm_win_activate(data);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_deiconify_timer_cb(void *data)
{
   printf("Deiconify window\n");
   elm_win_iconified_set(data, EINA_FALSE);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_unwith(void *data)
{
   printf("show\n");
   evas_object_show(data);
   elm_win_activate(data);
   return EINA_FALSE;
}

static void
my_bt_38_withdraw(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   printf("withdraw, current %i\n", elm_win_withdrawn_get(win));
   elm_win_withdrawn_set(win, EINA_TRUE);
   ecore_timer_add(10.0, _unwith, win);
}

static void
my_bt_38_massive(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   evas_object_resize(win, 4000, 2400);
}

static void
my_ck_38_resize(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   rotate_with_resize = elm_check_state_get(obj);
}

static void
my_bt_38_rot_0(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 0);
   else
     elm_win_rotation_set(win, 0);
}

static void
my_bt_38_rot_90(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 90);
   else
     elm_win_rotation_set(win, 90);
}

static void
my_bt_38_rot_180(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 180);
   else
     elm_win_rotation_set(win, 180);
}

static void
my_bt_38_rot_270(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 270);
   else
     elm_win_rotation_set(win, 270);
}

static void
my_ck_38_fullscreen(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   fullscreen = elm_check_state_get(obj);
   elm_win_fullscreen_set(win, fullscreen);
}

static void
my_ck_38_borderless(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Eina_Bool borderless = elm_check_state_get(obj);
   elm_win_borderless_set(win, borderless);
}

static void
my_win_move(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Coord x, y;
   elm_win_screen_position_get(obj, &x, &y);
   printf("MOVE - win geom: %4i %4i\n", x, y);
}

static void
_win_resize(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   printf("RESIZE - win geom: %4ix%4i\n", w, h);
}

static void
_win_foc_in(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("FOC IN\n");
}

static void
_win_foc_out(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("FOC OUT\n");
}

static void
_close_win(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_move_20_20(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_move(data, 20, 20);
}

static void
_move_0_0(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_move(data, 0, 0);
}

static void
_bt_win_lower(void *data, Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   printf("Lower window\n");
   elm_win_lower(data);
}

static void
_bt_win_iconify_and_activate(void *data, Evas_Object *obj EINA_UNUSED,
                             void *event_info EINA_UNUSED)
{
   printf("Iconify window. (current status: %i)\n", elm_win_iconified_get(data));
   elm_win_iconified_set(data, EINA_TRUE);

   printf("This window will be activated in 5 seconds.\n");
   ecore_timer_add(5.0, _activate_timer_cb, data);
}

static void
_bt_win_iconify_and_deiconify(void *data, Evas_Object *obj EINA_UNUSED,
                              void *event_info EINA_UNUSED)
{
   printf("Iconify window. (current status: %i)\n", elm_win_iconified_get(data));
   elm_win_iconified_set(data, EINA_TRUE);

   printf("This window will be deiconified in 5 seconds.\n");
   ecore_timer_add(5.0, _deiconify_timer_cb, data);
}

static void
_bt_win_center_cb(void *data, Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   printf("Center window.\n");
   elm_win_center(data, EINA_TRUE, EINA_TRUE);
}

static void
_win_state_print_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("WIN: %s\n", (char *)data);
}

static void
_win_state_focus_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("WIN FOCUS: %s\n", (char *)data);
}

static void
_win_state_visibility_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   /* event_info for "visibility,changed" callback
    * 0: the window is fully obscured
    * 1: the window is unobscured
    */
   int visibility = (int)(void *)event_info;
   printf("WIN: %s %d\n", (char *)data, visibility);
}

static void
_win_effect_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   const char *type_name;
   Elm_Win_Effect_Type type = (Elm_Win_Effect_Type)(event_info);

   switch (type)
     {
      case ELM_WIN_EFFECT_TYPE_SHOW:
         type_name = eina_stringshare_add("SHOW");
         break;
      case ELM_WIN_EFFECT_TYPE_HIDE:
         type_name = eina_stringshare_add("HIDE");
         break;
      case ELM_WIN_EFFECT_TYPE_RESTACK:
         type_name = eina_stringshare_add("RESTACK");
         break;
      default:
         type_name = eina_stringshare_add("UNKNOWN");
     }

   printf("EFFECT: %s %s\n", type_name, (char *)data);
   eina_stringshare_del(type_name);
}

static void
_win_show(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("win: show\n");
}

static void
_win_hide(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("win: hide\n");
}

void
test_win_state(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *sl, *bx, *bx2, *bt, *ck;

   win = elm_win_add(NULL, "window-states", ELM_WIN_BASIC);
   elm_win_title_set(win, "Window States");
   evas_object_smart_callback_add(win, "moved", my_win_move, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _win_resize, NULL);
   evas_event_callback_add(evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_FOCUS_IN, _win_foc_in, NULL);
   evas_event_callback_add(evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_FOCUS_OUT, _win_foc_out, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_SHOW, _win_show, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_HIDE, _win_hide, NULL);
   evas_object_smart_callback_add(win, "withdrawn", _win_state_print_cb, "withdrawn");
   evas_object_smart_callback_add(win, "iconified", _win_state_print_cb, "iconified");
   evas_object_smart_callback_add(win, "normal", _win_state_print_cb, "normal");
   evas_object_smart_callback_add(win, "stick", _win_state_print_cb, "stick");
   evas_object_smart_callback_add(win, "unstick", _win_state_print_cb, "unstick");
   evas_object_smart_callback_add(win, "fullscreen", _win_state_print_cb, "fullscreen");
   evas_object_smart_callback_add(win, "unfullscreen", _win_state_print_cb, "unfullscreen");
   evas_object_smart_callback_add(win, "maximized", _win_state_print_cb, "maximized");
   evas_object_smart_callback_add(win, "unmaximized", _win_state_print_cb, "unmaximized");
   evas_object_smart_callback_add(win, "ioerr", _win_state_print_cb, "ioerr");
   evas_object_smart_callback_add(win, "indicator,prop,changed", _win_state_print_cb, "indicator,prop,changed");
   evas_object_smart_callback_add(win, "rotation,changed", _win_state_print_cb, "rotation,changed");
   evas_object_smart_callback_add(win, "profile,changed", _win_state_print_cb, "profile,changed");
   evas_object_smart_callback_add(win, "focused", _win_state_focus_cb, "focused");
   evas_object_smart_callback_add(win, "unfocused", _win_state_focus_cb, "unfocused");
   evas_object_smart_callback_add(win, "focus,out", _win_state_focus_cb, "focus,out");
   evas_object_smart_callback_add(win, "focus,in", _win_state_focus_cb, "focus,in");
   evas_object_smart_callback_add(win, "delete,request", _win_state_print_cb, "delete,request");
   evas_object_smart_callback_add(win, "wm,rotation,changed", _win_state_print_cb, "wm,rotation,changed");
   evas_object_smart_callback_add(win, "visibility,changed", _win_state_visibility_cb, "visibility,changed");
   evas_object_smart_callback_add(win, "effect,started", _win_effect_cb, "effect,started");
   evas_object_smart_callback_add(win, "effect,done", _win_effect_cb, "effect,done");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);
   evas_object_data_set(win, "bg", bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Alpha On");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_alpha_on, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Alpha Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_alpha_off, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Withdraw");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_withdraw, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Massive");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_massive, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Move 20 20");
   evas_object_smart_callback_add(bt, "clicked", _move_20_20, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Lower");
   evas_object_smart_callback_add(bt, "clicked", _bt_win_lower, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Iconify and Activate");
   evas_object_smart_callback_add(bt, "clicked",
                                  _bt_win_iconify_and_activate, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx2);
   elm_object_text_set(bt, "Iconify and Deiconify");
   evas_object_smart_callback_add(bt, "clicked",
                                  _bt_win_iconify_and_deiconify, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Center");
   evas_object_smart_callback_add(bt, "clicked",
                                  _bt_win_center_cb, win);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Test");
   elm_slider_span_size_set(sl, 100);
   evas_object_size_hint_align_set(sl, 0.5, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 50);
   elm_slider_inverted_set(sl, EINA_TRUE);
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "resize");
   elm_check_state_set(ck, rotate_with_resize);
   evas_object_smart_callback_add(ck, "changed", my_ck_38_resize, win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ck, 0.02, 0.99);
   evas_object_show(ck);
   elm_box_pack_end(bx, ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "fullscreen");
   elm_check_state_set(ck, fullscreen);
   evas_object_smart_callback_add(ck, "changed", my_ck_38_fullscreen, win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ck, 0.02, 0.99);
   evas_object_show(ck);
   elm_box_pack_end(bx, ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "borderless");
   elm_check_state_set(ck, fullscreen);
   evas_object_smart_callback_add(ck, "changed", my_ck_38_borderless, win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ck, 0.02, 0.99);
   evas_object_show(ck);
   elm_box_pack_end(bx, ck);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 0");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_0, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 90");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_90, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 180");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_180, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 270");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_270, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Move 0 0");
   evas_object_smart_callback_add(bt, "clicked", _move_0_0, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 280, 400);
   evas_object_show(win);
}

void
test_win_state2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *sl, *bx, *bx2, *bt, *ck;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "window-states2", ELM_WIN_BASIC);
   elm_win_override_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "moved", my_win_move, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _win_resize, NULL);
   elm_win_title_set(win, "Window States 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);
   evas_object_data_set(win, "bg", bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Alpha On");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_alpha_on, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Alpha Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_alpha_off, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Close");
   evas_object_smart_callback_add(bt, "clicked", _close_win, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Move 20 20");
   evas_object_smart_callback_add(bt, "clicked", _move_20_20, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Override Redirect");
   elm_slider_span_size_set(sl, 100);
   evas_object_size_hint_align_set(sl, 0.5, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 50);
   elm_slider_inverted_set(sl, EINA_TRUE);
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "resize");
   elm_check_state_set(ck, rotate_with_resize);
   evas_object_smart_callback_add(ck, "changed", my_ck_38_resize, win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ck, 0.02, 0.99);
   evas_object_show(ck);
   elm_box_pack_end(bx, ck);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 0");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_0, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 90");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_90, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 180");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_180, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 270");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_270, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Move 0 0");
   evas_object_smart_callback_add(bt, "clicked", _move_0_0, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
