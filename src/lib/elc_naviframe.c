#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_naviframe.h"
#include "elm_widget_container.h"

EAPI Eo_Op ELM_OBJ_NAVIFRAME_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_NAVIFRAME_CLASS

#define MY_CLASS_NAME "Elm_Naviframe"
#define MY_CLASS_NAME_LEGACY "elm_naviframe"

static const char CONTENT_PART[] = "elm.swallow.content";
static const char PREV_BTN_PART[] = "elm.swallow.prev_btn";
static const char NEXT_BTN_PART[] = "elm.swallow.next_btn";
static const char ICON_PART[] = "elm.swallow.icon";
static const char TITLE_PART[] = "elm.text.title";
static const char SUBTITLE_PART[] = "elm.text.subtitle";
static const char TITLE_ACCESS_PART[] = "access.title";

static const char SIG_TRANSITION_FINISHED[] = "transition,finished";
static const char SIG_TITLE_CLICKED[] = "title,clicked";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_TRANSITION_FINISHED, ""},
   {SIG_TITLE_CLICKED, ""},
   {"focused", ""}, /**< handled by elm_widget */
   {"unfocused", ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static const char SIG_CLICKED[] = "clicked";

static void _on_item_back_btn_clicked(void *data, Evas_Object *obj, void *event_info __UNUSED__);

static void
_resize_object_reset(Evas_Object *obj, Elm_Naviframe_Item *it)
{
   if (it)
     {
        elm_widget_resize_object_set(obj, VIEW(it), EINA_FALSE);
        evas_object_raise(VIEW(it));
     }
}

static void
_prev_page_focus_recover(Elm_Naviframe_Item *it)
{
   Evas_Object *newest;
   unsigned int order = 0;

   newest = elm_widget_newest_focus_order_get(VIEW(it), &order, EINA_TRUE);
   if (newest)
     elm_object_focus_set(newest, EINA_TRUE);
   else
     {
        if (elm_object_focus_allow_get(VIEW(it)))
          elm_object_focus_set(VIEW(it), EINA_TRUE);
        else
          elm_object_focus_set(WIDGET(it), EINA_TRUE);
     }
}

static void
_elm_naviframe_smart_translate(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Naviframe_Smart_Data *sd = _pd;
   Elm_Naviframe_Item *it;

   EINA_INLIST_FOREACH(sd->stack, it)
     elm_widget_item_translate(it);

   if (ret) *ret = EINA_TRUE;
}

static void
_item_content_del_cb(void *data,
                     Evas *e __UNUSED__,
                     Evas_Object *obj __UNUSED__,
                     void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->content = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,content,hide", "elm");
}

static void
_item_title_prev_btn_del_cb(void *data,
                            Evas *e __UNUSED__,
                            Evas_Object *obj __UNUSED__,
                            void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->title_prev_btn = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,hide", "elm");
}

static void
_item_title_next_btn_del_cb(void *data,
                            Evas *e __UNUSED__,
                            Evas_Object *obj __UNUSED__,
                            void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->title_next_btn = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,next_btn,hide", "elm");
}

static void
_item_title_icon_del_cb(void *data,
                        Evas *e __UNUSED__,
                        Evas_Object *obj __UNUSED__,
                        void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->title_icon = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,icon,hide", "elm");
}

static void
_title_content_del(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   char buf[1024];
   Elm_Naviframe_Content_Item_Pair *pair = data;
   Elm_Naviframe_Item *it = pair->it;
   snprintf(buf, sizeof(buf), "elm,state,%s,hide", pair->part);
   elm_object_signal_emit(VIEW(it), buf, "elm");
   it->content_list = eina_inlist_remove(it->content_list,
                                         EINA_INLIST_GET(pair));
   eina_stringshare_del(pair->part);
   free(pair);
}

static void
_item_free(Elm_Naviframe_Item *it)
{
   Eina_Inlist *l;
   Elm_Naviframe_Content_Item_Pair *content_pair;
   Elm_Naviframe_Text_Item_Pair *text_pair;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   eina_stringshare_del(it->style);
   eina_stringshare_del(it->title_label);
   eina_stringshare_del(it->subtitle_label);

   EINA_INLIST_FOREACH_SAFE(it->content_list, l, content_pair)
     {
        if (content_pair->content)
          {
             evas_object_event_callback_del(content_pair->content,
                                            EVAS_CALLBACK_DEL,
                                            _title_content_del);
             evas_object_del(content_pair->content);
          }
        eina_stringshare_del(content_pair->part);
        free(content_pair);
     }
   EINA_INLIST_FOREACH_SAFE(it->text_list, l, text_pair)
     {
        eina_stringshare_del(text_pair->part);
        free(text_pair);
     }

   if (it->content)
     {
        if ((sd->preserve) && (!sd->on_deletion))
          {
             /* so that elm does not delete the contents with the item's
              * view after the del_pre_hook */
             elm_object_part_content_unset(VIEW(it), CONTENT_PART);
             evas_object_event_callback_del
                (it->content, EVAS_CALLBACK_DEL, _item_content_del_cb);
          }
     }
}

static void
_item_content_signals_emit(Elm_Naviframe_Item *it)
{
   Elm_Naviframe_Content_Item_Pair *content_pair;
   char buf[1024];
   //content
   if (it->content)
     elm_object_signal_emit(VIEW(it), "elm,state,content,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,content,hide", "elm");

   //prev button
   if (it->title_prev_btn)
     elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,hide", "elm");

   //next button
   if (it->title_next_btn)
     elm_object_signal_emit(VIEW(it), "elm,state,next_btn,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,next_btn,hide", "elm");

   if (it->title_icon)
     elm_object_signal_emit(VIEW(it), "elm,state,icon,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,icon,hide", "elm");

   EINA_INLIST_FOREACH(it->content_list, content_pair)
     {
        if (content_pair->content)
          snprintf(buf, sizeof(buf), "elm,state,%s,show", content_pair->part);
        else
          snprintf(buf, sizeof(buf), "elm,state,%s,hide", content_pair->part);
        elm_object_signal_emit(VIEW(it), buf, "elm");
     }
}

static void
_item_text_signals_emit(Elm_Naviframe_Item *it)
{
   Elm_Naviframe_Text_Item_Pair *text_pair;
   char buf[1024];

   if ((it->title_label) && (it->title_label[0]))
     elm_object_signal_emit(VIEW(it), "elm,state,title_label,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,title_label,hide", "elm");

   if ((it->subtitle_label) && (it->subtitle_label[0]))
     elm_object_signal_emit(VIEW(it), "elm,state,subtitle,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,subtitle,hide", "elm");

   EINA_INLIST_FOREACH(it->text_list, text_pair)
     {
        if (elm_object_part_text_get(VIEW(it), text_pair->part))
          snprintf(buf, sizeof(buf), "elm,state,%s,show", text_pair->part);
        else
          snprintf(buf, sizeof(buf), "elm,state,%s,hide", text_pair->part);
        elm_object_signal_emit(VIEW(it), buf, "elm");
     }
}

static Evas_Object *
_access_object_get(Elm_Naviframe_Item *it, const char* part)
{
   Evas_Object *po, *ao;

   po = (Evas_Object *)edje_object_part_object_get
          (elm_layout_edje_get(VIEW(it)), part);
   ao = evas_object_data_get(po, "_part_access_obj");

   return ao;
}

static void
_item_signals_emit(Elm_Naviframe_Item *it)
{
   _item_text_signals_emit(it);
   _item_content_signals_emit(it);
}

/* FIXME: we need to handle the case when this function is called
 * during a transition */
static void
_item_style_set(Elm_Naviframe_Item *it,
                const char *item_style)
{
   char buf[256];

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   if (!item_style)
     {
        strcpy(buf, "item/basic");
        eina_stringshare_replace(&it->style, "basic");
     }
   else
     {
        snprintf(buf, sizeof(buf), "item/%s", item_style);
        eina_stringshare_replace(&it->style, item_style);
     }

   if (!elm_layout_theme_set(VIEW(it), "naviframe", buf,
                             elm_widget_style_get(WIDGET(it))))
     CRITICAL("Failed to set layout!");

   if (sd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);
}

static void
_item_title_visible_update(Elm_Naviframe_Item *nit)
{
   if (nit->title_visible)
     elm_object_signal_emit(VIEW(nit), "elm,state,title,show", "elm");
   else
     elm_object_signal_emit(VIEW(nit), "elm,state,title,hide", "elm");
}

static void
_elm_naviframe_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Elm_Naviframe_Item *it;
   Elm_Naviframe_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   const char *style, *sstyle;

   eo_do(obj, elm_wdg_style_get(&style));

   EINA_INLIST_FOREACH(sd->stack, it)
     {
        eo_do(VIEW(it), elm_wdg_style_get(&sstyle));
        if ((style && sstyle) && strcmp(style, sstyle))
          _item_style_set(it, it->style);
        _item_signals_emit(it);
        _item_title_visible_update(it);
     }

   elm_layout_sizing_eval(obj);
   if (ret) *ret = EINA_TRUE;
}

