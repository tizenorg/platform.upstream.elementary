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

   //Apply vector ux only theme has "vector_ux"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;

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

   vd->obj = obj;

   //Outline VG
   vd->vg[0] = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_DEL,
                                  radio_del_cb, vd);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  radio_base_resize_cb, vd);
   //Center Circle
   vd->vg[1] = evas_object_vg_add(e);

   //Iconic Circle
   vd->vg[2] = evas_object_vg_add(e);

   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg[0]);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->vg[1]);
   elm_object_part_content_set(obj, "tizen_vg_shape3", vd->vg[2]);
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
   vd->obj = obj;

   //Outline Star
   vd->vg[0] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_DEL,
                                  check_favorite_del_cb, vd);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  check_favorite_vg_resize_cb, vd);
   //Inner Body Star
   vd->vg[1] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[1], EVAS_CALLBACK_RESIZE,
                                  check_favorite_vg2_resize_cb, vd);

   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg[0]);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->vg[1]);
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

   vd->obj = obj;

   //Base (BG) VG
   vd->vg[0] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_DEL,
                                  check_onoff_del_cb, vd);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  check_onoff_vg_resize_cb, vd);
   //Overlapped Circle VG
   vd->vg[1] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[1], EVAS_CALLBACK_RESIZE,
                                  check_onoff_vg2_resize_cb, vd);
   //Line-Circle VG
   vd->vg[2] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[2], EVAS_CALLBACK_RESIZE,
                                  check_onoff_vg3_resize_cb, vd);

   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg[0]);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->vg[1]);
   elm_object_part_content_set(obj, "tizen_vg_shape3", vd->vg[2]);
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
     evas_vg_node_color_set(vd->shape[1], 0, 0, 0, 0);

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
   int color;

   if (elm_check_state_get(vd->obj)) color = 255 * progress;
   else color = 255 * (1 - progress);

   evas_vg_node_color_set(vd->shape[1], color, color, color, color);
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
check_default_changed_cb(void *data, Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED,
                         const char *source EINA_UNUSED)
{
   check_default *vd = data;

   if (!source) return;
   if (strcmp(source, "tizen_vg")) return;

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
   elm_object_signal_callback_del(vd->obj, "elm,check,action,toggle", "tizen_vg", check_default_changed_cb);
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

   elm_object_signal_callback_add(obj, "elm,check,action,toggle", "tizen_vg", check_default_changed_cb, vd);

   vd->obj = obj;

   //Base VG
   vd->vg[0] = evas_object_vg_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_DEL,
                                  check_default_del_cb, vd);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  check_default_vg_resize_cb, vd);
   //Check Line VG
   vd->vg[1] = evas_object_vg_add(evas_object_evas_get(obj));

   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg[0]);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->vg[1]);
}

void
tizen_vg_check_set(Elm_Check *obj)
{
   check_default *vd = evas_object_data_get(obj, vg_key);
   if (vd) evas_object_del(vd->vg[0]);

   //Apply vector ux only theme has "vector_ux"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;

   if (!strcmp(str, "default"))
     tizen_vg_check_default_set(obj);
   else if (!strcmp(str, "on&off"))
     tizen_vg_check_onoff_set(obj);
   else if (!strcmp(str, "favorite"))
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
   evas_object_data_del(vd->obj, vg_key);
   free(vd);
}

void
tizen_vg_button_set(Elm_Button *obj)
{
   vg_button *vd = evas_object_data_get(obj, vg_key);
   if (vd)
     {
        elm_object_part_content_unset(obj, "tizen_vg_shape");
        elm_object_part_content_unset(obj, "tizen_vg_shape2");
        evas_object_del(vd->vg[0]);
        evas_object_del(vd->vg[1]);
     }

   //Apply vector ux only theme has "vector_ux"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;

   vd = calloc(1, sizeof(vg_button));
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }
   evas_object_data_set(obj, vg_key, vd);

   //Vector Graphics Object
   Evas *e = evas_object_evas_get(obj);

   vd->obj = obj;

   //Base VG
   vd->vg[0] = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_DEL,
                                  button_del_cb, vd);
   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  button_base_resize_cb, vd);
   //Effect VG
   vd->vg[1] = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->vg[1], EVAS_CALLBACK_RESIZE,
                                  button_effect_resize_cb, vd);

   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg[0]);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->vg[1]);
}

