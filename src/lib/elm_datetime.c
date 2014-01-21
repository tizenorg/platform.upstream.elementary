#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_datetime.h"

EAPI Eo_Op ELM_OBJ_DATETIME_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_DATETIME_CLASS

#define MY_CLASS_NAME "Elm_Datetime"
#define MY_CLASS_NAME_LEGACY "elm_datetime"

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#define MAX_SEPARATOR_LEN              6
#define MIN_DAYS_IN_MONTH              28
#define BUFFER_SIZE                    1024

/* interface between EDC & C code (field & signal names). values 0 to
 * ELM_DATETIME_TYPE_COUNT are in the valid range, and must get in the
 * place of "%d".
 */
#define EDC_DATETIME_FOCUSIN_SIG_STR   "elm,action,focus"
#define EDC_DATETIME_FOCUSOUT_SIG_STR  "elm,action,unfocus"
#define EDC_PART_FIELD_STR             "field%d"
#define EDC_PART_SEPARATOR_STR         "separator%d"
#define EDC_PART_FIELD_ENABLE_SIG_STR  "field%d,enable"
#define EDC_PART_FIELD_DISABLE_SIG_STR "field%d,disable"

/* struct tm does not define the fields in the order year, month,
 * date, hour, minute. values are reassigned to an array for easy
 * handling.
 */
#define DATETIME_TM_ARRAY(intptr, tmptr) \
  int *intptr[] = {                      \
     &(tmptr)->tm_year,                  \
     &(tmptr)->tm_mon,                   \
     &(tmptr)->tm_mday,                  \
     &(tmptr)->tm_hour,                  \
     &(tmptr)->tm_min}

// default limits for individual fields
static Format_Map mapping[ELM_DATETIME_TYPE_COUNT] = {
   [ELM_DATETIME_YEAR] = { "Yy", -1, -1, "" },
   [ELM_DATETIME_MONTH] = { "mbBh", 0, 11, "" },
   [ELM_DATETIME_DATE] = { "de", 1, 31, "" },
   [ELM_DATETIME_HOUR] = { "IHkl", 0, 23, "" },
   [ELM_DATETIME_MINUTE] = { "M", 0, 59, ":" },
   [ELM_DATETIME_AMPM] = { "pP", 0, 1, "" }
};

static const char *multifield_formats = "cxXrRTDF";
static const char *ignore_separators = "()";
static Datetime_Mod_Api *dt_mod = NULL;

static const char SIG_CHANGED[] = "changed";
static const char SIG_LANG_CHANGED[] = "language,changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_LANG_CHANGED, ""},
   {"focused", ""}, /**< handled by elm_widget */
   {"unfocused", ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static Datetime_Mod_Api *
_dt_mod_init()
{
   Elm_Module *mod = NULL;

   if (!(mod = _elm_module_find_as("datetime/api"))) return NULL;

   mod->api = malloc(sizeof(Datetime_Mod_Api));
   if (!mod->api) return NULL;

   ((Datetime_Mod_Api *)(mod->api))->obj_hook =
     _elm_module_symbol_get(mod, "obj_hook");
   ((Datetime_Mod_Api *)(mod->api))->obj_unhook =
     _elm_module_symbol_get(mod, "obj_unhook");
   ((Datetime_Mod_Api *)(mod->api))->obj_hide =
     _elm_module_symbol_get(mod, "obj_hide");
   ((Datetime_Mod_Api *)(mod->api))->field_create =
     _elm_module_symbol_get(mod, "field_create");
   ((Datetime_Mod_Api *)(mod->api))->field_value_display =
     _elm_module_symbol_get(mod, "field_value_display");

   return mod->api;
}

static void
_field_list_display(Evas_Object *obj)
{
   Datetime_Field *field;
   unsigned int idx = 0;

   ELM_DATETIME_DATA_GET(obj, sd);

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if (field->fmt_exist && field->visible)
          {
             if ((dt_mod) && (dt_mod->field_value_display))
               dt_mod->field_value_display(sd->mod_data, field->item_obj);
          }
     }
}

