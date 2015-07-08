#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define EFL_BETA_API_SUPPORT 1

#ifdef TIZEN_VECTOR_UX

static const char *vg_key = "_tizen_vg";


/////////////////////////////////////////////////////////////////////////
/* Radio */
/////////////////////////////////////////////////////////////////////////
typedef struct vg_radio_s
{
   Evas_Object *vg[3];       //0: outline, 1: center circle, 2: iconic circle
   Efl_VG_Shape *shape[4];   //0: outline, 1: center circle, 2: iconic outline, 3: iconic circle
   Elm_Transit *transit;
   Evas_Object *obj;
   Eina_Bool init : 1;
} vg_radio;

static void
transit_radio_op(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED,
                 double progress)
{
   vg_radio *vd = effect;

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[0], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);

   if (elm_radio_selected_object_get(vd->obj) != vd->obj)
     progress = 1 - progress;

   double radius =
      ELM_SCALE_SIZE((center_x > center_y ? center_x : center_y) - 2);

   //Iconic Circle (Outline)
   evas_vg_shape_stroke_width_set(vd->shape[2],
                                  (1 + progress * ELM_SCALE_SIZE(1.5)));
   //Iconic Circle (Outline)
   evas_vg_shape_shape_reset(vd->shape[2]);
   evas_vg_shape_shape_append_circle(vd->shape[2], center_x, center_y,
                                     radius);
   //Iconic Circle (Center)
   evas_vg_shape_shape_reset(vd->shape[3]);
   evas_vg_shape_shape_append_circle(vd->shape[3], center_x, center_y,
                                     radius * 0.5 * progress);
}

static void
transit_radio_del_cb(void *data, Elm_Transit *transit EINA_UNUSED)
{
   vg_radio *vd = data;
   vd->transit = NULL;
}

static void
radio_changed_cb(void *data, Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   vg_radio *vd = data;

   //Circle Effect
   elm_transit_del(vd->transit);
   vd->transit = elm_transit_add();
   elm_transit_effect_add(vd->transit, transit_radio_op, vd,
                          NULL);
   elm_transit_del_cb_set(vd->transit, transit_radio_del_cb, vd);
   elm_transit_tween_mode_set(vd->transit,
                              ELM_TRANSIT_TWEEN_MODE_DECELERATE);
   elm_transit_duration_set(vd->transit, 0.2);
   elm_transit_go(vd->transit);
}

static void
radio_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   vg_radio *vd = data;
   evas_object_data_set(vd->obj, vg_key, NULL);
   evas_object_smart_callback_del(vd->obj, "changed",
                                  radio_changed_cb);
   elm_transit_del(vd->transit);
   free(vd);
}

static void
radio_init(vg_radio *vd)
{
   if (vd->init) return;
   vd->init = EINA_TRUE;

   //Outline Shape
   vd->shape[0] = evas_vg_shape_add(evas_object_vg_root_node_get(vd->vg[0]));
   evas_vg_shape_stroke_color_set(vd->shape[0], 255, 255, 255, 255);
   evas_vg_shape_stroke_width_set(vd->shape[0], ELM_SCALE_SIZE(1));

   //Center Circle
   vd->shape[1] = evas_vg_shape_add(evas_object_vg_root_node_get(vd->vg[1]));
   evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255);

   //Iconic Circle (Outline)
   vd->shape[2] = evas_vg_shape_add(evas_object_vg_root_node_get(vd->vg[2]));
   evas_vg_shape_stroke_color_set(vd->shape[2], 255, 255, 255, 255);
   evas_vg_shape_stroke_width_set(vd->shape[2], ELM_SCALE_SIZE(1.5));

   //Iconic Circle (Center Point)
   vd->shape[3] = evas_vg_shape_add(evas_object_vg_root_node_get(vd->vg[2]));
   evas_vg_node_color_set(vd->shape[3], 255, 255, 255, 255);
}

static void
radio_base_resize_cb(void *data, Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   vg_radio *vd = data;

   radio_init(vd);

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[0], NULL, NULL, &w, &h);
   Evas_Coord center_x = (w / 2);
   Evas_Coord center_y = (h / 2);

   double radius =
      ELM_SCALE_SIZE((center_x > center_y ? center_x : center_y) - 2);

   //Outline
   evas_vg_shape_shape_reset(vd->shape[0]);
   evas_vg_shape_shape_append_circle(vd->shape[0], center_x, center_y,
                                     radius);

   if (elm_radio_selected_object_get(vd->obj) != vd->obj) return;

   //Center Circle
   evas_vg_shape_shape_reset(vd->shape[1]);
   evas_vg_shape_shape_append_circle(vd->shape[1], center_x, center_y,
                                     radius);

   //Iconic Circle (Outline)
   evas_vg_shape_shape_reset(vd->shape[2]);
   evas_vg_shape_shape_append_circle(vd->shape[2], center_x, center_y,
                                     radius);

   //Iconic Circle (Center)
   evas_vg_shape_shape_reset(vd->shape[3]);
   evas_vg_shape_shape_append_circle(vd->shape[3], center_x, center_y,
                                     radius * 0.5);
}