static char *
_access_info_cb(void *data, Evas_Object *obj __UNUSED__)
{
   Elm_Naviframe_Item *nit;
   Evas_Object *layout;
   Eina_Strbuf *buf;
   const char *info;
   char *ret;

   nit = data;
   if (!nit->title_visible) return NULL;

   layout = VIEW(nit);
   info = elm_object_part_text_get(layout, TITLE_PART);
   if (!info) return NULL;

   buf = eina_strbuf_new();
   eina_strbuf_append(buf, info);

   info = elm_object_part_text_get(layout, SUBTITLE_PART);
   if (!info) goto end;

   eina_strbuf_append_printf(buf, ", %s", info);

end:
   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_access_obj_process(Elm_Naviframe_Item *it, Eina_Bool is_access)
{
   Evas_Object *ao, *eo;

   if (is_access && (it->title_label || it->subtitle_label))
     {
        if (!_access_object_get(it, TITLE_ACCESS_PART))
          {
             eo = elm_layout_edje_get(VIEW(it));
             ao =_elm_access_edje_object_part_object_register(WIDGET(it), eo,
                                                            TITLE_ACCESS_PART);
            _elm_access_text_set(_elm_access_info_get(ao),
                                ELM_ACCESS_TYPE, E_("Title"));
            _elm_access_callback_set(_elm_access_info_get(ao),
                                     ELM_ACCESS_INFO, _access_info_cb, it);
            /* to access title access object, any idea? */
            ((Elm_Widget_Item *)it)->access_obj = ao;
         }
     }
   else
     {
        /* to access title access object, any idea? */
        ao = ((Elm_Widget_Item *)it)->access_obj;
        if (!ao) return;

        if (it->title_label || it->subtitle_label)
          _elm_access_edje_object_part_object_unregister
             (WIDGET(it), elm_layout_edje_get(VIEW(it)), TITLE_ACCESS_PART);
        evas_object_del(ao);
     }
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;
   Elm_Naviframe_Text_Item_Pair *pair = NULL;
   char buf[1024];

   if ((!part) || (!strcmp(part, "default")) ||
       (!strcmp(part, TITLE_PART)))
     {
        eina_stringshare_replace(&nit->title_label, label);
        if (label)
          elm_object_signal_emit(VIEW(it), "elm,state,title_label,show", "elm");
        else
          elm_object_signal_emit(VIEW(it), "elm,state,title_label,hide", "elm");
        elm_object_part_text_set(VIEW(it), TITLE_PART, label);
     }
   else if (!strcmp("subtitle", part))
     {
        eina_stringshare_replace(&nit->subtitle_label, label);
        if (label)
          elm_object_signal_emit(VIEW(it), "elm,state,subtitle,show", "elm");
        else
          elm_object_signal_emit(VIEW(it), "elm,state,subtitle,hide", "elm");
        elm_object_part_text_set(VIEW(it), SUBTITLE_PART, label);
     }
   else
     {
        EINA_INLIST_FOREACH(nit->text_list, pair)
          if (!strcmp(part, pair->part)) break;

        if (!pair)
          {
             pair = ELM_NEW(Elm_Naviframe_Text_Item_Pair);
             if (!pair)
               {
                  ERR("Failed to allocate new text part of the item! : naviframe=%p",
                  WIDGET(it));
                  return;
               }
             eina_stringshare_replace(&pair->part, part);
             nit->text_list = eina_inlist_append(nit->text_list,
                                                 EINA_INLIST_GET(pair));
          }
        if (label)
          snprintf(buf, sizeof(buf), "elm,state,%s,show", part);
        else
          snprintf(buf, sizeof(buf), "elm,state,%s,hide", part);
        elm_object_signal_emit(VIEW(it), buf, "elm");
        elm_object_part_text_set(VIEW(it), part, label);
     }

   /* access */
   if (_elm_config->access_mode)
     _access_obj_process(nit, EINA_TRUE);

   elm_layout_sizing_eval(WIDGET(nit));
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   char buf[1024];

   if (!part || !strcmp(part, "default"))
     snprintf(buf, sizeof(buf), TITLE_PART);
   else if (!strcmp("subtitle", part))
     snprintf(buf, sizeof(buf), SUBTITLE_PART);
   else
     snprintf(buf, sizeof(buf), "%s", part);

   return elm_object_part_text_get(VIEW(it), buf);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit, *prev_it = NULL;
   Eina_Bool top;

   nit = (Elm_Naviframe_Item *)it;
   ELM_NAVIFRAME_DATA_GET(WIDGET(nit), sd);

   if (nit->animator) ecore_animator_del(nit->animator);

   top = (it == elm_naviframe_top_item_get(WIDGET(nit)));
   if (evas_object_data_get(VIEW(nit), "out_of_list"))
     goto end;

   //FIXME: Really need?
   if (!sd->on_deletion)
     {
        nit->unfocusable = elm_widget_tree_unfocusable_get(VIEW(nit));
        elm_widget_tree_unfocusable_set(VIEW(nit), EINA_TRUE);
     }

   sd->stack = eina_inlist_remove(sd->stack, EINA_INLIST_GET(nit));

   if (top && !sd->on_deletion) /* must raise another one */
     {
        if (sd->stack && sd->stack->last)
          prev_it = EINA_INLIST_CONTAINER_GET(sd->stack->last,
                                              Elm_Naviframe_Item);
        if (!prev_it) goto end;

        if (sd->freeze_events)
          evas_object_freeze_events_set(VIEW(prev_it), EINA_FALSE);
        _resize_object_reset(WIDGET(prev_it), prev_it);
        evas_object_show(VIEW(prev_it));

        _prev_page_focus_recover(prev_it);

        elm_object_signal_emit(VIEW(prev_it), "elm,state,visible", "elm");
     }

end:
   _item_free(nit);

   return EINA_TRUE;
}

static void
_item_content_set(Elm_Naviframe_Item *it,
                  Evas_Object *content)
{
   if (it->content == content) return;

   if (it->content) evas_object_del(it->content);
   it->content = content;

   if (!content) return;

   elm_object_part_content_set(VIEW(it), CONTENT_PART, content);
   elm_object_signal_emit(VIEW(it), "elm,state,content,show", "elm");

   evas_object_event_callback_add
     (content, EVAS_CALLBACK_DEL, _item_content_del_cb, it);
}

static void
_item_title_prev_btn_set(Elm_Naviframe_Item *it,
                         Evas_Object *btn)
{
   if (it->title_prev_btn == btn) return;
   if (it->title_prev_btn) evas_object_del(it->title_prev_btn);
   it->title_prev_btn = btn;
   if (!btn) return;

   elm_object_part_content_set(VIEW(it), PREV_BTN_PART, btn);
   elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,show", "elm");
   evas_object_event_callback_add
     (btn, EVAS_CALLBACK_DEL, _item_title_prev_btn_del_cb, it);

   //FIXME: set back button callback here after elm 2.0
}

static void
_item_title_next_btn_set(Elm_Naviframe_Item *it,
                         Evas_Object *btn)
{
   if (it->title_next_btn == btn) return;
   if (it->title_next_btn) evas_object_del(it->title_next_btn);
   it->title_next_btn = btn;
   if (!btn) return;

   elm_object_part_content_set(VIEW(it), NEXT_BTN_PART, btn);
   elm_object_signal_emit(VIEW(it), "elm,state,next_btn,show", "elm");

   evas_object_event_callback_add
     (btn, EVAS_CALLBACK_DEL, _item_title_next_btn_del_cb, it);
}

static void
_item_title_icon_set(Elm_Naviframe_Item *it,
                     Evas_Object *icon)
{
   if (it->title_icon == icon) return;
   if (it->title_icon) evas_object_del(it->title_icon);
   it->title_icon = icon;
   if (!icon) return;

   elm_object_part_content_set(VIEW(it), ICON_PART, icon);
   elm_object_signal_emit(VIEW(it), "elm,state,icon,show", "elm");

   evas_object_event_callback_add
     (icon, EVAS_CALLBACK_DEL, _item_title_icon_del_cb, it);
}

static Evas_Object *
_item_content_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->content;

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), CONTENT_PART);
   elm_object_signal_emit(VIEW(it), "elm,state,content,hide", "elm");

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_content_del_cb);

   it->content = NULL;
   return content;
}