// FIXME: provide nl_langinfo on Windows if possible
// returns expanded format string for corresponding multi-field format character
static char *
_expanded_fmt_str_get(char ch)
{
   char *exp_fmt = "";
   switch (ch)
     {
      case 'c':
#ifdef HAVE_LANGINFO_H
        exp_fmt = nl_langinfo(D_T_FMT);
#else
        exp_fmt = "";
#endif
        break;

      case 'x':
#ifdef HAVE_LANGINFO_H
        exp_fmt = nl_langinfo(D_FMT);
#else
        exp_fmt = "";
#endif
        break;

      case 'X':
#ifdef HAVE_LANGINFO_H
        exp_fmt = nl_langinfo(T_FMT);
#else
        exp_fmt = "";
#endif
        break;

      case 'r':
#ifdef HAVE_LANGINFO_H
        exp_fmt = nl_langinfo(T_FMT_AMPM);
#else
        exp_fmt = "";
#endif
        break;

      case 'R':
        exp_fmt = "%H:%M";
        break;

      case 'T':
        exp_fmt = "%H:%M:%S";
        break;

      case 'D':
        exp_fmt = "%m/%d/%y";
        break;

      case 'F':
        exp_fmt = "%Y-%m-%d";
        break;

      default:
        exp_fmt = "";
        break;
     }

   return exp_fmt;
}

static void
_expand_format(char *dt_fmt)
{
   char *ptr, *expanded_fmt, ch;
   unsigned int idx = 0, len = 0;
   char buf[ELM_DATETIME_MAX_FORMAT_LEN] = {0, };
   Eina_Bool fmt_char = EINA_FALSE;

   ptr = dt_fmt;
   while ((ch = *ptr))
     {
        if ((fmt_char) && (strchr(multifield_formats, ch)))
          {
             /* replace the multi-field format characters with
              * corresponding expanded format */
             expanded_fmt = _expanded_fmt_str_get(ch);
             len = strlen(expanded_fmt);
             buf[--idx] = 0;
             strncat(buf, expanded_fmt, len);
             idx += len;
          }
        else buf[idx++] = ch;

        if (ch == '%') fmt_char = EINA_TRUE;
        else fmt_char = EINA_FALSE;

        ptr++;
     }

   buf[idx] = 0;
   strncpy(dt_fmt, buf, ELM_DATETIME_MAX_FORMAT_LEN);
}

static void
_field_list_arrange(Evas_Object *obj)
{
   Datetime_Field *field;
   char buf[BUFFER_SIZE];
   int idx;

   ELM_DATETIME_DATA_GET(obj, sd);

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        snprintf(buf, sizeof(buf), EDC_PART_FIELD_STR, field->location);

        if (field->visible && field->fmt_exist)
          {
             elm_layout_content_unset(obj, buf);
             elm_layout_content_set(obj, buf, field->item_obj);
          }
        else
          evas_object_hide(elm_layout_content_unset(obj, buf));
     }

   elm_layout_sizing_eval(obj);
   _field_list_display(obj);
}

static unsigned int
_parse_format(Evas_Object *obj,
              char *fmt_ptr)
{
   Eina_Bool fmt_parsing = EINA_FALSE, sep_parsing = EINA_FALSE,
             sep_lookup = EINA_FALSE;
   unsigned int len = 0, idx = 0, location = 0;
   char separator[MAX_SEPARATOR_LEN];
   Datetime_Field *field = NULL;
   char cur;

   ELM_DATETIME_DATA_GET(obj, sd);

   while ((cur = *fmt_ptr))
     {
        if (fmt_parsing)
          {
             fmt_parsing = EINA_FALSE;
             for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
               {
                  if (strchr(mapping[idx].fmt_char, cur))
                    {
                       field = sd->field_list + idx;
                       /* ignore the fields already have or disabled
                        * valid formats, means already parsed &
                        * repeated, ignore. */
                       if (field->location != -1) break;
                       field->fmt[1] = cur;
                       field->fmt_exist = EINA_TRUE;
                       field->location = location++;
                       sep_lookup = EINA_TRUE;
                       len = 0;
                       break;
                    }
               }
          }
        if (cur == '%')
          {
             fmt_parsing = EINA_TRUE;
             sep_parsing = EINA_FALSE;
             // set the separator to previous field
             separator[len] = 0;
             if (field) eina_stringshare_replace(&field->separator, separator);
          }
        // ignore the set of chars (global, field specific) as field separators
        if (sep_parsing &&
            (len < MAX_SEPARATOR_LEN - 1) &&
            (field->type != ELM_DATETIME_AMPM) &&
            (!strchr(ignore_separators, cur)) &&
            (!strchr(mapping[idx].ignore_sep, cur)))
          separator[len++] = cur;
        if (sep_lookup) sep_parsing = EINA_TRUE;
        sep_lookup = EINA_FALSE;
        fmt_ptr++;
     }
   // return the number of valid fields parsed.
   return location;
}

