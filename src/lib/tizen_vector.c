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
typedef struct vg_box_button_s
{
   Evas_Object *vg;
   Efl_VG_Shape *shape;
   Evas_Object *obj;
} vg_box_button;

static void
box_button_update(vg_box_button *vd)
{
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(vd->vg, &x, &y, &w, &h);
   evas_vg_shape_shape_append_rect(vd->shape, 0, 0, w, h, 40, 100);
   evas_vg_node_color_set(vd->shape, 255, 255, 255, 255);
}

static void
box_button_resize_cb(void *data, Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   vg_box_button *vd = data;
   box_button_update(vd);
}

static void
box_button_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   vg_box_button *vd = data;
   evas_object_data_set(vd->obj, vg_key, NULL);
   free(vd);
}

void
tizen_vg_box_button_set(Elm_Button *obj)
{
   //Apply vector ux only theme has "vector_ux" "on"
   const char *str = elm_layout_data_get(obj, "vector_ux");
   if (!str) return;
   if (strcmp(str, "on")) return;

   vg_box_button *vd = evas_object_data_get(obj, vg_key);
   if (vd) evas_object_del(vd->vg);

   vd = calloc(1, sizeof(vg_box_button));
   if (!vd)
     {
        ERR("Failed to allocate vector graphics data memory");
        return;
     }
   evas_object_data_set(obj, vg_key, vd);

   //Vector Graphics Object
   Evas *e = evas_object_evas_get(obj);
   vd->vg = evas_object_vg_add(e);
   evas_object_event_callback_add(vd->vg, EVAS_CALLBACK_DEL,
                                  box_button_del_cb, vd);
   evas_object_event_callback_add(vd->vg, EVAS_CALLBACK_RESIZE,
                                  box_button_resize_cb, vd);
   elm_object_part_content_set(obj, "tizen_vg_shape", vd->vg);

   Efl_VG *root = evas_object_vg_root_node_get(vd->vg);
   vd->shape = evas_vg_shape_add(root);
   vd->obj = obj;
}

#endif
