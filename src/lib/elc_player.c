#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Emotion.h>

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_layout.h"
#include "elm_widget_player.h"

EAPI Eo_Op ELM_OBJ_PLAYER_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_PLAYER_CLASS

#define MY_CLASS_NAME "Elm_Player"
#define MY_CLASS_NAME_LEGACY "elm_player"

static const char SIG_FORWARD_CLICKED[] = "forward,clicked";
static const char SIG_INFO_CLICKED[] = "info,clicked";
static const char SIG_NEXT_CLICKED[] = "next,clicked";
static const char SIG_PAUSE_CLICKED[] = "pause,clicked";
static const char SIG_PLAY_CLICKED[] = "play,clicked";
static const char SIG_PREV_CLICKED[] = "prev,clicked";
static const char SIG_REWIND_CLICKED[] = "rewind,clicked";
static const char SIG_STOP_CLICKED[] = "stop,clicked";
static const char SIG_EJECT_CLICKED[] = "eject,clicked";
static const char SIG_VOLUME_CLICKED[] = "volume,clicked";
static const char SIG_MUTE_CLICKED[] = "mute,clicked";
/*
static const char SIG_STOP_CLICKED[] = "repeat,clicked";
static const char SIG_STOP_CLICKED[] = "shuffle,clicked";
static const char SIG_STOP_CLICKED[] = "record,clicked";
static const char SIG_STOP_CLICKED[] = "replay,clicked";
static const char SIG_STOP_CLICKED[] = "power,clicked";
static const char SIG_STOP_CLICKED[] = "fullscreen,clicked";
static const char SIG_STOP_CLICKED[] = "normal,clicked";
static const char SIG_STOP_CLICKED[] = "quality,clicked";
 */

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   { SIG_FORWARD_CLICKED, "" },
   { SIG_INFO_CLICKED, "" },
   { SIG_NEXT_CLICKED, "" },
   { SIG_PAUSE_CLICKED, "" },
   { SIG_PLAY_CLICKED, "" },
   { SIG_PREV_CLICKED, "" },
   { SIG_REWIND_CLICKED, "" },
   { SIG_STOP_CLICKED, "" },
   { SIG_EJECT_CLICKED, "" },
   { SIG_VOLUME_CLICKED, "" },
   { SIG_MUTE_CLICKED, "" },
   {"focused", ""}, /**< handled by elm_widget */
   {"unfocused", ""}, /**< handled by elm_widget */
   { NULL, NULL }
};

static void
_elm_player_smart_event(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   Evas_Event_Key_Down *ev = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Player_Smart_Data *sd = _pd;

   if (ret) *ret = EINA_FALSE;
   (void) src;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!sd->video) return;

   if ((!strcmp(ev->key, "Left")) ||
       ((!strcmp(ev->key, "KP_Left")) && (!ev->string)))
     {
        double current, last;

        current = elm_video_play_position_get(sd->video);
        last = elm_video_play_length_get(sd->video);

        if (current < last)
          {
             current -= last / 100;
             elm_video_play_position_set(sd->video, current);
          }

        goto success;
     }
   else if ((!strcmp(ev->key, "Right")) ||
            ((!strcmp(ev->key, "KP_Right")) && (!ev->string)))
     {
        double current, last;

        current = elm_video_play_position_get(sd->video);
        last = elm_video_play_length_get(sd->video);

        if (current > 0)
          {
             current += last / 100;
             if (current < 0) current = 0;
             elm_video_play_position_set(sd->video, current);
          }

        goto success;
     }
   else if (!strcmp(ev->key, "space"))
     {
        if (elm_video_is_playing_get(sd->video))
          elm_video_pause(sd->video);
        else
          elm_video_play(sd->video);

        goto success;
     }

   return;

success:
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_update_theme_button(Evas_Object *obj, Evas_Object *bt, const char *name)
{
   Evas_Object *ic;
   char buf[256];

   if (!bt) return;
   ic = evas_object_data_get(bt, "icon");
   if (ic)
     {
        snprintf(buf, sizeof(buf), "media_player/%s/%s", name,
                 elm_widget_style_get(obj));
        elm_icon_standard_set(ic, buf);
     }
   snprintf(buf, sizeof(buf), "media_player/%s/%s", name,
            elm_widget_style_get(obj));
   elm_object_style_set(bt, buf);
   snprintf(buf, sizeof(buf), "elm.swallow.media_player.%s", name);
   if (!elm_layout_content_set(obj, buf, bt))
     evas_object_hide(bt);
   elm_object_disabled_set(bt, elm_widget_disabled_get(obj));
}