/////////////////////////////////////////////////////////////////////////
/* Progressbar */
/////////////////////////////////////////////////////////////////////////

typedef struct vg_progressbar_s
{
   Evas_Object *vg[3];       //0: base, 1: layer1, 2:layer2
   Efl_VG_Shape *shape[3];   //0: base, 1: layer1, 2: layer2
   Elm_Transit  *transit;
   Evas_Object *obj;
   Evas_Coord x, w, h;       // for normal style animation data
   double stroke_width;
   double shrink;
   double shift;
} vg_progressbar;

static void
progressbar_del_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
              void *event_info EINA_UNUSED)
{
   vg_progressbar *vd = evas_object_data_get(obj, vg_key);
   Elm_Transit *cur_transit = vd->transit;
   vd->transit = NULL;
   if (cur_transit)
     elm_transit_del(cur_transit);
   if (vd)
     {
        evas_object_data_set(obj, vg_key, NULL);
        free(vd);
     }
}

static void
transit_progressbar_normal_op1(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_progressbar *vd = effect;

   evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255);
   evas_vg_shape_shape_reset(vd->shape[1]);
   Evas_Coord start_width = vd->x + vd->h;
   evas_vg_shape_shape_append_rect(vd->shape[1], 0, 0, start_width + (vd->w - start_width)* progress, vd->h, vd->h/2, vd->h/2);

   evas_vg_node_color_set(vd->shape[2], 255, 255, 255, 255);
   evas_vg_shape_shape_reset(vd->shape[2]);
   Evas_Coord delta_width = (vd->w - vd->x - vd->h) * progress;
   evas_vg_shape_shape_append_rect(vd->shape[2], vd->x, 0, vd->h + delta_width, vd->h, vd->h/2, vd->h/2);

}

static void
transit_progressbar_normal_op2(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_progressbar *vd = effect;
   evas_vg_node_color_set(vd->shape[2], 255, 255, 255, 255);
   evas_vg_shape_shape_reset(vd->shape[2]);
   Evas_Coord delta_move =  (vd->w - vd->x - vd->h) * progress;
   evas_vg_shape_shape_append_rect(vd->shape[2], vd->x + delta_move, 0, vd->w - vd->x - delta_move, vd->h, vd->h/2, vd->h/2);
}

static void
transit_progressbar_normal_op3(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_progressbar *vd = effect;
   int c = 255 - 255*progress;
   evas_vg_node_color_set(vd->shape[2], c, c, c, c);
}

static void
progressbar_normal_bg_resize_cb(void *data , Evas *e EINA_UNUSED,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info EINA_UNUSED)
{
   Evas_Coord x, y, w, h;
   vg_progressbar *vd = data;
   evas_object_geometry_get(vd->vg[0], &x, &y, &w, &h);
   evas_vg_node_color_set(vd->shape[0], 255, 255, 255, 255);
   evas_vg_shape_shape_reset(vd->shape[0]);
   evas_vg_shape_shape_append_rect(vd->shape[0], 0, 0, w, h, h/2, h/2);
}

static void
progressbar_normal_fg_resize_cb(void *data, Evas *e EINA_UNUSED,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info EINA_UNUSED)
{
   Evas_Coord x, y, w, h;
   vg_progressbar *vd = data;
   evas_object_geometry_get(vd->vg[1], &x, &y, &w, &h);
   if (w < 2) return;

   if (vd->w >= w) vd->x = 0;
   else vd->x = vd->w - vd->h;
   vd->w = w;
   vd->h = h;
   Elm_Transit *transit1 = elm_transit_add();
   elm_transit_object_add(transit1, obj);
   elm_transit_effect_add(transit1, transit_progressbar_normal_op1, vd, NULL);
   elm_transit_duration_set(transit1, 0.4);
   elm_transit_objects_final_state_keep_set(transit1, EINA_TRUE);


   Elm_Transit *transit2 = elm_transit_add();
   elm_transit_object_add(transit2, obj);
   elm_transit_effect_add(transit2, transit_progressbar_normal_op2, vd, NULL);
   elm_transit_duration_set(transit2, 0.3);
   elm_transit_objects_final_state_keep_set(transit2, EINA_TRUE);

   Elm_Transit *transit3 = elm_transit_add();
   elm_transit_object_add(transit3, obj);
   elm_transit_effect_add(transit3, transit_progressbar_normal_op3, vd, NULL);
   elm_transit_duration_set(transit3, 0.3);
   elm_transit_objects_final_state_keep_set(transit3, EINA_TRUE);

   elm_transit_chain_transit_add(transit1, transit2);
   elm_transit_chain_transit_add(transit2, transit3);

   elm_transit_go(transit1);

}