static Evas_Object *
_item_title_prev_btn_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_prev_btn;

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), PREV_BTN_PART);
   elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,hide", "elm");

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_title_prev_btn_del_cb);
   evas_object_smart_callback_del(content, SIG_CLICKED,
                                  _on_item_back_btn_clicked);
   it->title_prev_btn = NULL;
   return content;
}

static Evas_Object *
_item_title_next_btn_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_next_btn;

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), NEXT_BTN_PART);
   elm_object_signal_emit(VIEW(it), "elm,state,next_btn,hide", "elm");

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_title_next_btn_del_cb);

   it->title_next_btn = NULL;
   return content;
}

static Evas_Object *
_item_title_icon_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_icon;

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), ICON_PART);
   elm_object_signal_emit(VIEW(it), "elm,state,icon,hide", "elm");

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_title_icon_del_cb);

   it->title_icon = NULL;
   return content;
}

/* since we have each item as layout, we can't reusing the layout's
 * aliasing, so let's do it ourselves */
static void
_part_aliasing_eval(const char **part)
{
   if (!*part || !strcmp("default", *part))
     *part = CONTENT_PART;
   else if (!strcmp(*part, "prev_btn"))
     *part = PREV_BTN_PART;
   else if (!strcmp(*part, "next_btn"))
     *part = NEXT_BTN_PART;
   else if (!strcmp(*part, "icon"))
     *part = ICON_PART;
}

static void
_title_content_set(Elm_Naviframe_Item *it,
                   const char *part,
                   Evas_Object *content)
{
   Elm_Naviframe_Content_Item_Pair *pair = NULL;
   char buf[1024];

   EINA_INLIST_FOREACH(it->content_list, pair)
     if (!strcmp(part, pair->part)) break;
   if (pair)
     {
        if (pair->content == content) return;
        if (pair->content)
          evas_object_event_callback_del(pair->content,
                                         EVAS_CALLBACK_DEL,
                                         _title_content_del);
        if (content) elm_object_part_content_set(VIEW(it), part, content);
     }
   else
     {
        if (!content) return;

        //Remove the pair if new content was swallowed into other part.
        EINA_INLIST_FOREACH(it->content_list, pair)
          {
             if (pair->content == content)
               {
                  eina_stringshare_del(pair->part);
                  it->content_list = eina_inlist_remove(it->content_list,
                                                        EINA_INLIST_GET(pair));
                  evas_object_event_callback_del(pair->content,
                                                 EVAS_CALLBACK_DEL,
                                                 _title_content_del);
                  free(pair);
                  break;
               }
          }

        //New pair
        pair = ELM_NEW(Elm_Naviframe_Content_Item_Pair);
        if (!pair)
          {
             ERR("Failed to allocate new content part of the item! : naviframe=%p",
             WIDGET(it));
             return;
          }
        pair->it = it;
        eina_stringshare_replace(&pair->part, part);
        it->content_list = eina_inlist_append(it->content_list,
                                              EINA_INLIST_GET(pair));
        elm_object_part_content_set(VIEW(it), part, content);
        snprintf(buf, sizeof(buf), "elm,state,%s,show", part);
        elm_object_signal_emit(VIEW(it), buf, "elm");
     }
   pair->content = content;
   evas_object_event_callback_add(content,
                                  EVAS_CALLBACK_DEL,
                                  _title_content_del,
                                  pair);
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   _part_aliasing_eval(&part);

   //specified parts
   if (!part || !strcmp(CONTENT_PART, part))
     _item_content_set(nit, content);
   else if (!strcmp(part, PREV_BTN_PART))
     _item_title_prev_btn_set(nit, content);
   else if (!strcmp(part, NEXT_BTN_PART))
     _item_title_next_btn_set(nit, content);
   else if (!strcmp(part, ICON_PART))
     _item_title_icon_set(nit, content);
   else
     _title_content_set(nit, part, content);

   elm_layout_sizing_eval(WIDGET(it));
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   _part_aliasing_eval(&part);

   //specified parts
   if (!part || !strcmp(CONTENT_PART, part))
     return nit->content;
   else if (!strcmp(part, PREV_BTN_PART))
     return nit->title_prev_btn;
   else if (!strcmp(part, NEXT_BTN_PART))
     return nit->title_next_btn;
   else if (!strcmp(part, ICON_PART))
     return nit->title_icon;

   //common parts
   return elm_object_part_content_get(VIEW(nit), part);
}