void
tizen_vg_radio_set(Elm_Radio *obj)
{
   vg_radio *vd = evas_object_data_get(obj, vg_key);
   if (vd) evas_object_del(vd->vg[0]);

   //Apply vector ux only theme has "vector_ux" "on"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;
   if (strcmp(str, "on")) return;

   vd = calloc(1, sizeof(vg_radio));
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }
   evas_object_data_set(obj, vg_key, vd);
   evas_object_smart_callback_add(obj, "changed", radio_changed_cb, vd);

   //Vector Graphics Object
   Evas *e = evas_object_evas_get(obj);

   //Outline VG
   vd->vg[0] = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_DEL,
                                  radio_del_cb, vd);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  radio_base_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg[0]);

   //Center Circle
   vd->vg[1] = evas_object_vg_add(e);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->vg[1]);

   //Iconic Circle
   vd->vg[2] = evas_object_vg_add(e);
   elm_object_part_content_set(obj, "tizen_vg_shape3", vd->vg[2]);

   vd->obj = obj;
}


/////////////////////////////////////////////////////////////////////////
/* Check: Favorite */
/////////////////////////////////////////////////////////////////////////
typedef struct check_favorite_s
{
   Evas_Object *vg[2]; //0: outline, 1: inner body
   Efl_VG_Shape *shape[2];  //0: outline, 1: inner body
   Elm_Transit *transit;
   Evas_Object *obj;
   Eina_Bool init : 1;
} check_favorite;

static void
check_favorite_init(check_favorite *vd)
{
   if (vd->init) return;
   vd->init = EINA_TRUE;

   //Outline Star
   vd->shape[0] = evas_vg_shape_add(evas_object_vg_root_node_get(vd->vg[0]));
   evas_vg_shape_stroke_color_set(vd->shape[0], 255, 255, 255, 255);
   evas_vg_shape_stroke_width_set(vd->shape[0], ELM_SCALE_SIZE(1.5));
   evas_vg_shape_stroke_join_set(vd->shape[0], EFL_GFX_JOIN_ROUND);

   //Inner Body Star
   vd->shape[1] = evas_vg_shape_add(evas_object_vg_root_node_get(vd->vg[1]));
   evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255);
}

static void
check_favorite_shape_do(check_favorite *vd, int idx)
{
   //Input Parameters
   #define PT_CNT 10

   const double STAR_PT[PT_CNT][2] = {{0, -18}, {6, -6}, {17, -6}, {8, 3},
                                      {12, 16}, {0, 9}, {-12, 16}, {-8, 3},
                                      {-17, -6}, {-6, -6}};
   check_favorite_init(vd);

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[idx], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);

   //Inner Star Body
   evas_vg_shape_shape_reset(vd->shape[idx]);
   evas_vg_shape_shape_append_move_to(vd->shape[idx],
                                      center_x + ELM_SCALE_SIZE(STAR_PT[0][0]),
                                      center_y + ELM_SCALE_SIZE(STAR_PT[0][1]));
   int i;
   for (i = 1; i < PT_CNT; i++)
     {
        evas_vg_shape_shape_append_line_to(vd->shape[idx],
                                      center_x + ELM_SCALE_SIZE(STAR_PT[i][0]),
                                      center_y + ELM_SCALE_SIZE(STAR_PT[i][1]));
     }
   evas_vg_shape_shape_append_close(vd->shape[idx]);

}

static void
check_favorite_vg_resize_cb(void *data, Evas *e EINA_UNUSED,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   check_favorite *vd = data;
   check_favorite_shape_do(vd, 0);
}

static void
check_favorite_vg2_resize_cb(void *data, Evas *e EINA_UNUSED,
                             Evas_Object *obj EINA_UNUSED,
                             void *event_info EINA_UNUSED)
{
   check_favorite *vd = data;
   if (!elm_check_state_get(vd->obj)) return;
   check_favorite_shape_do(vd, 1);
}