static void
_progressbar_normal_style(vg_progressbar *vd)
{
   Efl_VG *root;
   int i;
   Evas *e = evas_object_evas_get(vd->obj);

   for(i=0; i < 3; i++)
     {
        vd->vg[i] = evas_object_vg_add(e);
        root = evas_object_vg_root_node_get(vd->vg[i]);
        vd->shape[i] = evas_vg_shape_add(root);
     }

   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  progressbar_normal_bg_resize_cb, vd);
   evas_object_event_callback_add(vd->vg[1], EVAS_CALLBACK_RESIZE,
                                  progressbar_normal_fg_resize_cb, vd);

   //unset
   elm_object_part_content_unset(vd->obj, "tizen_vg_shape1");
   elm_object_part_content_unset(vd->obj, "tizen_vg_shape2");
   elm_object_part_content_unset(vd->obj, "tizen_vg_shape3");

   elm_object_part_content_set(vd->obj, "tizen_vg_shape1", vd->vg[0]);
   elm_object_part_content_set(vd->obj, "tizen_vg_shape2", vd->vg[1]);
   elm_object_part_content_set(vd->obj, "tizen_vg_shape3", vd->vg[2]);
}

static void
progressbar_process_resize_cb(void *data , Evas *e EINA_UNUSED,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info EINA_UNUSED)
{
   Evas_Coord x, y, w, h,i;
   vg_progressbar *vd = data;
   evas_object_geometry_get(vd->vg[0], &x, &y, &w, &h);
   for(i=0; i < 3; i++)
     {
        evas_vg_shape_shape_reset(vd->shape[i]);
        evas_vg_shape_stroke_color_set(vd->shape[i], 255, 255, 255, 255);
        evas_vg_shape_shape_append_arc(vd->shape[i], 0, 0, w - vd->shrink, h - vd->shrink, 90.5, -1);
        evas_vg_shape_stroke_width_set(vd->shape[i], vd->stroke_width);
        evas_vg_node_origin_set(vd->shape[i], vd->shift, vd->shift);
     }
}

static void
transit_progressbar_process_A_op1(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_progressbar *vd = effect;
   Evas_Coord x, y, w, h;
   double s_a = 90.5, e_a = 90.5, s_l = -1 , e_l = -126;
   evas_object_geometry_get(vd->vg[0], &x, &y, &w, &h);
   double start_angle = s_a + (e_a - s_a) * progress;
   double sweep_length = s_l + (e_l - s_l) * progress;
   evas_vg_shape_shape_reset(vd->shape[2]);
   evas_vg_shape_shape_append_arc(vd->shape[2], 0, 0, w - vd->shrink, h - vd->shrink, start_angle, sweep_length);
}

static void
transit_progressbar_process_A_op2(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_progressbar *vd = effect;
   Evas_Coord x, y, w, h;
   double s_a = 90.5, e_a = -170, s_l = -126 , e_l = -100;
   evas_object_geometry_get(vd->vg[0], &x, &y, &w, &h);
   double start_angle = s_a + (e_a - s_a) * progress;
   double sweep_length = s_l + (e_l - s_l) * progress;
   evas_vg_shape_shape_reset(vd->shape[2]);
   evas_vg_shape_shape_append_arc(vd->shape[2], 0, 0, w - vd->shrink, h - vd->shrink, start_angle, sweep_length);
}

