// TIZEN ONLY header file
#ifndef ELM_MODULE_PRIV_H
#define ELM_MODULE_PRIV_H

typedef struct _Elm_Entry_Extension_data Elm_Entry_Extension_data;
typedef void (*cpfunc)(void *data, Evas_Object *obj, void *event_info);

struct _Elm_Entry_Extension_data
{
   Evas_Object *popup;
   Evas_Object *ent;
   Evas_Object *caller;
#ifdef HAVE_ELEMENTARY_WAYLAND
   Evas_Object *cbhm_caller; //FIXME: remove when focus issue is resolved
   Ecore_Job *cbhm_init_job;
#endif
   Eina_Rectangle *viewport_rect;
   Evas_Coord_Rectangle selection_rect;
   Eina_List *items;
   cpfunc select;
   cpfunc copy;
   cpfunc cut;
   cpfunc paste;
   cpfunc cancel;
   cpfunc selectall;
   cpfunc cnpinit;
   cpfunc keep_selection;
   cpfunc paste_translation;
   cpfunc is_selected_all;
   Elm_Config *_elm_config;
   Eina_Bool password : 1;
   Eina_Bool editable : 1;
   Eina_Bool have_selection: 1;
   Eina_Bool selmode : 1;
   Eina_Bool context_menu : 1;
   Elm_Cnp_Mode cnp_mode : 2;
   Eina_Bool popup_showing : 1;
   Eina_Bool mouse_up : 1;
   Eina_Bool mouse_move : 1;
   Eina_Bool mouse_down : 1;
   Eina_Bool entry_move : 1;
   Eina_Bool popup_clicked : 1;
   Eina_Bool cursor_handler_shown : 1;
   Eina_Bool ent_scroll : 1;
#ifdef HAVE_ELEMENTARY_WAYLAND
   Eina_Bool cbhm_init_done : 1;
#endif
   Evas_Object *ctx_par;
   Evas_Object *start_handler;
   Evas_Object *end_handler;
   Evas_Object *cursor_handler;
   Ecore_Timer *show_timer;
   char *source_text;
   char *target_text;
};

void elm_entry_extension_module_data_get(Evas_Object *obj,Elm_Entry_Extension_data *ext_mod);

#endif