static void
transit_check_favorite_del_cb(void *data, Elm_Transit *transit EINA_UNUSED)
{
   check_favorite *vd = data;
   vd->transit = NULL;
}

static void
transit_check_favorite_op(Elm_Transit_Effect *effect,
                          Elm_Transit *transit EINA_UNUSED, double progress)
{
   check_favorite *vd = effect;
   check_favorite_shape_do(vd, 1);

   if (!elm_check_state_get(vd->obj)) progress = 1 - progress;

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[1], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);

   Eina_Matrix3 m;
   eina_matrix3_identity(&m);
   eina_matrix3_translate(&m, center_x, center_y);
   eina_matrix3_scale(&m, progress, progress);
   eina_matrix3_translate(&m, -center_x, -center_y);
   evas_vg_node_transformation_set(vd->shape[1], &m);
}

static void
check_favorite_changed_cb(void *data, Evas_Object *obj,
                          void *event_info EINA_UNUSED)
{
   check_favorite *vd = data;

   Eina_Bool check = elm_check_state_get(obj);

   //Circle Effect
   elm_transit_del(vd->transit);
   vd->transit = elm_transit_add();
   elm_transit_effect_add(vd->transit, transit_check_favorite_op, vd,
                          NULL);
   elm_transit_del_cb_set(vd->transit, transit_check_favorite_del_cb, vd);
   elm_transit_tween_mode_set(vd->transit,
                              ELM_TRANSIT_TWEEN_MODE_DECELERATE);
   if (check)
     {
        elm_transit_duration_set(vd->transit, 0.3);
        elm_transit_go_in(vd->transit, 0.1);
     }
   else
     {
        elm_transit_duration_set(vd->transit, 0.3);
        elm_transit_go(vd->transit);
     }
}

static void
check_favorite_del_cb(void *data, Evas *e EINA_UNUSED,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   check_favorite *vd = data;
   evas_object_data_set(vd->obj, vg_key, NULL);
   evas_object_smart_callback_del(vd->obj, "changed",
                                  check_favorite_changed_cb);
   elm_transit_del(vd->transit);
   free(vd);
}

void
tizen_vg_check_favorite_set(Elm_Check *obj)
{
   check_favorite *vd = calloc(1, sizeof(check_favorite));
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }
   evas_object_data_set(obj, vg_key, vd);
   evas_object_smart_callback_add(obj, "changed", check_favorite_changed_cb,
                                  vd);
   //Outline Star
   vd->vg[0] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_DEL,
                                  check_favorite_del_cb, vd);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  check_favorite_vg_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg[0]);

   //Inner Body Star
   vd->vg[1] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[1], EVAS_CALLBACK_RESIZE,
                                  check_favorite_vg2_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->vg[1]);

   vd->obj = obj;
}


/////////////////////////////////////////////////////////////////////////
/* Check: OnOff */
/////////////////////////////////////////////////////////////////////////
typedef struct check_onoff_s
{
   Evas_Object *vg[3]; //0: bg, 1: overlapped circle, 2: line-circle
   Efl_VG_Shape *shape[4];  //0: bg, 1: overlapped circle, 2: line, 3: circle
   Elm_Transit *transit[3];  //0: circle, 1: line, 2: overlapped circle
   Evas_Object *obj;
   Eina_Bool init : 1;
} check_onoff;


static void
check_onoff_init(check_onoff *vd)
{
   if (vd->init) return;
   vd->init = EINA_TRUE;

   //BG Circle
   vd->shape[0] = evas_vg_shape_add(evas_object_vg_root_node_get(vd->vg[0]));
   evas_vg_node_color_set(vd->shape[0], 255, 255, 255, 255);

   //Overlap Circle
   vd->shape[1] = evas_vg_shape_add(evas_object_vg_root_node_get(vd->vg[1]));
   evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255);

   //Line Shape
   vd->shape[2] = evas_vg_shape_add(evas_object_vg_root_node_get(vd->vg[2]));
   evas_vg_shape_stroke_color_set(vd->shape[2], 255, 255, 255, 255);
   evas_vg_shape_stroke_width_set(vd->shape[2], ELM_SCALE_SIZE(2));
   evas_vg_shape_stroke_cap_set(vd->shape[2], EFL_GFX_CAP_ROUND);

   //Circle Shape
   vd->shape[3] = evas_vg_shape_add(evas_object_vg_root_node_get(vd->vg[2]));
   evas_vg_shape_stroke_color_set(vd->shape[3], 255, 255, 255, 255);
   evas_vg_shape_stroke_width_set(vd->shape[3], ELM_SCALE_SIZE(2));
}