static void
_reload_format(Evas_Object *obj)
{
   unsigned int idx, field_count;
   Datetime_Field *field;
   char buf[BUFFER_SIZE];
   char *dt_fmt;

   ELM_DATETIME_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   // FIXME: provide nl_langinfo on Windows if possible
   // fetch the default format from Libc.
   if (!sd->user_format)
#ifdef HAVE_LANGINFO_H
     strncpy(sd->format, nl_langinfo(D_T_FMT), ELM_DATETIME_MAX_FORMAT_LEN);
#else
     strncpy(sd->format, "", ELM_DATETIME_MAX_FORMAT_LEN);
#endif
   sd->format[ELM_DATETIME_MAX_FORMAT_LEN - 1] = '\0';

   dt_fmt = (char *)malloc(ELM_DATETIME_MAX_FORMAT_LEN);
   if (!dt_fmt) return;

   strncpy(dt_fmt, sd->format, ELM_DATETIME_MAX_FORMAT_LEN);

   _expand_format(dt_fmt);

   // reset all the fields to disable state
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        field->fmt_exist = EINA_FALSE;
        field->location = -1;
     }

   field_count = _parse_format(obj, dt_fmt);
   free(dt_fmt);

   // assign locations to disabled fields for uniform usage
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if (field->location == -1) field->location = field_count++;

        if (field->fmt_exist && field->visible)
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_ENABLE_SIG_STR,
                      field->location);
             elm_layout_signal_emit(obj, buf, "elm");
          }
        else
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_DISABLE_SIG_STR,
                      field->location);
             elm_layout_signal_emit(obj, buf, "elm");
          }
        snprintf
          (buf, sizeof(buf), EDC_PART_SEPARATOR_STR, (field->location + 1));
        elm_layout_text_set(obj, buf, field->separator);
     }

   edje_object_message_signal_process(wd->resize_obj);
   _field_list_arrange(obj);
}

static void
_elm_datetime_smart_translate(Eo *obj, void *_pd, va_list *list)
{
   Elm_Datetime_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   if (!sd->user_format) _reload_format(obj);
   else _field_list_display(obj);

   evas_object_smart_callback_call(obj, SIG_LANG_CHANGED, NULL);

   if (ret) *ret = EINA_TRUE;
}

static Eina_List *
_datetime_items_get(const Evas_Object *obj)
{
   Eina_List *items = NULL;
   Datetime_Field *field;
   int loc = 0;
   unsigned int idx;
   Eina_Bool visible[ELM_DATETIME_TYPE_COUNT];

   ELM_DATETIME_DATA_GET(obj, sd);

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if (field->fmt_exist && field->visible) visible[idx] = EINA_TRUE;
        else visible[idx] = EINA_FALSE;
     }
   for (loc = 0; loc < ELM_DATETIME_TYPE_COUNT; loc++)
     {
        for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
          {
             field = sd->field_list + idx;
             if ((field->location == loc) && (visible[idx]))
               items = eina_list_append(items, field->item_obj);
          }
     }

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     items = eina_list_append(items, sd->access_obj);

   return items;
}

static void
_elm_datetime_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_datetime_smart_focus_next(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   if ((items = elm_widget_focus_custom_chain_get(obj)))
     {
        list_data_get = eina_list_data_get;
        list_free = NULL;
     }
   else
     {
        items = _datetime_items_get(obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;
        if (!items) return;
     }

   int_ret = elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);
   if (list_free) list_free((Eina_List *)items);

   if (ret) *ret = int_ret;
}