static Evas_Object *
_title_content_unset(Elm_Naviframe_Item *it, const char *part)
{
   Elm_Naviframe_Content_Item_Pair *pair = NULL;
   char buf[1028];
   Evas_Object *content = NULL;

   EINA_INLIST_FOREACH(it->content_list, pair)
     {
        if (!strcmp(part, pair->part))
          {
             content = pair->content;
             eina_stringshare_del(pair->part);
             it->content_list = eina_inlist_remove(it->content_list,
                                                   EINA_INLIST_GET(pair));
             free(pair);
             break;
          }
     }

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), part);
   snprintf(buf, sizeof(buf), "elm,state,%s,hide", part);
   elm_object_signal_emit(VIEW(it), buf, "elm");
   evas_object_event_callback_del(content,
                                  EVAS_CALLBACK_DEL,
                                  _title_content_del);
   return content;
}

static Evas_Object *
_item_content_unset_hook(Elm_Object_Item *it,
                         const char *part)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;
   Evas_Object *o = NULL;

   _part_aliasing_eval(&part);

   //specified parts
   if (!part || !strcmp(CONTENT_PART, part))
     o = _item_content_unset(nit);
   else if (!strcmp(part, PREV_BTN_PART))
     o = _item_title_prev_btn_unset(nit);
   else if (!strcmp(part, NEXT_BTN_PART))
     o = _item_title_next_btn_unset(nit);
   else if (!strcmp(part, ICON_PART))
     o = _item_title_icon_unset(nit);
   else
     o = _title_content_unset(nit, part);

   elm_layout_sizing_eval(WIDGET(it));

   return o;
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   elm_object_signal_emit(VIEW(it), emission, source);
}

static void
_elm_naviframe_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   Elm_Naviframe_Item *it, *top;
   Evas_Coord x, y, w, h;

   Elm_Naviframe_Smart_Data *sd = _pd;

   if (!sd->stack) return;

   top = (EINA_INLIST_CONTAINER_GET(sd->stack->last, Elm_Naviframe_Item));
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   EINA_INLIST_FOREACH(sd->stack, it)
     {
        evas_object_move(VIEW(it), x, y);
        evas_object_resize(VIEW(it), w, h);

        if (it == top)
          {
             edje_object_size_min_calc(elm_layout_edje_get(VIEW(it)),
                                       &it->minw, &it->minh);
             minw = it->minw;
             minh = it->minh;
          }
     }

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_on_item_back_btn_clicked(void *data,
                          Evas_Object *obj,
                          void *event_info __UNUSED__)
{
   /* Since edje has the event queue, clicked event could be happend
      multiple times on some heavy environment. This callback del will
      prevent those scenario and guarantee only one clicked for it's own
      page. */
   evas_object_smart_callback_del(obj, SIG_CLICKED, _on_item_back_btn_clicked);
   elm_naviframe_item_pop(data);
}

static Evas_Object *
_back_btn_new(Evas_Object *obj, const char *title_label)
{
   Evas_Object *btn, *ed;
   char buf[1024];

   btn = elm_button_add(obj);

   if (!btn) return NULL;
   evas_object_smart_callback_add
     (btn, SIG_CLICKED, _on_item_back_btn_clicked, obj);
   snprintf
     (buf, sizeof(buf), "naviframe/back_btn/%s", elm_widget_style_get(obj));
   elm_object_style_set(btn, buf);
   if (title_label)
     elm_layout_text_set(btn, NULL, title_label);
   else
     elm_object_domain_translatable_text_set(btn, PACKAGE, N_("Back"));

   /* HACK NOTE: this explicit check only exists to avoid an ERR()
    * message from elm_layout_content_set().
    *
    * The button was ALWAYS supposed to support an elm.swallow.content, but
    * default naviframe/back_btn/default theme did not provide such, then
    * old themes would emit such error message.
    *
    * Once we can break the theme API, remove this check and always
    * set an icon.
    */
   ed = elm_layout_edje_get(btn);
   if (edje_object_part_exists(ed, CONTENT_PART))
     {
        Evas_Object *ico = elm_icon_add(btn);
        elm_icon_standard_set(ico, "arrow_left");
        elm_layout_content_set(btn, CONTENT_PART, ico);
     }

   return btn;
}

static void
_elm_naviframe_smart_signal_emit(Eo *obj, void *_pd, va_list *list)
{
   Elm_Naviframe_Smart_Data *sd = _pd;

   const char *emission = va_arg(*list, const char *);
   const char *source = va_arg(*list, const char *);

   if (!sd->stack) return;

   eo_do_super(obj, MY_CLASS, elm_obj_layout_signal_emit(emission, source));
}

/* content/text smart functions proxying things to the top item, which
 * is the resize object of the layout */
static void
_elm_naviframe_smart_text_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Object_Item *it;

   const char *part = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret = EINA_FALSE;
   if (ret) *ret = EINA_FALSE;

   it = elm_naviframe_top_item_get(obj);
   if (!it) return;

   elm_object_item_part_text_set(it, part, label);
   int_ret = !strcmp(elm_object_item_part_text_get(it, part), label);

   if (ret) *ret = int_ret;
}

static void
_elm_naviframe_smart_text_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);
   const char *part = va_arg(*list, const char *);
   const char **text = va_arg(*list, const char **);
   *text = NULL;

   if (!it) return;

   *text = elm_object_item_part_text_get(it, part);
}

/* we have to keep a "manual" set here because of the callbacks on the
 * children */
static void
_elm_naviframe_smart_content_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Object_Item *it;

   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   it = elm_naviframe_top_item_get(obj);
   if (!it) return;

   elm_object_item_part_content_set(it, part, content);

   if(content == elm_object_item_part_content_get(it, part))
      if (ret) *ret = EINA_TRUE;
}

static void
_elm_naviframe_smart_content_get(Eo *obj, void *_pd EINA_UNUSED,  va_list *list)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);

   const char *part = va_arg(*list, const char *);
   Evas_Object **content = va_arg(*list, Evas_Object **);
   *content = NULL;

   if (!it) return;

   *content = elm_object_item_part_content_get(it, part);
}

static void
_elm_naviframe_smart_content_unset(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);

   const char *part = va_arg(*list, const char *);
   Evas_Object **content = va_arg(*list, Evas_Object **);
   *content = NULL;
   if (!it) return;

   *content = elm_object_item_part_content_unset(it, part);
}

static void
_on_item_title_clicked(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   evas_object_smart_callback_call(WIDGET(it), SIG_TITLE_CLICKED, it);
}