static void
transit_progressbar_process_A_op3(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_progressbar *vd = effect;
   Evas_Coord x, y, w, h;
   double s_a = -170, e_a = -269.5, s_l = -100 , e_l = -1;
   evas_object_geometry_get(vd->vg[0], &x, &y, &w, &h);
   double start_angle = s_a + (e_a - s_a) * progress;
   double sweep_length = s_l + (e_l - s_l) * progress;
   evas_vg_shape_shape_reset(vd->shape[2]);
   evas_vg_shape_shape_append_arc(vd->shape[2], 0, 0, w - vd->shrink, h - vd->shrink, start_angle, sweep_length);
}

static void
transit_progressbar_process_B_op1(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_progressbar *vd = effect;
   Evas_Coord x, y, w, h;
   double s_a = 90.5, e_a = 90.5, s_l = -1 , e_l = -172.8;
   evas_object_geometry_get(vd->vg[0], &x, &y, &w, &h);
   double start_angle = s_a + (e_a - s_a) * progress;
   double sweep_length = s_l + (e_l - s_l) * progress;
   evas_vg_shape_shape_reset(vd->shape[1]);
   evas_vg_shape_shape_append_arc(vd->shape[1], 0, 0, w - vd->shrink, h - vd->shrink, start_angle, sweep_length);
}

static void
transit_progressbar_process_B_op2(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_progressbar *vd = effect;
   Evas_Coord x, y, w, h;
   double s_a = 90.5, e_a = -129.7, s_l = -172.8 , e_l = -140.3;
   evas_object_geometry_get(vd->vg[0], &x, &y, &w, &h);
   double start_angle = s_a + (e_a - s_a) * progress;
   double sweep_length = s_l + (e_l - s_l) * progress;
   evas_vg_shape_shape_reset(vd->shape[1]);
   evas_vg_shape_shape_append_arc(vd->shape[1], 0, 0, w - vd->shrink, h - vd->shrink, start_angle, sweep_length);
}

static void
transit_progressbar_process_B_op3(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_progressbar *vd = effect;
   Evas_Coord x, y, w, h;
   double s_a = -129.7, e_a = -269.5, s_l = -140.3 , e_l = -1;
   evas_object_geometry_get(vd->vg[0], &x, &y, &w, &h);
   double start_angle = s_a + (e_a - s_a) * progress;
   double sweep_length = s_l + (e_l - s_l) * progress;
   evas_vg_shape_shape_reset(vd->shape[1]);
   evas_vg_shape_shape_append_arc(vd->shape[1], 0, 0, w - vd->shrink, h - vd->shrink, start_angle, sweep_length);
}

static void
transit_progressbar_process_C_op1(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_progressbar *vd = effect;
   Evas_Coord x, y, w, h;
   double s_a = 89.5, e_a = -270.5, s_l = -1 , e_l = -1;
   evas_object_geometry_get(vd->vg[0], &x, &y, &w, &h);
   double start_angle = s_a + (e_a - s_a) * progress;
   double sweep_length = s_l + (e_l - s_l) * progress;
   evas_vg_shape_shape_reset(vd->shape[0]);
   evas_vg_shape_shape_append_arc(vd->shape[0], 0, 0, w - vd->shrink, h - vd->shrink, start_angle, sweep_length);
}

static void _transit_progreassbar_process_end(Elm_Transit_Effect *effect, Elm_Transit *transit);