static void
_elm_datetime_smart_on_focus(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_wdg_on_focus(&int_ret));
   if (!int_ret) return;

   if (!elm_widget_focus_get(obj))
     {
        Elm_Datetime_Smart_Data *sd = _pd;

        if ((dt_mod) && (dt_mod->obj_hide))
          dt_mod->obj_hide(sd->mod_data);
     }

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_datetime_smart_disable(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Datetime_Field *field;
   unsigned int idx = 0;
   if (ret) *ret = EINA_FALSE;
   Elm_Datetime_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, elm_wdg_disable(ret));
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        elm_object_disabled_set(field->item_obj, elm_object_disabled_get(obj));
     }
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_datetime_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Datetime_Field *field;
   Evas_Coord minw = -1, minh = -1;
   unsigned int idx, field_count = 0;

   Elm_Datetime_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if ((field->visible) && (field->fmt_exist)) field_count++;
     }
   if (field_count)
     elm_coords_finger_size_adjust(field_count, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_elm_datetime_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   Datetime_Field *field;
   char buf[BUFFER_SIZE];
   unsigned int idx;

   Elm_Datetime_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, elm_wdg_theme(&int_ret));
   if (!int_ret) return;

   if ((!dt_mod) || (!dt_mod->field_value_display))
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if (field->fmt_exist && field->visible)
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_ENABLE_SIG_STR,
                      field->location);
             elm_layout_signal_emit(obj, buf, "elm");

             snprintf
               (buf, sizeof(buf), EDC_PART_SEPARATOR_STR, field->location);
             elm_layout_text_set(obj, buf, field->separator);

             dt_mod->field_value_display(sd->mod_data, field->item_obj);
          }
        else
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_DISABLE_SIG_STR,
                      field->location);
             elm_layout_signal_emit(obj, buf, "elm");
          }
     }

   edje_object_message_signal_process(wd->resize_obj);
   elm_layout_sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
}

static int
_max_days_get(int year,
              int month)
{
   struct tm time1;
   time_t t;
   int day;

   t = time(NULL);
   localtime_r(&t, &time1);
   time1.tm_year = year;
   time1.tm_mon = month;
   /* To restrict month wrapping because of summer time in some locales,
    * disable day light saving mode.*/
   time1.tm_isdst = 0;
   for (day = MIN_DAYS_IN_MONTH; day <= mapping[ELM_DATETIME_DATE].def_max;
        day++)
     {
        time1.tm_mday = day;
        mktime(&time1);
        if (time1.tm_mday == 1) break;
     }
   day--;

   return day;
}

static Eina_Bool
_date_cmp(struct tm *time1,
          struct tm *time2)
{
   unsigned int idx;

   DATETIME_TM_ARRAY(timearr1, time1);
   DATETIME_TM_ARRAY(timearr2, time2);

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT - 1; idx++)
     {
        if (*timearr1[idx] != *timearr2[idx])
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

// validates curr_time/min_limt/max_limit according to the newly set value
static void
_validate_datetime_limits(struct tm *time1,
                          struct tm *time2,
                          Eina_Bool swap)
{
   struct tm *t1, *t2;
   unsigned int idx;

   if (!time1 || !time2) return;

   t1 = (swap) ? time2 : time1;
   t2 = (swap) ? time1 : time2;

   DATETIME_TM_ARRAY(timearr1, time1);
   DATETIME_TM_ARRAY(timearr2, time2);
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT - 1; idx++)
     {
        if (*timearr1[idx] < *timearr2[idx])
          {
             memcpy(t1, t2, sizeof(struct tm));
             break;
          }
        else if (*timearr1[idx] > *timearr2[idx])
          break;
     }
}

static void
_apply_field_limits(Evas_Object *obj)
{
   Datetime_Field *field;
   unsigned int idx = 0;
   int val;

   ELM_DATETIME_DATA_GET(obj, sd);

   DATETIME_TM_ARRAY(timearr, &sd->curr_time);
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT - 1; idx++)
     {
        field = sd->field_list + idx;
        val = *timearr[idx];
        if (val < field->min)
          *timearr[idx] = field->min;
        else if (val > field->max)
          *timearr[idx] = field->max;
     }

   _field_list_display(obj);
}