/* "elm,state,cur,pushed"
 */
static void
_on_item_push_finished(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   if (!it) return;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   evas_object_hide(VIEW(it));

   elm_widget_tree_unfocusable_set(VIEW(it), it->unfocusable);

   if (sd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);
}

/* "elm,state,cur,popped"
 */
static void
_on_item_pop_finished(void *data,
                      Evas_Object *obj __UNUSED__,
                      const char *emission __UNUSED__,
                      const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   if (sd->preserve)
     elm_widget_tree_unfocusable_set(VIEW(it), it->unfocusable);
   sd->popping = eina_list_remove(sd->popping, it);

   elm_widget_item_del(data);
}

/* "elm,state,new,pushed",
 * "elm,state,prev,popped
 */
static void
_on_item_show_finished(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   elm_object_signal_emit(VIEW(it), "elm,state,visible", "elm");

   elm_widget_tree_unfocusable_set(VIEW(it), it->unfocusable);

   _prev_page_focus_recover(it);

   if (sd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);

   evas_object_smart_callback_call(WIDGET(it), SIG_TRANSITION_FINISHED, data);
}

static void
_on_item_size_hints_changed(void *data,
                            Evas *e __UNUSED__,
                            Evas_Object *obj __UNUSED__,
                            void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_item_dispmode_set(Elm_Naviframe_Item *it, Evas_Display_Mode dispmode)
{
   if (it->dispmode == dispmode) return;
   switch (dispmode)
     {
      case EVAS_DISPLAY_MODE_COMPRESS:
         elm_object_signal_emit(VIEW(it), "elm,state,display,compress", "elm");
         break;
      default:
         elm_object_signal_emit(VIEW(it), "elm,state,display,default", "elm");
         break;
     }
   it->dispmode = dispmode;
}

static char *
_access_prev_btn_info_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
   return strdup(E_("Back"));
}

static Elm_Naviframe_Item *
_item_new(Evas_Object *obj,
          const Elm_Naviframe_Item *prev_it,
          const char *title_label,
          Evas_Object *prev_btn,
          Evas_Object *next_btn,
          Evas_Object *content,
          const char *item_style)
{
   Elm_Naviframe_Item *it;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   it = elm_widget_item_new(obj, Elm_Naviframe_Item);
   if (!it)
     {
        ERR("Failed to allocate new item! : naviframe=%p", obj);
        return NULL;
     }

   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_content_unset_hook_set(it, _item_content_unset_hook);
   elm_widget_item_signal_emit_hook_set(it, _item_signal_emit_hook);

   //item base layout
   VIEW(it) = elm_layout_add(obj);
   evas_object_smart_member_add(VIEW(it), obj);

   if (!elm_widget_sub_object_add(obj, VIEW(it)))
     ERR("could not add %p as sub object of %p", VIEW(it), obj);

   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_item_size_hints_changed, obj);

   elm_object_signal_callback_add
     (VIEW(it), "elm,action,show,finished", "*", _on_item_show_finished, it);
   elm_object_signal_callback_add
     (VIEW(it), "elm,action,pushed,finished", "*", _on_item_push_finished, it);
   elm_object_signal_callback_add
     (VIEW(it), "elm,action,popped,finished", "*", _on_item_pop_finished, it);
   elm_object_signal_callback_add
     (VIEW(it), "elm,action,title,clicked", "*", _on_item_title_clicked, it);

   _item_style_set(it, item_style);

   if (title_label)
     _item_text_set_hook((Elm_Object_Item *)it, TITLE_PART, title_label);

   //title buttons
   if ((!prev_btn) && sd->auto_pushed && prev_it)
     {
        const char *prev_title = prev_it->title_label;
        prev_btn = _back_btn_new(obj, prev_title);
     }

   if (prev_btn)
     {
        _item_content_set_hook((Elm_Object_Item *)it, PREV_BTN_PART, prev_btn);

        if (!elm_layout_text_get(prev_btn, NULL))
          _elm_access_callback_set
            (_elm_access_info_get(prev_btn), ELM_ACCESS_INFO,
             _access_prev_btn_info_cb, it);
     }

   if (next_btn)
     {
        _item_content_set_hook((Elm_Object_Item *)it, NEXT_BTN_PART, next_btn);

        if (!elm_layout_text_get(next_btn, NULL))
          _elm_access_text_set
            (_elm_access_info_get(next_btn), ELM_ACCESS_INFO, E_("Next"));
     }

   _item_content_set(it, content);
   _item_dispmode_set(it, sd->dispmode);

   it->title_visible = EINA_TRUE;

   return it;
}

static void
_on_obj_size_hints_changed(void *data __UNUSED__, Evas *e __UNUSED__,
                           Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it;
   Evas_Display_Mode dispmode;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   dispmode = evas_object_size_hint_display_mode_get(obj);
   if (sd->dispmode == dispmode) return;

   sd->dispmode = dispmode;

   EINA_INLIST_FOREACH(sd->stack, it)
     _item_dispmode_set(it, dispmode);
}


static void
_elm_naviframe_smart_focus_next(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *ao;

   Eina_List *l = NULL;
   Elm_Naviframe_Item *top_it;
   void *(*list_data_get)(const Eina_List *list);

   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   top_it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   if (!top_it) goto end;

   list_data_get = eina_list_data_get;

   l = eina_list_append(l, VIEW(top_it));

   /* access */
   if (_elm_config->access_mode)
     {
        ao = _access_object_get(top_it, TITLE_ACCESS_PART);
        if (ao) l = eina_list_append(l, ao);
     }

   int_ret = elm_widget_focus_list_next_get(obj, l, list_data_get, dir, next);
   eina_list_free(l);
   if (ret) *ret = int_ret;

end:
   if (ret && !*ret)
     {
        *next = obj;
        *ret = !elm_widget_focus_get(obj);
     }
}

static void
_elm_naviframe_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_naviframe_smart_focus_direction(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *base = va_arg(*list, Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   Eina_List *l = NULL;
   Elm_Naviframe_Item *top_it;
   void *(*list_data_get)(const Eina_List *list);

   top_it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   if (!top_it) return;

   list_data_get = eina_list_data_get;

   l = eina_list_append(l, VIEW(top_it));

   int_ret = elm_widget_focus_list_direction_get
            (obj, base, l, list_data_get, degree, direction, weight);

   if (ret) *ret = int_ret;
   eina_list_free(l);
}

static void
_elm_naviframe_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Naviframe_Smart_Data *priv = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->dummy_edje = wd->resize_obj;
   evas_object_smart_member_add(priv->dummy_edje, obj);

   priv->auto_pushed = EINA_TRUE;
   priv->freeze_events = EINA_TRUE;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _on_obj_size_hints_changed, obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static Eina_Bool
_pop_transition_cb(void *data)
{
   Elm_Naviframe_Item *prev_it, *it;
   it = (Elm_Naviframe_Item *)data;

   it->animator = NULL;

   prev_it = (Elm_Naviframe_Item *) elm_naviframe_top_item_get(WIDGET(it));
   if (prev_it)
     {
        elm_object_signal_emit(VIEW(prev_it), "elm,state,prev,popped,deferred",
                               "elm");
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
     }
   elm_object_signal_emit(VIEW(it), "elm,state,cur,popped,deferred", "elm");
   edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));

   return ECORE_CALLBACK_CANCEL;
}

