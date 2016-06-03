#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACTION_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

extern Eina_Hash *_elm_key_bindings;

//Tizen only(20160524):apply callbacks on direct reading stop/cancel
static const char SIG_READ_STOP[] = "access,read,stop";
static const char SIG_READ_CANCEL[] = "access,read,cancel";

EOLIAN static Eina_List*
_elm_interface_atspi_widget_action_elm_interface_atspi_action_actions_get(Eo *obj, void *pd EINA_UNUSED);

EOLIAN static const char *
_elm_interface_atspi_widget_action_elm_interface_atspi_action_name_get(Eo *obj, void *pd EINA_UNUSED, int id);
//

EOLIAN static Eina_Bool
_elm_interface_atspi_widget_action_elm_interface_atspi_action_action_do(Eo *obj, void *pd EINA_UNUSED, int id)
{
   const Elm_Atspi_Action *actions = NULL;
   const char *param;
   Eina_Bool (*func)(Eo *eo, const char *params) = NULL;
   int tmp = 0;

   //Tizen only(20160524):apply callbacks on direct reading stop/cancel
   const char *action_name;
   action_name = _elm_interface_atspi_widget_action_elm_interface_atspi_action_name_get(obj, NULL, id);

   if (!strcmp("ReadingStopped", action_name))
     {
        Eina_Bool ret;
        eo_do(obj, ret = elm_interface_atspi_widget_action_on_reading_stopped());
        return ret;
     }
   else if (!strcmp("ReadingCancelled", action_name))
     {
        Eina_Bool ret;
        eo_do(obj, ret = elm_interface_atspi_widget_action_on_reading_cancelled());
        return ret;
     }
   else
     {
        eo_do(obj, actions = elm_interface_atspi_widget_action_elm_actions_get());

        if (!actions) return EINA_FALSE;

        while (actions[tmp].name)
          {
             if (tmp == id)
               {
                  func = actions[tmp].func;
                  param = actions[tmp].param;
                  break;
               }
             tmp++;
         }
     }
   //
   if (!func)
     return EINA_FALSE;

   return func(obj, param);
}

EOLIAN static char*
_elm_interface_atspi_widget_action_elm_interface_atspi_action_keybinding_get(Eo *obj, void *pd EINA_UNUSED, int id)
{
   const Elm_Atspi_Action *actions = NULL;
   Eina_List *l1, *binding_list;
   const char *action = NULL, *param = NULL;
   Elm_Config_Binding_Key *binding;
   int tmp = 0;

   if (!eo_isa(obj, ELM_WIDGET_CLASS))
      return NULL;

   eo_do(obj, actions = elm_interface_atspi_widget_action_elm_actions_get());
   if (!actions) return NULL;

   while (actions[tmp].name)
     {
        if (tmp == id)
          {
             action = actions[tmp].action;
             param = actions[tmp].param;
             break;
          }
        tmp++;
     }
   if (!action) return NULL;

   binding_list = eina_hash_find(_elm_key_bindings, elm_widget_type_get(obj));

   if (binding_list)
     {
        EINA_LIST_FOREACH(binding_list, l1, binding)
          {
             if (!strcmp(binding->action, action) && (!param ||
                 !strcmp(binding->params, param)))
               {
                  Eina_List *l2;
                  Elm_Config_Binding_Modifier *bm;
                  char *ret;
                  Eina_Strbuf *buf = eina_strbuf_new();
                  eina_strbuf_append_printf(buf, "%s", binding->key);
                  EINA_LIST_FOREACH(binding->modifiers, l2, bm)
                    if (bm->flag) eina_strbuf_append_printf(buf, "+%s", bm->mod);
                  ret = eina_strbuf_string_steal(buf);
                  eina_strbuf_free(buf);
                  return ret;
               }
          }
     }

   return NULL;
}

EOLIAN static const char *
_elm_interface_atspi_widget_action_elm_interface_atspi_action_name_get(Eo *obj, void *pd EINA_UNUSED, int id)
{
   //Tizen only(20160524):apply callbacks on direct reading stop/cancel
   Eina_List *action_names = _elm_interface_atspi_widget_action_elm_interface_atspi_action_actions_get(obj, NULL);
   return eina_list_nth(action_names, id);
   //
}

EOLIAN static Eina_Bool
_elm_interface_atspi_widget_action_elm_interface_atspi_action_description_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, int id EINA_UNUSED, const char *description EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static const char *
_elm_interface_atspi_widget_action_elm_interface_atspi_action_description_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, int id EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Eina_List*
_elm_interface_atspi_widget_action_elm_interface_atspi_action_actions_get(Eo *obj, void *pd EINA_UNUSED)
{
   const Elm_Atspi_Action *actions = NULL;
   Eina_List *ret = NULL;
   int tmp = 0;

   eo_do(obj, actions = elm_interface_atspi_widget_action_elm_actions_get());

   //Tizen only(20160524):apply callbacks on direct reading stop/cancel
   while (actions && actions[tmp].name)
     {
        ret = eina_list_append(ret, actions[tmp].name);
        tmp++;
     }

   ret = eina_list_append(ret, "ReadingStopped");
   ret = eina_list_append(ret, "ReadingCancelled");
   //

   return ret;
}

#include "elm_interface_atspi_widget_action.eo.c"