static void
transit_check_onoff_circle_op(Elm_Transit_Effect *effect,
                              Elm_Transit *transit EINA_UNUSED, double progress)
{
   check_onoff *vd = effect;

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[2], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);

   evas_vg_shape_shape_reset(vd->shape[3]);

   double radius =
      ELM_SCALE_SIZE((center_x > center_y ? center_x : center_y) - 2);

   evas_vg_shape_shape_append_circle(vd->shape[3], center_x, center_y,
                                     radius);

   if (elm_check_state_get(vd->obj)) progress = 1 - progress;

   Eina_Matrix3 m;
   eina_matrix3_identity(&m);
   eina_matrix3_translate(&m, center_x, center_y);
   eina_matrix3_scale(&m, progress, progress);
   eina_matrix3_translate(&m, -center_x, -center_y);
   evas_vg_node_transformation_set(vd->shape[3], &m);
}

static void
transit_check_onoff_circle_del_cb(void *data, Elm_Transit *transit EINA_UNUSED)
{
   check_onoff *vd = data;
   vd->transit[0] = NULL;
}

static void
transit_check_onoff_line_op(void *data, Elm_Transit *transit EINA_UNUSED,
                            double progress)
{
   check_onoff *vd = data;

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[2], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);

   evas_vg_shape_shape_reset(vd->shape[2]);

   if (!elm_check_state_get(vd->obj)) progress = 1 - progress;

   double diff = ELM_SCALE_SIZE(center_y - 2);

   evas_vg_shape_shape_append_move_to(vd->shape[2], center_x,
                                      (center_y - (diff * progress)));
   evas_vg_shape_shape_append_line_to(vd->shape[2], center_x,
                                      (center_y + (diff * progress)));
}

static void
transit_check_onoff_line_del_cb(void *data, Elm_Transit *transit EINA_UNUSED)
{
   check_onoff *vd = data;
   vd->transit[1] = NULL;
}


static void
transit_check_onoff_sizing_op(void *data, Elm_Transit *transit EINA_UNUSED,
                              double progress)
{
   check_onoff *vd = data;

   check_onoff_init(vd);

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[1], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);


   evas_vg_shape_shape_reset(vd->shape[1]);
   evas_vg_shape_shape_append_circle(vd->shape[1],
                                     center_x, center_y,
                                     center_x);

   if (!elm_check_state_get(vd->obj)) progress = 1 - progress;
   progress *= 0.3;

   Eina_Matrix3 m;
   eina_matrix3_identity(&m);
   eina_matrix3_translate(&m, center_x, center_y);
   eina_matrix3_scale(&m, 0.7 + progress, 0.7 + progress);
   eina_matrix3_translate(&m, -center_x, -center_y);
   evas_vg_node_transformation_set(vd->shape[1], &m);
}

static void
transit_check_onoff_sizing_del_cb(void *data,
                                  Elm_Transit *transit EINA_UNUSED)
{
   check_onoff *vd = data;
   vd->transit[2] = NULL;
}

static void
check_onoff_changed_cb(void *data, Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   check_onoff *vd = data;

   check_onoff_init(vd);

   Eina_Bool check = elm_check_state_get(obj);

   //Circle Effect
   elm_transit_del(vd->transit[0]);
   vd->transit[0] = elm_transit_add();
   elm_transit_effect_add(vd->transit[0], transit_check_onoff_circle_op, vd,
                          NULL);
   elm_transit_del_cb_set(vd->transit[0], transit_check_onoff_circle_del_cb,
                          vd);
   elm_transit_tween_mode_set(vd->transit[0],
                              ELM_TRANSIT_TWEEN_MODE_DECELERATE);
   if (check)
     {
        elm_transit_duration_set(vd->transit[0], 0.1);
        elm_transit_go(vd->transit[0]);
     }
   else
     {
        elm_transit_duration_set(vd->transit[0], 0.25);
        elm_transit_go_in(vd->transit[0], 0.05);
     }

   //Line Effect
   elm_transit_del(vd->transit[1]);
   vd->transit[1] = elm_transit_add();
   elm_transit_effect_add(vd->transit[1], transit_check_onoff_line_op, vd,
                          NULL);
   elm_transit_del_cb_set(vd->transit[1], transit_check_onoff_line_del_cb,
                          vd);
   elm_transit_tween_mode_set(vd->transit[1],
                              ELM_TRANSIT_TWEEN_MODE_DECELERATE);
   if (check)
     {
        elm_transit_duration_set(vd->transit[1], 0.25);
        elm_transit_go_in(vd->transit[1], 0.05);
     }
   else
     {
        elm_transit_duration_set(vd->transit[1], 0.1);
        elm_transit_go(vd->transit[1]);
     }

   //Overlap Circle Sizing Effect
   elm_transit_del(vd->transit[2]);
   vd->transit[2] = elm_transit_add();
   elm_transit_effect_add(vd->transit[2], transit_check_onoff_sizing_op, vd,
                          NULL);
   elm_transit_del_cb_set(vd->transit[2], transit_check_onoff_sizing_del_cb,
                          vd);
   elm_transit_tween_mode_set(vd->transit[2],
                              ELM_TRANSIT_TWEEN_MODE_DECELERATE);
   elm_transit_duration_set(vd->transit[2], 0.3);
   elm_transit_go(vd->transit[2]);
}