static void
_apply_range_restrictions(struct tm *tim)
{
   unsigned int idx;
   int val, min, max;

   if (!tim) return;

   DATETIME_TM_ARRAY(timearr, tim);
   for (idx = ELM_DATETIME_MONTH; idx < ELM_DATETIME_TYPE_COUNT - 1; idx++)
     {
        val = *timearr[idx];
        min = mapping[idx].def_min;
        max = mapping[idx].def_max;
        if (idx == ELM_DATETIME_DATE)
          max = _max_days_get(tim->tm_year, tim->tm_mon);
        if (val < min)
          *timearr[idx] = min;
        else if (val > max)
          *timearr[idx] = max;
     }
}

static const char *
_field_format_get(Evas_Object *obj,
                  Elm_Datetime_Field_Type field_type)
{
   Datetime_Field *field;

   ELM_DATETIME_DATA_GET(obj, sd);

   field = sd->field_list + field_type;
   if (!field) return NULL;

   return field->fmt;
}

static void
_field_limit_get(Evas_Object *obj,
                 Elm_Datetime_Field_Type field_type,
                 int *range_min,
                 int *range_max)
{
   int min, max, max_days;
   Datetime_Field *field;
   unsigned int idx;

   ELM_DATETIME_DATA_GET(obj, sd);

   field = sd->field_list + field_type;
   if (!field) return;

   min = field->min;
   max = field->max;

   DATETIME_TM_ARRAY(curr_timearr, &sd->curr_time);
   DATETIME_TM_ARRAY(min_timearr, &sd->min_limit);
   DATETIME_TM_ARRAY(max_timearr, &sd->max_limit);

   for (idx = 0; idx < field->type; idx++)
     if (*curr_timearr[idx] > *min_timearr[idx]) break;
   if ((idx == field_type) && (min < *min_timearr[field_type]))
     min = *min_timearr[field_type];
   if (field_type == ELM_DATETIME_DATE)
     {
        max_days = _max_days_get(sd->curr_time.tm_year, sd->curr_time.tm_mon);
        if (max > max_days) max = max_days;
     }
   for (idx = 0; idx < field->type; idx++)
     if (*curr_timearr[idx] < *max_timearr[idx]) break;
   if ((idx == field_type) && (max > *max_timearr[field_type]))
     max = *max_timearr[field_type];

   *range_min = min;
   *range_max = max;
}

static void
_field_list_init(Evas_Object *obj)
{
   Datetime_Field *field;
   unsigned int idx;
   time_t t;

   ELM_DATETIME_DATA_GET(obj, sd);

   t = time(NULL);
   localtime_r(&t, &sd->curr_time);

   mapping[ELM_DATETIME_YEAR].def_min = _elm_config->year_min;
   mapping[ELM_DATETIME_YEAR].def_max = _elm_config->year_max;
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        field->type = ELM_DATETIME_YEAR + idx;
        field->fmt[0] = '%';
        field->fmt_exist = EINA_FALSE;
        field->visible = EINA_TRUE;
        field->min = mapping[idx].def_min;
        field->max = mapping[idx].def_max;
     }
   DATETIME_TM_ARRAY(min_timearr, &sd->min_limit);
   DATETIME_TM_ARRAY(max_timearr, &sd->max_limit);
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT - 1; idx++)
     {
        *min_timearr[idx] = mapping[idx].def_min;
        *max_timearr[idx] = mapping[idx].def_max;
     }
}