static void
_update_theme_slider(Evas_Object *obj, Evas_Object *sl, const char *name, const char *name2)
{
   char buf[256];

   if (!sl) return;
   snprintf(buf, sizeof(buf), "media_player/%s/%s", name,
            elm_widget_style_get(obj));
   elm_object_style_set(sl, buf);
   snprintf(buf, sizeof(buf), "elm.swallow.media_player.%s", name2);
   if (!elm_layout_content_set(obj, buf, sl))
     evas_object_hide(sl);
   elm_object_disabled_set(sl, elm_widget_disabled_get(obj));
}

static void
_elm_player_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   Elm_Player_Smart_Data *sd = _pd;
   eo_do_super(obj, MY_CLASS, elm_wdg_theme(&int_ret));
   if (!int_ret) return;
   _update_theme_button(obj, sd->forward, "forward");
   _update_theme_button(obj, sd->info, "info");
   _update_theme_button(obj, sd->next, "next");
   _update_theme_button(obj, sd->pause, "pause");
   _update_theme_button(obj, sd->play, "play");
   _update_theme_button(obj, sd->prev, "prev");
   _update_theme_button(obj, sd->rewind, "rewind");
   _update_theme_button(obj, sd->next, "next");
   _update_theme_button(obj, sd->stop, "stop");
   _update_theme_button(obj, sd->eject, "eject");
   _update_theme_button(obj, sd->volume, "volume");
   _update_theme_button(obj, sd->mute, "mute");
   _update_theme_slider(obj, sd->slider,  "position", "positionslider");
   _update_theme_slider(obj, sd->vslider,  "volume", "volumeslider");
   elm_layout_sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_player_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Coord w, h;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_get(wd->resize_obj, &w, &h);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &w, &h, w, h);
   evas_object_size_hint_min_set(obj, w, h);
}

static void
_update_slider(void *data,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   double pos, length;
   Eina_Bool seekable;

   ELM_PLAYER_DATA_GET(data, sd);
   if (!sd) return;

   seekable = elm_video_is_seekable_get(sd->video);
   length = elm_video_play_length_get(sd->video);
   pos = elm_video_play_position_get(sd->video);

   elm_object_disabled_set(sd->slider,
                           (!seekable) | elm_widget_disabled_get(data));
   elm_slider_min_max_set(sd->slider, 0, length);
   if ((elm_slider_value_get(sd->slider) != pos) &&
       (!sd->dragging))
     elm_slider_value_set(sd->slider, pos);
}

static void
_update_frame(void *data,
              Evas_Object *obj,
              void *event_info)
{
   ELM_PLAYER_DATA_GET(data, sd);
   if (!sd) return;
   _update_slider(data, obj, event_info);
}