static void
_elm_naviframe_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Naviframe_Item *it;

   Elm_Naviframe_Smart_Data *sd = _pd;

   sd->on_deletion = EINA_TRUE;

   while (sd->stack)
     {
        it = EINA_INLIST_CONTAINER_GET(sd->stack, Elm_Naviframe_Item);
        elm_widget_item_del(it);
     }

   //All popping items which are not called yet by animator.
   EINA_LIST_FREE(sd->popping, it)
     {
        if (it->animator) ecore_animator_del(it->animator);
        elm_widget_item_del(it);
     }

   sd->on_deletion = EINA_FALSE;
   evas_object_del(sd->dummy_edje);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static void
_elm_naviframe_smart_event(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   Evas_Event_Key_Down *ev = va_arg(*list, Evas_Event_Key_Down *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Naviframe_Item *it;

   if (ret) *ret = EINA_FALSE;
   (void)src;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (strcmp(ev->key, "BackSpace")) return;

   eo_do(obj, elm_obj_naviframe_top_item_get((Elm_Object_Item **)&it));
   if (!it) return;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;

   //FIXME: Replace this below code to elm_naviframe_item_pop() at elm 2.0.
   ///Leave for compatibility.
   if (it->title_prev_btn)
     evas_object_smart_callback_call(it->title_prev_btn, SIG_CLICKED, NULL);
}

static void
_elm_naviframe_smart_access(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Naviframe_Smart_Data *sd = _pd;
   Elm_Naviframe_Item *it;

   Eina_Bool is_access = va_arg(*list, int);
   EINA_INLIST_FOREACH(sd->stack, it)
     _access_obj_process(it, is_access);
}

static Eina_Bool
_push_transition_cb(void *data)
{
   Elm_Naviframe_Item *prev_it, *it = data;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   it->animator = NULL;

   if (sd->stack->last->prev)
     {
        prev_it = EINA_INLIST_CONTAINER_GET(sd->stack->last->prev,
                                            Elm_Naviframe_Item);
        elm_object_signal_emit(VIEW(prev_it), "elm,state,cur,pushed,deferred",
                                "elm");
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
     }
   elm_object_signal_emit(VIEW(it), "elm,state,new,pushed,deferred", "elm");
   edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));

   return ECORE_CALLBACK_CANCEL;
}

EAPI Evas_Object *
elm_naviframe_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI Elm_Object_Item *
elm_naviframe_item_push(Evas_Object *obj,
                        const char *title_label,
                        Evas_Object *prev_btn,
                        Evas_Object *next_btn,
                        Evas_Object *content,
                        const char *item_style)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_naviframe_item_push(title_label, prev_btn, next_btn, content, item_style, &ret));
   return ret;
}

static void
_item_push(Eo *obj, void *_pd, va_list *list)
{
   Elm_Naviframe_Item *prev_it, *it;

   Elm_Naviframe_Smart_Data *sd = _pd;

   const char *title_label = va_arg(*list, const char *);
   Evas_Object *prev_btn = va_arg(*list, Evas_Object *);
   Evas_Object *next_btn = va_arg(*list, Evas_Object *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   const char *item_style = va_arg(*list, const char *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   prev_it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   it = _item_new(obj, prev_it,
                  title_label, prev_btn, next_btn, content, item_style);
   if (!it) return;

   evas_object_show(VIEW(it));

   if (prev_it) elm_widget_focused_object_clear(VIEW(prev_it));
   _resize_object_reset(obj, it);
   if (prev_it)
     {
        if (sd->freeze_events)
          {
             evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
             evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
          }

        elm_object_signal_emit(VIEW(prev_it), "elm,state,cur,pushed", "elm");
        elm_object_signal_emit(VIEW(it), "elm,state,new,pushed", "elm");
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));

        prev_it->unfocusable = elm_widget_tree_unfocusable_get(VIEW(prev_it));
        elm_widget_tree_unfocusable_set(VIEW(prev_it), EINA_TRUE);

        if (it->animator) ecore_animator_del(it->animator);
        it->animator = ecore_animator_add(_push_transition_cb, it);
     }
   else
     {
        if (elm_object_focus_allow_get(VIEW(it)))
          elm_object_focus_set(VIEW(it), EINA_TRUE);
        else
          elm_object_focus_set(WIDGET(it), EINA_TRUE);
     }

   sd->stack = eina_inlist_append(sd->stack, EINA_INLIST_GET(it));

   elm_layout_sizing_eval(obj);

   *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_naviframe_item_insert_before(Evas_Object *obj,
                                 Elm_Object_Item *before,
                                 const char *title_label,
                                 Evas_Object *prev_btn,
                                 Evas_Object *next_btn,
                                 Evas_Object *content,
                                 const char *item_style)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_naviframe_item_insert_before(before, title_label, prev_btn, next_btn, content, item_style, &ret));
   return ret;
}