static char *
_access_info_cb(void *data, Evas_Object *obj __UNUSED__)
{
   char *ret;
   Eina_Strbuf *buf;
   buf = eina_strbuf_new();

   ELM_DATETIME_DATA_GET(data, sd);
   eina_strbuf_append_printf(buf,
                             "%d year, %d month, %d date, %d hour, %d minute",
                             sd->curr_time.tm_year, sd->curr_time.tm_mon + 1,
                             sd->curr_time.tm_mday, sd->curr_time.tm_hour,
                             sd->curr_time.tm_min);

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_elm_datetime_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Datetime_Smart_Data *priv = _pd;
   Datetime_Field *field;
   int idx;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set(obj, "datetime", "base",
                             elm_widget_style_get(obj)))
     CRITICAL("Failed to set layout!");

   // module - initialise module for datetime
   if (!dt_mod) dt_mod = _dt_mod_init();
   if ((dt_mod) && (dt_mod->obj_hook)) priv->mod_data = dt_mod->obj_hook(obj);

   // update module data
   if (priv->mod_data)
     {
        priv->mod_data->base = obj;
        priv->mod_data->field_limit_get = _field_limit_get;
        priv->mod_data->field_format_get = _field_format_get;
     }

   _field_list_init(obj);
   _reload_format(obj);

   if ((dt_mod) && (dt_mod->field_create))
     {
        for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
          {
             field = priv->field_list + idx;
             field->item_obj = dt_mod->field_create(priv->mod_data, idx);
          }
     }

   _field_list_arrange(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        Elm_Access_Info *ai;

        priv->access_obj = _elm_access_edje_object_part_object_register
          (obj, elm_layout_edje_get(obj), "elm.access");
        if (!priv->access_obj)
          priv->access_obj = _elm_access_edje_object_part_object_register
          (obj, elm_layout_edje_get(obj), "access");

        ai = _elm_access_info_get(priv->access_obj);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE, "date time");
        _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, obj);
     }
}

static void
_elm_datetime_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Datetime_Field *tmp;
   unsigned int idx;

   Elm_Datetime_Smart_Data *sd = _pd;

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        tmp = sd->field_list + idx;
        evas_object_del(tmp->item_obj);
        eina_stringshare_del(tmp->separator);
     }

   if ((dt_mod) && (dt_mod->obj_unhook))
     dt_mod->obj_unhook(sd->mod_data);  // module - unhook

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_datetime_add(Evas_Object *parent)
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

EAPI const char *
elm_datetime_format_get(const Evas_Object *obj)
{
   ELM_DATETIME_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_datetime_format_get(&ret));
   return ret;
}

static void
_format_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Datetime_Smart_Data *sd = _pd;
   *ret = sd->format;
}

EAPI void
elm_datetime_format_set(Evas_Object *obj,
                        const char *fmt)
{
   ELM_DATETIME_CHECK(obj);
   eo_do(obj, elm_obj_datetime_format_set(fmt));
}

static void
_format_set(Eo *obj, void *_pd, va_list *list)
{
   const char *fmt = va_arg(*list, const char *);
   Elm_Datetime_Smart_Data *sd = _pd;

   if (fmt)
     {
        strncpy(sd->format, fmt, ELM_DATETIME_MAX_FORMAT_LEN);
        sd->format[ELM_DATETIME_MAX_FORMAT_LEN - 1] = '\0';
        sd->user_format = EINA_TRUE;
     }
   else sd->user_format = EINA_FALSE;

   _reload_format(obj);
}

EAPI Eina_Bool
elm_datetime_field_visible_get(const Evas_Object *obj,
                               Elm_Datetime_Field_Type fieldtype)
{
   ELM_DATETIME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_datetime_field_visible_get(fieldtype, &ret));
   return ret;
}

static void
_field_visible_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Datetime_Field_Type fieldtype = va_arg(*list, Elm_Datetime_Field_Type);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;

   Datetime_Field *field;
   Elm_Datetime_Smart_Data *sd = _pd;

   if (fieldtype > ELM_DATETIME_AMPM) return;

   field = sd->field_list + fieldtype;

   *ret = field->visible;
}

EAPI void
elm_datetime_field_visible_set(Evas_Object *obj,
                               Elm_Datetime_Field_Type fieldtype,
                               Eina_Bool visible)
{
   ELM_DATETIME_CHECK(obj);
   eo_do(obj, elm_obj_datetime_field_visible_set(fieldtype, visible));
}

