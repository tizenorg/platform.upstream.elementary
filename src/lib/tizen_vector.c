#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define EFL_BETA_API_SUPPORT 1

#ifdef TIZEN_VECTOR_UX

static const char *vg_key = "_tizen_vg";



/////////////////////////////////////////////////////////////////////////
/* Box Button */
/////////////////////////////////////////////////////////////////////////
typedef struct vg_button_s
{
   Evas_Object *base_vg;
   Evas_Object *effect_vg;
   Efl_VG_Shape *base_shape;
   Efl_VG_Shape *effect_shape;
   Evas_Object *obj;
} vg_button;

static void
button_base_update(vg_button *vd)
{
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(vd->base_vg, &x, &y, &w, &h);
   //Base Shape
   evas_vg_shape_shape_reset(vd->base_shape);
   evas_vg_shape_shape_append_rect(vd->base_shape, 0, 0, w, h, 35, 100);
}

static void
button_effect_update(vg_button *vd)
{
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(vd->effect_vg, &x, &y, &w, &h);
   //Effect Shape
   evas_vg_shape_shape_reset(vd->effect_shape);
   evas_vg_shape_shape_append_rect(vd->effect_shape, 0, 0, w, h, 35, 100);
}

static void
button_effect_resize_cb(void *data, Evas *e EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   vg_button *vd = data;
   button_effect_update(vd);
}

static void
button_base_resize_cb(void *data, Evas *e EINA_UNUSED,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   vg_button *vd = data;
   button_base_update(vd);
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
   if (vd) evas_object_del(vd->base_vg);

   vd = calloc(1, sizeof(vg_button));
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }
   evas_object_data_set(obj, vg_key, vd);

   //Vector Graphics Object
   Evas *e = evas_object_evas_get(obj);

   //Base Shape
   vd->base_vg = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->base_vg, EVAS_CALLBACK_DEL,
                                  button_del_cb, vd);
   evas_object_event_callback_add(vd->base_vg, EVAS_CALLBACK_RESIZE,
                                  button_base_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape", vd->base_vg);

   Efl_VG *base_root = evas_object_vg_root_node_get(vd->base_vg);
   vd->base_shape = evas_vg_shape_add(base_root);
   evas_vg_node_color_set(vd->base_shape, 255, 255, 255, 255);

   //Effect Shape
   vd->effect_vg = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->effect_vg, EVAS_CALLBACK_RESIZE,
                                  button_effect_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape2", vd->effect_vg);

   Efl_VG *effect_root = evas_object_vg_root_node_get(vd->effect_vg);
   vd->effect_shape = evas_vg_shape_add(effect_root);
   evas_vg_node_color_set(vd->effect_shape, 255, 255, 255, 255);
   vd->obj = obj;
}

#endif