static void
_progressbar_process_pulse_start_helper(vg_progressbar *vd)
{
   // For Layer A animation
   Elm_Transit *transit1 = elm_transit_add();
   elm_transit_object_add(transit1, vd->obj);
   elm_transit_effect_add(transit1, transit_progressbar_process_A_op1, vd, NULL);
   elm_transit_duration_set(transit1, 0.35);
   elm_transit_objects_final_state_keep_set(transit1, EINA_TRUE);

   Elm_Transit *transit2 = elm_transit_add();
   elm_transit_object_add(transit2, vd->obj);
   elm_transit_effect_add(transit2, transit_progressbar_process_A_op2, vd, NULL);
   elm_transit_duration_set(transit2, 0.65);
   elm_transit_objects_final_state_keep_set(transit2, EINA_TRUE);

   Elm_Transit *transit3 = elm_transit_add();
   elm_transit_object_add(transit3, vd->obj);
   elm_transit_effect_add(transit3, transit_progressbar_process_A_op3, vd, NULL);
   elm_transit_duration_set(transit3, 0.25);
   elm_transit_objects_final_state_keep_set(transit3, EINA_TRUE);

   elm_transit_chain_transit_add(transit1, transit2);
   elm_transit_chain_transit_add(transit2, transit3);

   elm_transit_go(transit1);

   // For Layer B Animation
   transit1 = elm_transit_add();
   elm_transit_object_add(transit1, vd->obj);
   elm_transit_effect_add(transit1, transit_progressbar_process_B_op1, vd, NULL);
   elm_transit_duration_set(transit1, 0.48);
   elm_transit_objects_final_state_keep_set(transit1, EINA_TRUE);

   transit2 = elm_transit_add();
   elm_transit_object_add(transit2, vd->obj);
   elm_transit_effect_add(transit2, transit_progressbar_process_B_op2, vd, NULL);
   elm_transit_duration_set(transit2, 0.52);
   elm_transit_objects_final_state_keep_set(transit2, EINA_TRUE);

   transit3 = elm_transit_add();
   elm_transit_object_add(transit3, vd->obj);
   elm_transit_effect_add(transit3, transit_progressbar_process_B_op3, vd, NULL);
   elm_transit_duration_set(transit3, 0.33);
   elm_transit_objects_final_state_keep_set(transit3, EINA_TRUE);

   elm_transit_chain_transit_add(transit1, transit2);
   elm_transit_chain_transit_add(transit2, transit3);

   elm_transit_go(transit1);

   // For Layer C Animation
   transit1 = elm_transit_add();
   elm_transit_object_add(transit1, vd->obj);
   elm_transit_effect_add(transit1, transit_progressbar_process_C_op1, vd, _transit_progreassbar_process_end);
   elm_transit_duration_set(transit1, 0.85);
   elm_transit_objects_final_state_keep_set(transit1, EINA_TRUE);
   vd->transit = transit1;
   elm_transit_go_in(transit1, .54);
}

static void
_transit_progreassbar_process_end(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED)
{
   vg_progressbar *vd = effect;
   if (!vd->transit) return;
   vd->transit = NULL;
   _progressbar_process_pulse_start_helper(vd);
}

static void
_progressbar_process_pulse_start(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   _progressbar_process_pulse_start_helper(data);
}

static void
_progressbar_process_pulse_stop(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   vg_progressbar *vd = data;
   Elm_Transit *cur_transit = vd->transit;
   vd->transit = NULL;
   if (cur_transit)
     elm_transit_del(cur_transit);
}

static void
_progressbar_process_style(vg_progressbar *vd)
{
   Efl_VG *root;
   int i;
   Evas *e = evas_object_evas_get(vd->obj);

   for(i=0; i < 3; i++)
     {
        vd->vg[i] = evas_object_vg_add(e);
        root = evas_object_vg_root_node_get(vd->vg[i]);
        vd->shape[i] = evas_vg_shape_add(root);
        evas_vg_shape_stroke_color_set(vd->shape[i], 255, 255, 255, 255);
        evas_vg_shape_stroke_cap_set(vd->shape[i], EFL_GFX_CAP_ROUND);
     }

   evas_object_event_callback_add(vd->vg[0], EVAS_CALLBACK_RESIZE,
                                  progressbar_process_resize_cb, vd);

   elm_object_part_content_set(vd->obj, "tizen_vg_shape1", vd->vg[0]);
   elm_object_part_content_set(vd->obj, "tizen_vg_shape2", vd->vg[1]);
   elm_object_part_content_set(vd->obj, "tizen_vg_shape3", vd->vg[2]);

   elm_object_signal_callback_add(vd->obj, "elm,state,pulse,start",
                                  "*", _progressbar_process_pulse_start, vd);

   elm_object_signal_callback_add(vd->obj, "elm,state,pulse,stop",
                                   "*", _progressbar_process_pulse_stop, vd);

}