static void
_field_visible_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Datetime_Field_Type fieldtype = va_arg(*list, Elm_Datetime_Field_Type);
   Eina_Bool visible = va_arg(*list, int);
   Datetime_Field *field;

   Elm_Datetime_Smart_Data *sd = _pd;

   if (fieldtype > ELM_DATETIME_AMPM) return;

   field = sd->field_list + fieldtype;
   if (field->visible == visible) return;

   field->visible = visible;
   _reload_format(obj);
}

EAPI void
elm_datetime_field_limit_get(const Evas_Object *obj,
                             Elm_Datetime_Field_Type fieldtype,
                             int *min,
                             int *max)
{
   ELM_DATETIME_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_datetime_field_limit_get(fieldtype, min, max));
}

static void
_elm_datetime_field_limit_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Datetime_Field_Type fieldtype = va_arg(*list, Elm_Datetime_Field_Type);
   int *min = va_arg(*list, int *);
   int *max = va_arg(*list, int *);

   Datetime_Field *field;

   Elm_Datetime_Smart_Data *sd = _pd;

   if (fieldtype >= ELM_DATETIME_AMPM) return;

   field = sd->field_list + fieldtype;
   if (min) *min = field->min;
   if (max) *max = field->max;
}

EAPI void
elm_datetime_field_limit_set(Evas_Object *obj,
                             Elm_Datetime_Field_Type fieldtype,
                             int min,
                             int max)
{
   ELM_DATETIME_CHECK(obj);
   eo_do(obj, elm_obj_datetime_field_limit_set(fieldtype, min, max));
}

static void
_elm_datetime_field_limit_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Datetime_Field_Type fieldtype = va_arg(*list, Elm_Datetime_Field_Type);
   int min = va_arg(*list, int);
   int max = va_arg(*list, int);

   Datetime_Field *field;

   Elm_Datetime_Smart_Data *sd = _pd;

   if (fieldtype >= ELM_DATETIME_AMPM) return;

   if (min > max) return;

   field = sd->field_list + fieldtype;
   if (((min >= mapping[fieldtype].def_min) &&
        (min <= mapping[fieldtype].def_max)) ||
       (field->type == ELM_DATETIME_YEAR))
     field->min = min;
   if (((max >= mapping[fieldtype].def_min) &&
        (max <= mapping[fieldtype].def_max)) ||
       (field->type == ELM_DATETIME_YEAR))
     field->max = max;

   _apply_field_limits(obj);
}

EAPI Eina_Bool
elm_datetime_value_get(const Evas_Object *obj,
                       struct tm *currtime)
{
   ELM_DATETIME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_datetime_value_get(currtime, &ret));
   return ret;
}

static void
_value_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   struct tm *currtime = va_arg(*list, struct tm *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN(currtime);
   Elm_Datetime_Smart_Data *sd = _pd;

   *currtime = sd->curr_time;
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_set(Evas_Object *obj,
                       const struct tm *newtime)
{
   ELM_DATETIME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_datetime_value_set(newtime, &ret));
   return ret;
}

static void
_value_set(Eo *obj, void *_pd, va_list *list)
{
   const struct tm *newtime = va_arg(*list, const struct tm *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   struct tm old_time;

   EINA_SAFETY_ON_NULL_RETURN(newtime);
   Elm_Datetime_Smart_Data *sd = _pd;

   old_time = sd->curr_time;
   sd->curr_time = *newtime;
   // apply default field restrictions for curr_time
   _apply_range_restrictions(&sd->curr_time);
   // validate the curr_time according to the min_limt and max_limt
   _validate_datetime_limits(&sd->curr_time, &sd->min_limit, EINA_FALSE);
   _validate_datetime_limits(&sd->max_limit, &sd->curr_time, EINA_TRUE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &sd->curr_time))
     evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_min_get(const Evas_Object *obj,
                           struct tm *mintime)
{
   ELM_DATETIME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_datetime_value_min_get(mintime, &ret));
   return ret;
}

static void
_value_min_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   struct tm *mintime = va_arg(*list, struct tm *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN(mintime);
   Elm_Datetime_Smart_Data *sd = _pd;

   *mintime = sd->min_limit;
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_min_set(Evas_Object *obj,
                           const struct tm *mintime)
{
   ELM_DATETIME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_datetime_value_min_set(mintime, &ret));
   return ret;
}