static void
check_onoff_del_cb(void *data, Evas *e EINA_UNUSED,
                   Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   check_onoff *vd = data;
   evas_object_data_set(vd->obj, vg_key, NULL);
   evas_object_smart_callback_del(vd->obj, "changed", check_onoff_changed_cb);
   elm_transit_del(vd->transit[0]);
   elm_transit_del(vd->transit[1]);
   elm_transit_del(vd->transit[2]);
   free(vd);
}

static void
check_onoff_vg_resize_cb(void *data, Evas *e EINA_UNUSED,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   check_onoff *vd = data;

   check_onoff_init(vd);

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[0], NULL, NULL, &w, &h);
   Evas_Coord center_x = (w / 2);
   Evas_Coord center_y = (h / 2);

   evas_vg_shape_shape_reset(vd->shape[0]);
   evas_vg_shape_shape_append_circle(vd->shape[0], center_x, center_y,
                                     center_x);
}

static void
check_onoff_vg2_resize_cb(void *data, Evas *e EINA_UNUSED,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   check_onoff *vd = data;

   check_onoff_init(vd);

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[1], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);

   evas_vg_shape_shape_reset(vd->shape[1]);
   evas_vg_shape_shape_append_circle(vd->shape[1],
                                     center_x, center_y,
                                     center_x);
}

static void
check_onoff_vg3_resize_cb(void *data, Evas *e EINA_UNUSED,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   check_onoff *vd = data;

   check_onoff_init(vd);

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[2], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);

   evas_vg_shape_shape_reset(vd->shape[2]);
   evas_vg_shape_shape_reset(vd->shape[3]);

   //Line
   if (elm_check_state_get(vd->obj))
     {
        double diff = ELM_SCALE_SIZE(2);

        evas_vg_shape_shape_append_move_to(vd->shape[2], center_x, diff);
        evas_vg_shape_shape_append_line_to(vd->shape[2], center_x, h - diff);
     }
   //Circle
   else
     {
        double radius = ELM_SCALE_SIZE(center_x - 2);
        evas_vg_shape_shape_append_circle(vd->shape[3],
                                          center_x, center_y, radius);
     }
}

void
tizen_vg_check_onoff_set(Elm_Check *obj)
{
   check_onoff *vd = calloc(1, sizeof(check_onoff));
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }
   evas_object_data_set(obj, vg_key, vd);
   evas_object_smart_callback_add(obj, "changed", check_onoff_changed_cb, vd);

   //Base (BG) VG
   vd->vg[0] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_DEL,
                                  check_onoff_del_cb, vd);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  check_onoff_vg_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg[0]);

   //Overlapped Circle VG
   vd->vg[1] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[1], EVAS_CALLBACK_RESIZE,
                                  check_onoff_vg2_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->vg[1]);

   //Line-Circle VG
   vd->vg[2] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[2], EVAS_CALLBACK_RESIZE,
                                  check_onoff_vg3_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape3", vd->vg[2]);

   vd->obj = obj;
}



/////////////////////////////////////////////////////////////////////////
/* Check: Default */
/////////////////////////////////////////////////////////////////////////
typedef struct check_default_s
{
   Evas_Object *vg[2]; //0: base, 1: line
   Efl_VG_Shape *shape[4];  //0: outline, 1: bg, 2: left line, 3: right line
   Elm_Transit *transit[3]; //0: bg color, 1: bg scale, 2: check lines
   Evas_Object *obj;
   double left_move_to[2];
   double left_line_to[2];
   double right_move_to[2];
   double right_line_to[2];
   Eina_Bool init : 1;
} check_default;