static void
_item_insert_before(Eo *obj, void *_pd, va_list *list)
{
   Elm_Naviframe_Item *it, *prev_it = NULL;

   Elm_Object_Item *before = va_arg(*list, Elm_Object_Item *);
   const char *title_label = va_arg(*list, const char *);
   Evas_Object *prev_btn = va_arg(*list, Evas_Object *);
   Evas_Object *next_btn = va_arg(*list, Evas_Object *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   const char *item_style = va_arg(*list, const char *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   ELM_NAVIFRAME_ITEM_CHECK(before);
   Elm_Naviframe_Smart_Data *sd = _pd;

   it = (Elm_Naviframe_Item *)before;
   if (EINA_INLIST_GET(it)->prev)
     prev_it = EINA_INLIST_CONTAINER_GET(EINA_INLIST_GET(it)->prev,
                                         Elm_Naviframe_Item);
   it = _item_new(obj, prev_it,
                  title_label, prev_btn, next_btn, content, item_style);
   if (!it) return;

   sd->stack = eina_inlist_prepend_relative
       (sd->stack, EINA_INLIST_GET(it),
       EINA_INLIST_GET(((Elm_Naviframe_Item *)before)));

   elm_layout_sizing_eval(obj);

   *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_naviframe_item_insert_after(Evas_Object *obj,
                                Elm_Object_Item *after,
                                const char *title_label,
                                Evas_Object *prev_btn,
                                Evas_Object *next_btn,
                                Evas_Object *content,
                                const char *item_style)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_naviframe_item_insert_after(after, title_label, prev_btn, next_btn, content, item_style, &ret));
   return ret;
}

static void
_item_insert_after(Eo *obj, void *_pd, va_list *list)
{
   Elm_Naviframe_Item *it;
   Eina_Bool top_inserted = EINA_FALSE;

   Elm_Object_Item *after = va_arg(*list, Elm_Object_Item *);
   const char *title_label = va_arg(*list, const char *);
   Evas_Object *prev_btn = va_arg(*list, Evas_Object *);
   Evas_Object *next_btn = va_arg(*list, Evas_Object *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   const char *item_style = va_arg(*list, const char *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   ELM_NAVIFRAME_ITEM_CHECK(after);
   Elm_Naviframe_Smart_Data *sd = _pd;

   it = _item_new(obj, (Elm_Naviframe_Item *)after,
                  title_label, prev_btn, next_btn, content, item_style);
   if (!it) return;

   if (elm_naviframe_top_item_get(obj) == after) top_inserted = EINA_TRUE;

   sd->stack = eina_inlist_append_relative
       (sd->stack, EINA_INLIST_GET(it),
       EINA_INLIST_GET(((Elm_Naviframe_Item *)after)));

   if (top_inserted)
     {
        elm_widget_focused_object_clear(VIEW(after));
        _resize_object_reset(obj, it);
        evas_object_show(VIEW(it));
        evas_object_hide(VIEW(after));
        if (elm_object_focus_allow_get(VIEW(it)))
          elm_object_focus_set(VIEW(it), EINA_TRUE);
        else
          elm_object_focus_set(WIDGET(it), EINA_TRUE);
     }

   elm_layout_sizing_eval(obj);

   *ret = (Elm_Object_Item *)it;
}

EAPI Evas_Object *
elm_naviframe_item_pop(Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do(obj, elm_obj_naviframe_item_pop(&ret));
   return ret;
}

static void
_item_pop(Eo *obj, void *_pd, va_list *list)
{
   Elm_Naviframe_Item *it, *prev_it = NULL;
   Evas_Object *content = NULL;

   Evas_Object **ret = va_arg(*list, Evas_Object **);
   *ret = NULL;

   Elm_Naviframe_Smart_Data *sd = _pd;

   if (sd->freeze_events && sd->popping) return;

   it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   if (!it) return;

   if (it->popping) return;
   it->popping = EINA_TRUE;

   if (it->pop_cb)
     {
        if (!it->pop_cb(it->pop_data, (Elm_Object_Item *)it))
          {
             it->popping = EINA_FALSE;
             return;
          }
     }

   if (sd->preserve)
     content = it->content;

   evas_object_data_set(VIEW(it), "out_of_list", (void *)1);

   it->unfocusable = elm_widget_tree_unfocusable_get(VIEW(it));
   elm_widget_tree_unfocusable_set(VIEW(it), EINA_TRUE);

   if (sd->stack->last->prev)
     prev_it = EINA_INLIST_CONTAINER_GET
         (sd->stack->last->prev, Elm_Naviframe_Item);

   sd->stack = eina_inlist_remove(sd->stack, EINA_INLIST_GET(it));

   if (prev_it)
     {
        if (sd->freeze_events)
          {
             evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
             evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
          }

        _resize_object_reset(obj, prev_it);

        /* these 2 signals MUST take place simultaneously */
        elm_object_signal_emit(VIEW(it), "elm,state,cur,popped", "elm");
        evas_object_show(VIEW(prev_it));
        elm_object_signal_emit(VIEW(prev_it), "elm,state,prev,popped", "elm");

        edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));

        if (it->animator) ecore_animator_del(it->animator);
        it->animator = ecore_animator_add(_pop_transition_cb, it);
        sd->popping = eina_list_append(sd->popping, it);
     }
   else
     elm_widget_item_del(it);

   *ret = content;
}

EAPI void
elm_naviframe_item_pop_to(Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit;
   Eina_Inlist *l, *prev_l;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   nit = (Elm_Naviframe_Item *)it;
   ELM_NAVIFRAME_DATA_GET(WIDGET(nit), sd);

   if (it == elm_naviframe_top_item_get(WIDGET(nit))) return;

   l = sd->stack->last->prev;

   sd->on_deletion = EINA_TRUE;

   while (l)
     {
        Elm_Naviframe_Item *iit = EINA_INLIST_CONTAINER_GET
            (l, Elm_Naviframe_Item);

        if (iit == nit) break;

        prev_l = l->prev;
        sd->stack = eina_inlist_remove(sd->stack, l);

        elm_widget_item_del(iit);

        l = prev_l;
     }

   sd->on_deletion = EINA_FALSE;

   elm_naviframe_item_pop(WIDGET(nit));
}

EAPI void
elm_naviframe_item_promote(Elm_Object_Item *it)
{
   Elm_Object_Item *prev_top;
   Elm_Naviframe_Item *nit;
   Elm_Naviframe_Item *prev_it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   nit = (Elm_Naviframe_Item *)it;
   ELM_NAVIFRAME_DATA_GET(WIDGET(nit), sd);

   prev_top = elm_naviframe_top_item_get(nit->base.widget);
   if (it == prev_top) return;

   /* remember, last is 1st on the naviframe, push it to last pos. */
   sd->stack = eina_inlist_demote(sd->stack, EINA_INLIST_GET(nit));

   /* this was the previous top one */
   prev_it = EINA_INLIST_CONTAINER_GET
       (sd->stack->last->prev, Elm_Naviframe_Item);

   elm_widget_focused_object_clear(VIEW(nit));
   _resize_object_reset(WIDGET(it), nit);

   if (sd->freeze_events)
     {
        evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
        evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
     }

   elm_object_signal_emit(VIEW(prev_it), "elm,state,cur,pushed", "elm");

   evas_object_show(VIEW(nit));

   elm_object_signal_emit(VIEW(nit), "elm,state,new,pushed", "elm");

   edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
   edje_object_message_signal_process(elm_layout_edje_get(VIEW(nit)));
   if (nit->animator) ecore_animator_del(nit->animator);
   nit->animator = ecore_animator_add(_push_transition_cb, nit);
}

EAPI void
elm_naviframe_item_simple_promote(Evas_Object *obj,
                                  Evas_Object *content)
{
   ELM_NAVIFRAME_CHECK(obj);
   eo_do(obj, elm_obj_naviframe_item_simple_promote(content));
}

static void
_item_simple_promote(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Naviframe_Item *itr;

   Evas_Object *content = va_arg(*list, Evas_Object *);

   Elm_Naviframe_Smart_Data *sd = _pd;

   EINA_INLIST_FOREACH(sd->stack, itr)
     {
        if (elm_object_item_content_get((Elm_Object_Item *)itr) == content)
          {
             elm_naviframe_item_promote((Elm_Object_Item *)itr);
             break;
          }
     }
}

EAPI void
elm_naviframe_content_preserve_on_pop_set(Evas_Object *obj,
                                          Eina_Bool preserve)
{
   ELM_NAVIFRAME_CHECK(obj);
   eo_do(obj, elm_obj_naviframe_content_preserve_on_pop_set(preserve));
}

static void
_content_preserve_on_pop_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool preserve = va_arg(*list, int);
   Elm_Naviframe_Smart_Data *sd = _pd;

   sd->preserve = !!preserve;
}

EAPI Eina_Bool
elm_naviframe_content_preserve_on_pop_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_naviframe_content_preserve_on_pop_get(&ret));
   return ret;
}

