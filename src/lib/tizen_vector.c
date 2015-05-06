#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define EFL_BETA_API_SUPPORT 1

#ifdef TIZEN_VECTOR_UX

static const char *vg_key = "_tizen_vg";


/////////////////////////////////////////////////////////////////////////
/* Check */
/////////////////////////////////////////////////////////////////////////
typedef struct vg_check_s
{
   Evas_Object *vg;
   Efl_VG_Shape *shape[4];  //0: outline, 1: bg, 2: left line, 3: right line
   Elm_Transit *transit[1]; //0: bg color,
   Evas_Object *obj;
} vg_check;


static void
check_base_resize_cb(void *data, Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   vg_check *vd = data;

   Evas_Coord x, y, w, h;
   evas_object_geometry_get(vd->vg, &x, &y, &w, &h);

   //Update Outline Shape
   evas_vg_shape_shape_reset(vd->shape[0]);
   evas_vg_shape_shape_append_rect(vd->shape[0], 1, 1, w - 2, h -2, 10, 10);

   //Update BG Shape
   evas_vg_shape_shape_reset(vd->shape[1]);
   evas_vg_shape_shape_append_rect(vd->shape[1], 0, 0, w, h, 10, 10);
   if (elm_check_state_get(vd->obj))
     evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255);
   else
     evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 0);
}

static void
transit_check_bg_color_end(Elm_Transit_Effect *effect,
                           Elm_Transit *transit EINA_UNUSED)
{
   vg_check *vd = effect;
   vd->transit[0] = NULL;
}

static void
transit_check_bg_color_op(Elm_Transit_Effect *effect,
                          Elm_Transit *transit EINA_UNUSED, double progress)
{
   vg_check *vd = effect;

   if (elm_check_state_get(vd->obj))
     evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255 * progress);
   else
     evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255 * (1 - progress));
}

static void
check_changed_cb(void *data, Evas_Object *obj,
                 void *event_info EINA_UNUSED)
{
   vg_check *vd = data;

   //BG Color Effect
   elm_transit_del(vd->transit[0]);

   vd->transit[0] = elm_transit_add();
   elm_transit_effect_add(vd->transit[0], transit_check_bg_color_op, vd,
                          transit_check_bg_color_end);
   elm_transit_tween_mode_set(vd->transit[0],
                              ELM_TRANSIT_TWEEN_MODE_DECELERATE);

   if (elm_check_state_get(obj))
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

}

static void
check_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   vg_check *vd = data;
   evas_object_data_set(vd->obj, vg_key, NULL);
   evas_object_smart_callback_del(vd->obj, "changed", check_changed_cb);
   elm_transit_del(vd->transit[0]);
   free(vd);
}

void
tizen_vg_check_set(Elm_Check *obj)
{
   //Apply vector ux only theme has "vector_ux" "on"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;
   if (strcmp(str, "on")) return;

   vg_check *vd = evas_object_data_get(obj, vg_key);
   if (vd) evas_object_del(vd->vg);

   vd = calloc(1, sizeof(vg_check));
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }
   evas_object_data_set(obj, vg_key, vd);

   evas_object_smart_callback_add(obj, "changed", check_changed_cb, vd);

   Evas *e = evas_object_evas_get(obj);

   //Vector Object
   vd->vg = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->vg, EVAS_CALLBACK_DEL,
                                  check_del_cb, vd);
   evas_object_event_callback_add(vd->vg, EVAS_CALLBACK_RESIZE,
                                  check_base_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg);

   Efl_VG *base_root = evas_object_vg_root_node_get(vd->vg);

   //Outline Shape
   vd->shape[0] = evas_vg_shape_add(base_root);
   evas_vg_shape_stroke_color_set(vd->shape[0], 255, 255, 255, 255);
   evas_vg_shape_stroke_width_set(vd->shape[0], ELM_SCALE_SIZE(1.25));

   //BG Shape
   vd->shape[1] = evas_vg_shape_add(base_root);

   //Left Line Shape
//   vd->shape[2] = evas_vg_shape_add(base_root);

   //Right Line Shape
//   vd->shape[3] = evas_vg_shape_add(base_root);

   vd->obj = obj;
}




/////////////////////////////////////////////////////////////////////////
/* Button */
/////////////////////////////////////////////////////////////////////////
typedef struct vg_button_s
{
   Evas_Object *vg[2];       //0: base, 1: effect
   Efl_VG_Shape *shape[2];   //0: base, 1: effect
   Evas_Object *obj;
} vg_button;

static void
button_effect_resize_cb(void *data, Evas *e EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   vg_button *vd = data;

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

   //Update Base Shape
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(vd->vg[0], &x, &y, &w, &h);
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
   //Apply vector ux only theme has "vector_ux" "on"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;
   if (strcmp(str, "on")) return;

   vg_button *vd = evas_object_data_get(obj, vg_key);
   if (vd) evas_object_del(vd->vg[0]);

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

   //Base Shape
   Efl_VG *base_root = evas_object_vg_root_node_get(vd->vg[0]);
   vd->shape[0] = evas_vg_shape_add(base_root);
   evas_vg_node_color_set(vd->shape[0], 255, 255, 255, 255);

   //Effect VG
   vd->vg[1] = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->vg[1], EVAS_CALLBACK_RESIZE,
                                  button_effect_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->vg[1]);

   //Effect Shape
   Efl_VG *effect_root = evas_object_vg_root_node_get(vd->vg[1]);
   vd->shape[1] = evas_vg_shape_add(effect_root);
   evas_vg_node_color_set(vd->shape[1], 255, 255, 255, 255);
   vd->obj = obj;
}

#endif