static void
check_default_init(check_default *vd)
{
   if (vd->init) return;
   vd->init = EINA_TRUE;

   Efl_VG *base_root = evas_object_vg_root_node_get(vd->vg[0]);

   //Outline Shape
   vd->shape[0] = evas_vg_shape_add(base_root);
   evas_vg_shape_stroke_color_set(vd->shape[0], 255, 255, 255, 255);
   evas_vg_shape_stroke_width_set(vd->shape[0], ELM_SCALE_SIZE(1.25));

   //BG Shape
   vd->shape[1] = evas_vg_shape_add(base_root);

   //Check Line VG
   vd->vg[1] = evas_object_vg_add(evas_object_evas_get(vd->obj));
   elm_object_part_content_set(vd->obj, "tizen_vg_shape2", vd->vg[1]);

   Efl_VG *line_root = evas_object_vg_root_node_get(vd->vg[1]);

   //Left Line Shape
   vd->shape[2] = evas_vg_shape_add(line_root);
   evas_vg_shape_stroke_width_set(vd->shape[2], ELM_SCALE_SIZE(1.75));
   evas_vg_shape_stroke_color_set(vd->shape[2], 255, 255, 255, 255);
   evas_vg_shape_stroke_cap_set(vd->shape[2], EFL_GFX_CAP_ROUND);

   //Right Line Shape
   vd->shape[3] = evas_vg_shape_add(line_root);
   evas_vg_shape_stroke_width_set(vd->shape[3], ELM_SCALE_SIZE(1.75));
   evas_vg_shape_stroke_color_set(vd->shape[3], 255, 255, 255, 255);
   evas_vg_shape_stroke_cap_set(vd->shape[3], EFL_GFX_CAP_ROUND);

   vd->left_move_to[0] = ELM_SCALE_SIZE(-5);
   vd->left_move_to[1] = ELM_SCALE_SIZE(10);
   vd->left_line_to[0] = ELM_SCALE_SIZE(-8);
   vd->left_line_to[1] = ELM_SCALE_SIZE(-8);

   vd->right_move_to[0] = ELM_SCALE_SIZE(-5);
   vd->right_move_to[1] = ELM_SCALE_SIZE(10);
   vd->right_line_to[0] = ELM_SCALE_SIZE(18);
   vd->right_line_to[1] = ELM_SCALE_SIZE(-18);
}

static void
check_default_vg_resize_cb(void *data, Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   check_default *vd = data;

   check_default_init(vd);

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[0], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);

   //Update Outline Shape
   evas_vg_shape_shape_reset(vd->shape[0]);
   evas_vg_shape_shape_append_rect(vd->shape[0], 1, 1, w - 2, h -2, 5, 5);

   //Update BG Shape
   evas_vg_shape_shape_reset(vd->shape[1]);
   evas_vg_shape_shape_append_rect(vd->shape[1], 0, 0, w, h, 5, 5);
   if (elm_check_state_get(vd->obj))
     evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255);
   else
     evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 0);

   //Update Line Shape
   if (elm_check_state_get(vd->obj))
     {
        //Left
        evas_vg_shape_shape_reset(vd->shape[2]);
        evas_vg_shape_shape_append_move_to(vd->shape[2],
                                           center_x + vd->left_move_to[0],
                                           center_y + vd->left_move_to[1]);
        evas_vg_shape_shape_append_line_to(vd->shape[2],
                                           (center_x + vd->left_move_to[0]) +
                                           (vd->left_line_to[0]),
                                           (center_y + vd->left_move_to[1]) +
                                           (vd->left_line_to[1]));
        //Right
        evas_vg_shape_shape_reset(vd->shape[3]);
        evas_vg_shape_shape_append_move_to(vd->shape[3],
                                           center_x + vd->right_move_to[0],
                                           center_y + vd->right_move_to[1]);
        evas_vg_shape_shape_append_line_to(vd->shape[3],
                                           (center_x + vd->right_move_to[0]) +
                                           (vd->right_line_to[0]),
                                           (center_y + vd->right_move_to[1]) +
                                           (vd->right_line_to[1]));
     }
}

static void
transit_check_default_bg_color_del_cb(Elm_Transit_Effect *effect,
                                      Elm_Transit *transit EINA_UNUSED)
{
   check_default *vd = effect;
   vd->transit[0] = NULL;
}

static void
transit_check_default_bg_color_op(Elm_Transit_Effect *effect,
                                  Elm_Transit *transit EINA_UNUSED,
                                  double progress)
{
   check_default *vd = effect;

   if (elm_check_state_get(vd->obj))
     evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255 * progress);
   else
     evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255 * (1 - progress));
}