static void
_value_min_set(Eo *obj, void *_pd, va_list *list)
{
   const struct tm *mintime = va_arg(*list, const struct tm *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   struct tm old_time;

   EINA_SAFETY_ON_NULL_RETURN(mintime);
   Elm_Datetime_Smart_Data *sd = _pd;

   sd->min_limit = *mintime;
   old_time = sd->curr_time;
   // apply default field restrictions for min_limit
   _apply_range_restrictions(&sd->min_limit);
   // validate curr_time and max_limt according to the min_limit
   _validate_datetime_limits(&sd->max_limit, &sd->min_limit, EINA_FALSE);
   _validate_datetime_limits(&sd->curr_time, &sd->min_limit, EINA_FALSE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &sd->curr_time))
     evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_max_get(const Evas_Object *obj,
                           struct tm *maxtime)
{
   ELM_DATETIME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_datetime_value_max_get(maxtime, &ret));
   return ret;
}

static void
_value_max_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   struct tm *maxtime = va_arg(*list, struct tm *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN(maxtime);
   Elm_Datetime_Smart_Data *sd = _pd;

   *maxtime = sd->max_limit;
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_max_set(Evas_Object *obj,
                           const struct tm *maxtime)
{
   ELM_DATETIME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_datetime_value_max_set(maxtime, &ret));
   return ret;
}

static void
_value_max_set(Eo *obj, void *_pd, va_list *list)
{
   const struct tm *maxtime = va_arg(*list, const struct tm *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   struct tm old_time;

   EINA_SAFETY_ON_NULL_RETURN(maxtime);
   Elm_Datetime_Smart_Data *sd = _pd;

   sd->max_limit = *maxtime;
   old_time = sd->curr_time;
   // apply default field restrictions for max_limit
   _apply_range_restrictions(&sd->max_limit);
   // validate curr_time and min_limt according to the max_limit
   _validate_datetime_limits(&sd->max_limit, &sd->min_limit, EINA_TRUE);
   _validate_datetime_limits(&sd->max_limit, &sd->curr_time, EINA_TRUE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &sd->curr_time))
     evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   if (ret) *ret = EINA_TRUE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_datetime_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_datetime_smart_del),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TRANSLATE), _elm_datetime_smart_translate),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_datetime_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT), _elm_datetime_smart_focus_next),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME), _elm_datetime_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ON_FOCUS), _elm_datetime_smart_on_focus),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DISABLE), _elm_datetime_smart_disable),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_datetime_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FORMAT_GET), _format_get),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FORMAT_SET), _format_set),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FIELD_VISIBLE_GET), _field_visible_get),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FIELD_VISIBLE_SET), _field_visible_set),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FIELD_LIMIT_GET), _elm_datetime_field_limit_get),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FIELD_LIMIT_SET), _elm_datetime_field_limit_set),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_GET), _value_get),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_SET), _value_set),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_MIN_GET), _value_min_get),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_MIN_SET), _value_min_set),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_MAX_GET), _value_max_get),
        EO_OP_FUNC(ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_MAX_SET), _value_max_set),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_FORMAT_GET, "Get the datetime format."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_FORMAT_SET, "Set the datetime format."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_FIELD_VISIBLE_GET, "Get whether a field can be visible/not."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_FIELD_VISIBLE_SET, "Set a field to be visible or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_FIELD_LIMIT_GET, "Get the field limits of a field."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_FIELD_LIMIT_SET, "Set the field limits of a field."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_VALUE_GET, "Get the current value of a field."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_VALUE_SET, "Set the current value of a Datetime object."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_VALUE_MIN_GET, "Get the lower boundary of a field."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_VALUE_MIN_SET, "Set the lower boundary of a field."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_VALUE_MAX_GET, "Get the upper boundary of a field."),
     EO_OP_DESCRIPTION(ELM_OBJ_DATETIME_SUB_ID_VALUE_MAX_SET, "Set the upper boundary of a field."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_DATETIME_BASE_ID, op_desc, ELM_OBJ_DATETIME_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Datetime_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_datetime_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