static void
_update_position(void *data,
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
{
   double pos;
   ELM_PLAYER_DATA_GET(data, sd);

   pos = elm_slider_value_get(sd->slider);
   if (pos != elm_video_play_position_get(sd->video))
     elm_video_play_position_set(sd->video, pos);
}

static void
_drag_start(void *data,
            Evas_Object *obj __UNUSED__,
            void *event_info __UNUSED__)
{
   ELM_PLAYER_DATA_GET(data, sd);
   sd->dragging = EINA_TRUE;
}

static void
_drag_stop(void *data,
            Evas_Object *obj __UNUSED__,
            void *event_info __UNUSED__)
{
   ELM_PLAYER_DATA_GET(data, sd);
   sd->dragging = EINA_FALSE;
}

static void
_update_volume(void *data,
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
{
   double vol;
   ELM_PLAYER_DATA_GET(data, sd);

   vol = elm_slider_value_get(sd->vslider) / 100.0;
   if (vol != elm_video_audio_level_get(sd->video))
     elm_video_audio_level_set(sd->video, vol);
}

static void
_forward(void *data,
         Evas_Object *obj __UNUSED__,
         void *event_info __UNUSED__)
{
   double pos, length;
   ELM_PLAYER_DATA_GET(data, sd);

   length = elm_video_play_length_get(sd->video);
   pos = elm_video_play_position_get(sd->video);
   pos += 30.0;
   if (pos > length) pos = length;
   elm_video_play_position_set(sd->video, pos);

   elm_layout_signal_emit(data, "elm,button,forward", "elm");
   evas_object_smart_callback_call(data, SIG_FORWARD_CLICKED, NULL);
}

static void
_info(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,button,info", "elm");
   evas_object_smart_callback_call(data, SIG_INFO_CLICKED, NULL);
}

static void
_next(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,button,next", "elm");
   evas_object_smart_callback_call(data, SIG_NEXT_CLICKED, NULL);
}

static void
_pause(void *data,
       Evas_Object *obj __UNUSED__,
       void *event_info __UNUSED__)
{
   ELM_PLAYER_DATA_GET(data, sd);

   elm_layout_signal_emit(data, "elm,player,pause", "elm");
   elm_video_pause(sd->video);
   evas_object_smart_callback_call(data, SIG_PAUSE_CLICKED, NULL);
}

static void
_play(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   ELM_PLAYER_DATA_GET(data, sd);

   elm_layout_signal_emit(data, "elm,player,play", "elm");
   elm_video_play(sd->video);
   evas_object_smart_callback_call(data, SIG_PLAY_CLICKED, NULL);
}

static void
_prev(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_PREV_CLICKED, NULL);
   elm_layout_signal_emit(data, "elm,button,prev", "elm");
}

static void
_rewind(void *data,
        Evas_Object *obj __UNUSED__,
        void *event_info __UNUSED__)
{
   double pos;
   ELM_PLAYER_DATA_GET(data, sd);

   pos = elm_video_play_position_get(sd->video);
   pos -= 30.0;
   if (pos < 0.0) pos = 0.0;
   elm_video_play_position_set(sd->video, pos);

   elm_layout_signal_emit(data, "elm,button,rewind", "elm");
   evas_object_smart_callback_call(data, SIG_REWIND_CLICKED, NULL);
}

static void
_stop(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,button,stop", "elm");
   evas_object_smart_callback_call(data, SIG_STOP_CLICKED, NULL);
}

static void
_eject(void *data,
       Evas_Object *obj __UNUSED__,
       void *event_info __UNUSED__)
{
   ELM_PLAYER_DATA_GET(data, sd);

   elm_layout_signal_emit(data, "elm,button,eject", "elm");
   emotion_object_eject(elm_video_emotion_get(sd->video));
   evas_object_smart_callback_call(data, SIG_EJECT_CLICKED, NULL);
}

static void
_mute_toggle(Evas_Object *obj)
{
   ELM_PLAYER_DATA_GET(obj, sd);

   if (!elm_video_audio_mute_get(sd->video))
     {
        elm_video_audio_mute_set(sd->video, EINA_TRUE);
        elm_layout_signal_emit(obj, "elm,player,mute", "elm");
     }
   else
     {
        elm_video_audio_mute_set(sd->video, EINA_FALSE);
        elm_layout_signal_emit(obj, "elm,player,unmute", "elm");
     }
}

static void
_volume(void *data,
        Evas_Object *obj __UNUSED__,
        void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,button,volume", "elm");
   _mute_toggle(data);
   evas_object_smart_callback_call(data, SIG_VOLUME_CLICKED, NULL);
}

static void
_mute(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,button,mute", "elm");
   _mute_toggle(data);
   evas_object_smart_callback_call(data, SIG_MUTE_CLICKED, NULL);
}

static void
_play_started(void *data,
              Evas_Object *obj __UNUSED__,
              void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,player,play", "elm");
}

static void
_play_finished(void *data,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,player,pause", "elm");
}

static void
_on_video_del(Elm_Player_Smart_Data *sd)
{
   elm_object_disabled_set(sd->forward, EINA_TRUE);
   elm_object_disabled_set(sd->info, EINA_TRUE);
   elm_object_disabled_set(sd->next, EINA_TRUE);
   elm_object_disabled_set(sd->pause, EINA_TRUE);
   elm_object_disabled_set(sd->play, EINA_TRUE);
   elm_object_disabled_set(sd->prev, EINA_TRUE);
   elm_object_disabled_set(sd->rewind, EINA_TRUE);
   elm_object_disabled_set(sd->next, EINA_TRUE);
   elm_object_disabled_set(sd->stop, EINA_TRUE);
   elm_object_disabled_set(sd->volume, EINA_TRUE);
   elm_object_disabled_set(sd->mute, EINA_TRUE);
   elm_object_disabled_set(sd->slider, EINA_TRUE);
   elm_object_disabled_set(sd->vslider, EINA_TRUE);

   sd->video = NULL;
   sd->emotion = NULL;
}

static void
_video_del(void *data,
           Evas *e __UNUSED__,
           Evas_Object *obj __UNUSED__,
           void *event_info __UNUSED__)
{
   _on_video_del(data);
}

static Evas_Object *
_player_button_add(Evas_Object *obj,
                   const char *name,
                   Evas_Smart_Cb func)
{
   Evas_Object *ic;
   Evas_Object *bt;
   char buf[256];

   ic = elm_icon_add(obj);
   snprintf(buf, sizeof(buf), "media_player/%s/%s", name,
            elm_widget_style_get(obj));
   elm_icon_standard_set(ic, buf);

   bt = elm_button_add(obj);
   if (ic) evas_object_data_set(bt, "icon", ic);
   elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
   elm_object_content_set(bt, ic);
   evas_object_show(ic);

   snprintf(buf, sizeof(buf), "media_player/%s/%s", name,
            elm_widget_style_get(obj));
   elm_object_style_set(bt, buf);
   evas_object_smart_callback_add(bt, "clicked", func, obj);
   snprintf(buf, sizeof(buf), "elm.swallow.media_player.%s", name);
   if (!elm_layout_content_set(obj, buf, bt))
     {
        elm_widget_sub_object_add(obj, bt);
        evas_object_hide(bt);
     }
   evas_object_show(bt);
   return bt;
}

static char *
_double_to_time(double value)
{
   char buf[256];
   int ph, pm, ps, pf;

   ph = value / 3600;
   pm = value / 60 - (ph * 60);
   ps = value - (pm * 60) - (ph * 3600);
   pf = value * 100 - (ps * 100) - (pm * 60 * 100) - (ph * 60 * 60 * 100);

   if (ph)
     snprintf(buf, sizeof(buf), "%i:%02i:%02i.%02i",
              ph, pm, ps, pf);
   else if (pm)
     snprintf(buf, sizeof(buf), "%02i:%02i.%02i",
              pm, ps, pf);
   else
     snprintf(buf, sizeof(buf), "%02i.%02i",
              ps, pf);
   return (char *)eina_stringshare_add(buf);
}

static void
_str_free(char *data)
{
   eina_stringshare_del(data);
}

/* a video object is never parented by a player one, just tracked.
 * treating this special case here and delegating other objects to own
 * layout */

static void
_elm_player_smart_content_set(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   Elm_Player_Smart_Data *sd = _pd;

   double pos, length;
   Eina_Bool seekable;

   if (part && strcmp(part, "video"))
     {
        eo_do_super(obj, MY_CLASS,
                    elm_obj_container_content_set(part, content, &int_ret));
        if (ret) *ret = int_ret;
        return;
     }
   if ((!part) || (!strcmp(part, "video"))) part = "elm.swallow.content";
   eo_do_super(obj, MY_CLASS,
               elm_obj_container_content_set(part, content, &int_ret));
   if (ret) *ret = int_ret;

   if (!_elm_video_check(content)) return;
   if (sd->video == content) goto end;

   if (sd->video) evas_object_del(sd->video);

   sd->video = content;

   if (!content) goto end;

   elm_object_disabled_set(sd->forward, EINA_FALSE);
   elm_object_disabled_set(sd->info, EINA_FALSE);
   elm_object_disabled_set(sd->next, EINA_FALSE);
   elm_object_disabled_set(sd->pause, EINA_FALSE);
   elm_object_disabled_set(sd->play, EINA_FALSE);
   elm_object_disabled_set(sd->prev, EINA_FALSE);
   elm_object_disabled_set(sd->rewind, EINA_FALSE);
   elm_object_disabled_set(sd->next, EINA_FALSE);
   elm_object_disabled_set(sd->stop, EINA_FALSE);
   elm_object_disabled_set(sd->volume, EINA_FALSE);
   elm_object_disabled_set(sd->mute, EINA_FALSE);
   elm_object_disabled_set(sd->slider, EINA_FALSE);
   elm_object_disabled_set(sd->vslider, EINA_FALSE);

   sd->emotion = elm_video_emotion_get(sd->video);
   emotion_object_priority_set(sd->emotion, EINA_TRUE);
   evas_object_event_callback_add
     (sd->video, EVAS_CALLBACK_DEL, _video_del, sd);

   seekable = elm_video_is_seekable_get(sd->video);
   length = elm_video_play_length_get(sd->video);
   pos = elm_video_play_position_get(sd->video);

   elm_object_disabled_set(sd->slider, !seekable);
   elm_slider_min_max_set(sd->slider, 0, length);
   elm_slider_value_set(sd->slider, pos);

   elm_slider_value_set(sd->vslider,
                        elm_video_audio_level_get(sd->video) * 100.0);
   // XXX: get mute

   if (elm_video_is_playing_get(sd->video))
     elm_layout_signal_emit(obj, "elm,player,play", "elm");
   else elm_layout_signal_emit(obj, "elm,player,pause", "elm");

   evas_object_smart_callback_add(sd->emotion, "frame_decode",
                                  _update_frame, obj);
   evas_object_smart_callback_add(sd->emotion, "frame_resize",
                                  _update_slider, obj);
   evas_object_smart_callback_add(sd->emotion, "length_change",
                                  _update_slider, obj);
   evas_object_smart_callback_add(sd->emotion, "position_update",
                                  _update_frame, obj);
   evas_object_smart_callback_add(sd->emotion, "playback_started",
                                  _play_started, obj);
   evas_object_smart_callback_add(sd->emotion, "playback_finished",
                                  _play_finished, obj);

   /* FIXME: track info from video */
end:
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_player_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Player_Smart_Data *priv = _pd;
   char buf[256];

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set(obj, "player", "base", elm_widget_style_get(obj)))
     CRITICAL("Failed to set layout!");

   priv->forward = _player_button_add(obj, "forward", _forward);
   priv->info = _player_button_add(obj, "info", _info);
   priv->next = _player_button_add(obj, "next", _next);
   priv->pause = _player_button_add(obj, "pause", _pause);
   priv->play = _player_button_add(obj, "play", _play);
   priv->prev = _player_button_add(obj, "prev", _prev);
   priv->rewind = _player_button_add(obj, "rewind", _rewind);
   priv->stop = _player_button_add(obj, "stop", _stop);
   priv->eject = _player_button_add(obj, "eject", _eject);
   priv->volume = _player_button_add(obj, "volume", _volume);
   priv->mute = _player_button_add(obj, "mute", _mute);

   priv->slider = elm_slider_add(obj);
   snprintf(buf, sizeof(buf), "media_player/position/%s",
            elm_widget_style_get(obj));
   elm_object_style_set(priv->slider, buf);
   elm_slider_indicator_show_set(priv->slider, EINA_TRUE);
   elm_slider_indicator_format_function_set
     (priv->slider, _double_to_time, _str_free);
   elm_slider_units_format_function_set
     (priv->slider, _double_to_time, _str_free);
   elm_slider_min_max_set(priv->slider, 0, 0);
   elm_slider_value_set(priv->slider, 0);
   elm_object_disabled_set(priv->slider, EINA_TRUE);
   evas_object_size_hint_align_set(priv->slider, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set
     (priv->slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_layout_content_set(obj, "elm.swallow.media_player.positionslider",
                          priv->slider);
   evas_object_smart_callback_add
     (priv->slider, "changed", _update_position, obj);
   evas_object_smart_callback_add
     (priv->slider, "slider,drag,start", _drag_start, obj);
   evas_object_smart_callback_add
     (priv->slider, "slider,drag,stop", _drag_stop, obj);

   priv->vslider = elm_slider_add(obj);
   elm_slider_indicator_show_set(priv->vslider, EINA_FALSE);
   snprintf(buf, sizeof(buf), "media_player/volume/%s",
            elm_widget_style_get(obj));
   elm_object_style_set(priv->vslider, buf);
   elm_slider_min_max_set(priv->vslider, 0, 100);
   elm_slider_value_set(priv->vslider, 100);
   elm_slider_horizontal_set(priv->vslider, EINA_FALSE);
   elm_slider_inverted_set(priv->vslider, EINA_TRUE);
   evas_object_size_hint_align_set(priv->vslider, 0.5, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set
     (priv->vslider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_layout_content_set(obj, "elm.swallow.media_player.volumeslider",
                          priv->vslider);
   evas_object_smart_callback_add
     (priv->vslider, "changed", _update_volume, obj);

   elm_layout_sizing_eval(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static void
_elm_player_smart_del(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_player_add(Evas_Object *parent)
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

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {

        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_player_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_player_smart_del),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME), _elm_player_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT), _elm_player_smart_event),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_player_smart_content_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_player_smart_sizing_eval),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_PLAYER_BASE_ID, op_desc, ELM_OBJ_PLAYER_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Player_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_player_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