static void
transit_check_default_bg_scale_del_cb(Elm_Transit_Effect *effect,
                                      Elm_Transit *transit EINA_UNUSED)
{
   check_default *vd = effect;
   vd->transit[1] = NULL;
}

static void
transit_check_default_bg_scale_op(Elm_Transit_Effect *effect,
                                  Elm_Transit *transit EINA_UNUSED,
                                  double progress)
{
   check_default *vd = effect;

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[0], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);

   Eina_Matrix3 m;
   eina_matrix3_identity(&m);
   eina_matrix3_translate(&m, center_x, center_y);
   eina_matrix3_scale(&m, progress, progress);
   eina_matrix3_translate(&m, -center_x, -center_y);
   evas_vg_node_transformation_set(vd->shape[1], &m);
}

static void
transit_check_default_line_del_cb(Elm_Transit_Effect *effect,
                                  Elm_Transit *transit EINA_UNUSED)
{
   check_default *vd = effect;
   vd->transit[2] = NULL;
}

static void
transit_check_default_line_op(Elm_Transit_Effect *effect,
                              Elm_Transit *transit EINA_UNUSED, double progress)
{
   check_default *vd = effect;

   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[1], NULL, NULL, &w, &h);
   double center_x = ((double)w / 2);
   double center_y = ((double)h / 2);

   //Update Line Shape
   if (!elm_check_state_get(vd->obj)) progress = 1 - progress;

   //Left
   evas_vg_shape_shape_reset(vd->shape[2]);
   evas_vg_shape_shape_append_move_to(vd->shape[2],
                                      center_x + vd->left_move_to[0],
                                      center_y + vd->left_move_to[1]);
   evas_vg_shape_shape_append_line_to(vd->shape[2],
                                      (center_x + vd->left_move_to[0]) +
                                      (vd->left_line_to[0] * progress),
                                      (center_y + vd->left_move_to[1]) +
                                      (vd->left_line_to[1] * progress));

   //Right
   evas_vg_shape_shape_reset(vd->shape[3]);
   evas_vg_shape_shape_append_move_to(vd->shape[3],
                                      center_x + vd->right_move_to[0],
                                      center_y + vd->right_move_to[1]);
   evas_vg_shape_shape_append_line_to(vd->shape[3],
                                      (center_x + vd->right_move_to[0]) +
                                      (vd->right_line_to[0] * progress),
                                      (center_y + vd->right_move_to[1]) +
                                      (vd->right_line_to[1] * progress));
}

static void
check_default_changed_cb(void *data, Evas_Object *obj,
                         void *event_info EINA_UNUSED)
{
   check_default *vd = data;

   check_default_init(vd);

   Eina_Bool check = elm_check_state_get(obj);

   //BG Color Effect
   elm_transit_del(vd->transit[0]);
   vd->transit[0] = elm_transit_add();
   elm_transit_effect_add(vd->transit[0], transit_check_default_bg_color_op, vd,
                          NULL);
   elm_transit_del_cb_set(vd->transit[0], transit_check_default_bg_color_del_cb,
                          vd);
   elm_transit_tween_mode_set(vd->transit[0],
                              ELM_TRANSIT_TWEEN_MODE_DECELERATE);

   if (check)
     {
        elm_transit_duration_set(vd->transit[0], 0.3);
        elm_transit_go(vd->transit[0]);
     }
   else
     {
        elm_transit_duration_set(vd->transit[0], 0.3);
        elm_transit_go_in(vd->transit[0], 0.15);
     }

   //BG Size Effect
   elm_transit_del(vd->transit[1]);

   if (check)
     {
        vd->transit[1] = elm_transit_add();
        elm_transit_effect_add(vd->transit[1],
                               transit_check_default_bg_scale_op, vd, NULL);
        elm_transit_del_cb_set(vd->transit[1],
                               transit_check_default_bg_scale_del_cb, vd);
        elm_transit_tween_mode_set(vd->transit[1],
                                   ELM_TRANSIT_TWEEN_MODE_DECELERATE);
        elm_transit_duration_set(vd->transit[1], 0.15);
        elm_transit_go(vd->transit[1]);
     }

   //Draw Line
   elm_transit_del(vd->transit[2]);

   vd->transit[2] = elm_transit_add();
   elm_transit_effect_add(vd->transit[2], transit_check_default_line_op, vd,
                          NULL);
   elm_transit_del_cb_set(vd->transit[2], transit_check_default_line_del_cb,
                          vd);
   elm_transit_tween_mode_set(vd->transit[2],
                              ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
   elm_transit_duration_set(vd->transit[2], 0.15);

   if (check)
     elm_transit_go_in(vd->transit[2], 0.15);
   else
     elm_transit_go(vd->transit[2]);
}

static void
check_default_del_cb(void *data, Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   check_default *vd = data;
   evas_object_data_set(vd->obj, vg_key, NULL);
   evas_object_smart_callback_del(vd->obj, "changed", check_default_changed_cb);
   elm_transit_del(vd->transit[0]);
   elm_transit_del(vd->transit[1]);
   elm_transit_del(vd->transit[2]);

   free(vd);
}

void
tizen_vg_check_default_set(Elm_Check *obj)
{
   check_default *vd = calloc(1, sizeof(check_default));
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }
   evas_object_data_set(obj, vg_key, vd);

   evas_object_smart_callback_add(obj, "changed", check_default_changed_cb, vd);

   //Base VG
   vd->vg[0] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_DEL,
                                  check_default_del_cb, vd);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  check_default_vg_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg[0]);

   vd->obj = obj;
}