static void
_content_preserve_on_pop_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Naviframe_Smart_Data *sd = _pd;

   *ret = sd->preserve;
}

EAPI Elm_Object_Item *
elm_naviframe_top_item_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_naviframe_top_item_get(&ret));
   return ret;
}

static void
_top_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;
   Elm_Naviframe_Smart_Data *sd = _pd;

   if (!sd->stack) return;
   *ret = (Elm_Object_Item *)(EINA_INLIST_CONTAINER_GET
                                (sd->stack->last, Elm_Naviframe_Item));
}

EAPI Elm_Object_Item *
elm_naviframe_bottom_item_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_naviframe_bottom_item_get(&ret));
   return ret;
}

static void
_bottom_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;
   Elm_Naviframe_Smart_Data *sd = _pd;

   if (!sd->stack) return;
   *ret = (Elm_Object_Item *)(EINA_INLIST_CONTAINER_GET
                                (sd->stack, Elm_Naviframe_Item));
}

EAPI void
elm_naviframe_item_style_set(Elm_Object_Item *it,
                             const char *item_style)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   if (item_style && !strcmp(item_style, nit->style)) return;

   if (!item_style)
     if (!strcmp("basic", nit->style)) return;

   _item_style_set(nit, item_style);
   _item_signals_emit(nit);
   _item_title_visible_update(nit);
}

EAPI const char *
elm_naviframe_item_style_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it, NULL);

   return nit->style;
}

EAPI void
elm_naviframe_item_title_visible_set(Elm_Object_Item *it,
                                     Eina_Bool visible)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   visible = !!visible;
   if (nit->title_visible == visible) return;

   nit->title_visible = visible;
   _item_title_visible_update(nit);
}

EAPI Eina_Bool
elm_naviframe_item_title_visible_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return nit->title_visible;
}

EAPI void
elm_naviframe_item_pop_cb_set(Elm_Object_Item *it, Elm_Naviframe_Item_Pop_Cb func, void *data)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   nit->pop_cb = func;
   nit->pop_data = data;
}

EAPI void
elm_naviframe_prev_btn_auto_pushed_set(Evas_Object *obj,
                                       Eina_Bool auto_pushed)
{
   ELM_NAVIFRAME_CHECK(obj);
   eo_do(obj, elm_obj_naviframe_prev_btn_auto_pushed_set(auto_pushed));
}

static void
_prev_btn_auto_pushed_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool auto_pushed = va_arg(*list, int);
   Elm_Naviframe_Smart_Data *sd = _pd;

   sd->auto_pushed = !!auto_pushed;
}

EAPI Eina_Bool
elm_naviframe_prev_btn_auto_pushed_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_naviframe_prev_btn_auto_pushed_get(&ret));
   return ret;
}

static void
_prev_btn_auto_pushed_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Naviframe_Smart_Data *sd = _pd;

   *ret = sd->auto_pushed;
}

EAPI Eina_List *
elm_naviframe_items_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   Eina_List *ret;
   eo_do((Eo *) obj, elm_obj_naviframe_items_get(&ret));
   return ret;
}

static void
_items_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_List **ret = va_arg(*list, Eina_List **);
   *ret = NULL;
   Elm_Naviframe_Item *itr;

   Elm_Naviframe_Smart_Data *sd = _pd;

   EINA_INLIST_FOREACH(sd->stack, itr)
     *ret = eina_list_append(*ret, itr);
}

EAPI void
elm_naviframe_event_enabled_set(Evas_Object *obj,
                                Eina_Bool enabled)
{
   ELM_NAVIFRAME_CHECK(obj);
   eo_do(obj, elm_obj_naviframe_event_enabled_set(enabled));
}

static void
_event_enabled_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool enabled = va_arg(*list, int);
   Elm_Naviframe_Smart_Data *sd = _pd;

   enabled = !!enabled;
   if (sd->freeze_events == !enabled) return;
   sd->freeze_events = !enabled;
}

EAPI Eina_Bool
elm_naviframe_event_enabled_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_naviframe_event_enabled_get(&ret));
   return ret;
}

static void
_event_enabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Naviframe_Smart_Data *sd = _pd;

   *ret = !sd->freeze_events;
}

static void
_elm_naviframe_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_naviframe_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_naviframe_smart_del),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_naviframe_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT), _elm_naviframe_smart_focus_next),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_naviframe_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION), _elm_naviframe_smart_focus_direction),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME), _elm_naviframe_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TRANSLATE), _elm_naviframe_smart_translate),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACCESS), _elm_naviframe_smart_access),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT), _elm_naviframe_smart_event),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_naviframe_smart_content_set),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET), _elm_naviframe_smart_content_get),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), _elm_naviframe_smart_content_unset),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_EMIT), _elm_naviframe_smart_signal_emit),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET), _elm_naviframe_smart_text_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_GET), _elm_naviframe_smart_text_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_naviframe_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_PUSH), _item_push),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_INSERT_BEFORE), _item_insert_before),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_INSERT_AFTER), _item_insert_after),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_POP), _item_pop),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_SIMPLE_PROMOTE), _item_simple_promote),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_CONTENT_PRESERVE_ON_POP_SET), _content_preserve_on_pop_set),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_CONTENT_PRESERVE_ON_POP_GET), _content_preserve_on_pop_get),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_TOP_ITEM_GET), _top_item_get),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_BOTTOM_ITEM_GET), _bottom_item_get),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_PREV_BTN_AUTO_PUSHED_SET), _prev_btn_auto_pushed_set),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_PREV_BTN_AUTO_PUSHED_GET), _prev_btn_auto_pushed_get),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEMS_GET), _items_get),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_EVENT_ENABLED_SET), _event_enabled_set),
        EO_OP_FUNC(ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_EVENT_ENABLED_GET), _event_enabled_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const
Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_PUSH, "Push a new item to the top of the naviframe stack (and show it)."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_INSERT_BEFORE, "Insert a new item into the naviframe before item before."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_INSERT_AFTER, "Insert a new item into the naviframe after item after."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_POP, "Pop an item that is on top of the stack."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_SIMPLE_PROMOTE, "Simple version of item_promote."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_CONTENT_PRESERVE_ON_POP_SET, "preserve the content objects when items are popped."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_CONTENT_PRESERVE_ON_POP_GET, "Get a value whether preserve mode is enabled or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_TOP_ITEM_GET, "Get a top item on the naviframe stack."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_BOTTOM_ITEM_GET, "Get a bottom item on the naviframe stack."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_PREV_BTN_AUTO_PUSHED_SET, "Set creating prev button automatically or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_PREV_BTN_AUTO_PUSHED_GET, "Get a value whether prev button(back button) will be auto pushed or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_ITEMS_GET, "Get a list of all the naviframe items."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_EVENT_ENABLED_SET, "Set the event enabled when pushing/popping items."),
     EO_OP_DESCRIPTION(ELM_OBJ_NAVIFRAME_SUB_ID_EVENT_ENABLED_GET, "Get the value of event enabled status."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_NAVIFRAME_BASE_ID, op_desc, ELM_OBJ_NAVIFRAME_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Naviframe_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_naviframe_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