void
tizen_vg_progressbar_set(Elm_Progressbar *obj)
{
   vg_progressbar *vd = evas_object_data_get(obj, vg_key);
   if (vd)
     {
        if (vd->vg[0]) evas_object_del(vd->vg[0]);
        if (vd->vg[1]) evas_object_del(vd->vg[1]);
        if (vd->vg[2]) evas_object_del(vd->vg[2]);
        vd->x = 0;
     }

   //Apply vector ux only theme has "vector_ux"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;

   if (!vd)
     {
        vd = calloc(1, sizeof(vg_progressbar));
        evas_object_data_set(obj, vg_key, vd);
        vd->obj = obj;
        // callback to free vd data
        evas_object_event_callback_add(vd->obj, EVAS_CALLBACK_DEL,
                                       progressbar_del_cb, NULL);
     }
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }


   if (!strcmp(str, "list_progress"))
     {
        _progressbar_normal_style(vd);
        return;
     }
   if (!strcmp(str, "process_large") ||
       !strcmp(str, "process_medium") ||
       !strcmp(str, "process_small"))
     {
        vd->stroke_width = ELM_SCALE_SIZE(3);
        if (!strcmp(str, "process_medium"))
          {
             vd->stroke_width = ELM_SCALE_SIZE(2);
          }
        if (!strcmp(str, "process_small"))
          {
             vd->stroke_width = ELM_SCALE_SIZE(1.5);
          }
        vd->shrink = 2 * vd->stroke_width;
        vd->shift = vd->stroke_width + 1;
        _progressbar_process_style(vd);
        return;
     }
}

/////////////////////////////////////////////////////////////////////////
/* Slider */
/////////////////////////////////////////////////////////////////////////

typedef struct vg_slider_s
{
   Evas_Object *vg[7];
   Efl_VG_Shape *shape[7];
   Evas_Object *obj;
   Eina_Stringshare *style;
} vg_slider;

static int slider_base = 0;
static int slider_level = 1;
static int slider_level2 = 2;
static int slider_level_rest = 3;
static int slider_center = 4;
static int slider_handle = 5;
static int slider_handle_pressed = 6;

static void
slider_del_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
              void *event_info EINA_UNUSED)
{
   vg_slider *vd = evas_object_data_get(obj, vg_key);
   if (vd)
     {
        evas_object_data_set(obj, vg_key, NULL);
        free(vd);
     }
}

static void
_append_circle(Efl_VG *shape, int w, int h)
{
   Evas_Coord center_x = (w / 2);
   Evas_Coord center_y = (h / 2);
   double radius = ELM_SCALE_SIZE((center_x > center_y ? center_x : center_y) - 2);
   evas_vg_shape_shape_reset(shape);
   evas_vg_shape_shape_append_circle(shape, center_x, center_y, radius);
}

static void
_append_round_rect(Efl_VG *shape, int w, int h)
{
   double radius = w/2 > h/2 ? h/2 : w/2;
   evas_vg_shape_shape_reset(shape);
   evas_vg_shape_shape_append_rect(shape, 0, 0, w, h, radius, radius);
}

static void
slider_vg_handle_normal_resize_cb(void *data , Evas *e EINA_UNUSED,
                                  Evas_Object *obj EINA_UNUSED,
                                  void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   vg_slider *vd = data;
   evas_object_geometry_get(vd->vg[slider_handle], NULL, NULL, &w, &h);
   _append_circle(vd->shape[slider_handle], w, h);
}

static void
slider_vg_handle_pressed_resize_cb(void *data , Evas *e EINA_UNUSED,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   vg_slider *vd = data;
   evas_object_geometry_get(vd->vg[slider_handle_pressed], NULL, NULL, &w, &h);
   if (w == h)
     _append_circle(vd->shape[slider_handle_pressed], w, h);
   else
     _append_round_rect(vd->shape[slider_handle_pressed], w, h);
}

static void
slider_base_resize_cb(void *data , Evas *e EINA_UNUSED,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   vg_slider *vd = data;
   evas_object_geometry_get(vd->vg[slider_base], NULL, NULL, &w, &h);
   _append_round_rect(vd->shape[slider_base], w, h);
}

static void
slider_center_resize_cb(void *data , Evas *e EINA_UNUSED,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   vg_slider *vd = data;
   evas_object_geometry_get(vd->vg[slider_center], NULL, NULL, &w, &h);
   _append_round_rect(vd->shape[slider_center], w, h);
}

static void
slider_level_resize_cb(void *data , Evas *e EINA_UNUSED,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   vg_slider *vd = data;
   evas_object_geometry_get(vd->vg[slider_level], NULL, NULL, &w, &h);
   _append_round_rect(vd->shape[slider_level], w, h);
}