void
tizen_vg_check_set(Elm_Check *obj)
{
   check_default *vd = evas_object_data_get(obj, vg_key);
   if (vd) evas_object_del(vd->vg[0]);

   //Apply vector ux only theme has "vector_ux" "on"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;
   if (strcmp(str, "on")) return;

   const char *style = elm_object_style_get(obj);

   if (!strcmp(style, "default"))
     tizen_vg_check_default_set(obj);
   else if (!strcmp(style, "on&off"))
     tizen_vg_check_onoff_set(obj);
   else if (!strcmp(style, "favorite"))
     tizen_vg_check_favorite_set(obj);
}



/////////////////////////////////////////////////////////////////////////
/* Button */
/////////////////////////////////////////////////////////////////////////
typedef struct vg_button_s
{
   Evas_Object *vg[2];       //0: base, 1: effect
   Efl_VG_Shape *shape[2];   //0: base, 1: effect
   Evas_Object *obj;
   Eina_Bool init : 1;
} vg_button;

static void
button_init(vg_button *vd)
{
   if (vd->init) return;
   vd->init = EINA_TRUE;

   //Base Shape
   Efl_VG *base_root = evas_object_vg_root_node_get(vd->vg[0]);
   vd->shape[0] = evas_vg_shape_add(base_root);
   evas_vg_node_color_set(vd->shape[0], 255, 255, 255, 255);

   //Effect Shape
   Efl_VG *effect_root = evas_object_vg_root_node_get(vd->vg[1]);
   vd->shape[1] = evas_vg_shape_add(effect_root);
   evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255);
}

static void
button_effect_resize_cb(void *data, Evas *e EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   vg_button *vd = data;

   button_init(vd);

   //Update Effect Shape
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(vd->vg[1], &x, &y, &w, &h);
   evas_vg_shape_shape_reset(vd->shape[1]);
   evas_vg_shape_shape_append_rect(vd->shape[1], 0, 0, w, h, 35, 100);
}

static void
button_base_resize_cb(void *data, Evas *e EINA_UNUSED,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   vg_button *vd = data;

   button_init(vd);

   //Update Base Shape
   Evas_Coord w, h;
   evas_object_geometry_get(vd->vg[0], NULL, NULL, &w, &h);
   evas_vg_shape_shape_reset(vd->shape[0]);
   evas_vg_shape_shape_append_rect(vd->shape[0], 0, 0, w, h, 35, 100);
}

static void
button_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   vg_button *vd = data;
   evas_object_data_set(vd->obj, vg_key, NULL);
   free(vd);
}

void
tizen_vg_button_set(Elm_Button *obj)
{
   vg_button *vd = evas_object_data_get(obj, vg_key);
   if (vd) evas_object_del(vd->vg[0]);

   //Apply vector ux only theme has "vector_ux" "on"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;
   if (strcmp(str, "on")) return;

   vd = calloc(1, sizeof(vg_button));
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }
   evas_object_data_set(obj, vg_key, vd);

   //Vector Graphics Object
   Evas *e = evas_object_evas_get(obj);

   //Base VG
   vd->vg[0] = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_DEL,
                                  button_del_cb, vd);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  button_base_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg[0]);

   //Effect VG
   vd->vg[1] = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->vg[1], EVAS_CALLBACK_RESIZE,
                                  button_effect_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->vg[1]);

   vd->obj = obj;
}

#endif