static void
slider_level2_resize_cb(void *data , Evas *e EINA_UNUSED,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   vg_slider *vd = data;
   evas_object_geometry_get(vd->vg[slider_level2], NULL, NULL, &w, &h);
   _append_round_rect(vd->shape[slider_level2], w, h);
}

static void
slider_level_rest_resize_cb(void *data , Evas *e EINA_UNUSED,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   vg_slider *vd = data;
   evas_object_geometry_get(vd->vg[slider_level_rest], NULL, NULL, &w, &h);
   _append_round_rect(vd->shape[slider_level_rest], w, h);
}

static void
_slider_create_handle(vg_slider *vd)
{
   Efl_VG *root;
   int i;
   Evas *e = evas_object_evas_get(vd->obj);
   for(i=0; i < 7; i++)
     {
        vd->vg[i] = evas_object_vg_add(e);
        root = evas_object_vg_root_node_get(vd->vg[i]);
        vd->shape[i] = evas_vg_shape_add(root);
        evas_vg_node_color_set(vd->shape[i], 255, 255, 255, 255);

     }
   // slider base
   evas_object_event_callback_add(vd->vg[slider_base], EVAS_CALLBACK_RESIZE,
                                  slider_base_resize_cb, vd);
   elm_object_part_content_set(vd->obj, "elm.swallow.tizen_vg_shape1", vd->vg[slider_base]);

   // level
   evas_object_event_callback_add(vd->vg[slider_level], EVAS_CALLBACK_RESIZE,
                                  slider_level_resize_cb, vd);
   elm_object_part_content_set(vd->obj, "elm.swallow.tizen_vg_shape2", vd->vg[slider_level]);

   // level2
   evas_object_event_callback_add(vd->vg[slider_level2], EVAS_CALLBACK_RESIZE,
                                  slider_level2_resize_cb, vd);
   elm_object_part_content_set(vd->obj, "elm.swallow.tizen_vg_shape3", vd->vg[slider_level2]);

   // level rest only for warning type
   evas_object_event_callback_add(vd->vg[slider_level_rest], EVAS_CALLBACK_RESIZE,
                                  slider_level_rest_resize_cb, vd);
   elm_object_part_content_set(vd->obj, "elm.swallow.tizen_vg_shape4", vd->vg[slider_level_rest]);

   // center point
   evas_object_event_callback_add(vd->vg[slider_center], EVAS_CALLBACK_RESIZE,
                                  slider_center_resize_cb, vd);
   elm_object_part_content_set(vd->obj, "elm.swallow.tizen_vg_shape5", vd->vg[slider_center]);

   // slider handle
   evas_object_event_callback_add(vd->vg[slider_handle], EVAS_CALLBACK_RESIZE,
                                  slider_vg_handle_normal_resize_cb, vd);
   evas_object_event_callback_add(vd->vg[slider_handle_pressed], EVAS_CALLBACK_RESIZE,
                                  slider_vg_handle_pressed_resize_cb, vd);
   elm_object_part_content_set(vd->obj, "elm.dragable.slider:elm.swallow.tizen_vg_shape1", vd->vg[slider_handle]);
   elm_object_part_content_set(vd->obj, "elm.dragable.slider:elm.swallow.tizen_vg_shape2", vd->vg[slider_handle_pressed]);
}

void
tizen_vg_slider_set(Elm_Slider *obj)
{
   EINA_LOG_CRIT("slider VG creation");
   vg_slider *vd = evas_object_data_get(obj, vg_key);
   if (vd)
     {
        int i;
        for(i=0; i < 7; i++)
          if (vd->vg[i]) evas_object_del(vd->vg[i]);
        eina_stringshare_del(vd->style);
     }

   //Apply vector ux only theme has "vector_ux" "on"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;

   if (!vd)
     {
        vd = calloc(1, sizeof(vg_slider));
        evas_object_data_set(obj, vg_key, vd);
        vd->obj = obj;
        // callback to free vd data
        evas_object_event_callback_add(vd->obj, EVAS_CALLBACK_DEL,
                                       slider_del_cb, NULL);
     }
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }

   vd->style = eina_stringshare_add(str);

   _slider_create_handle(vd);
}
#endif
