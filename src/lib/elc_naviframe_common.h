/**
 * @typedef Elm_Naviframe_Item_Pop_Cb
 *
 * Pop callback called when @c it is going to be popped. @c data is user
 * specific data. If it returns the @c EINA_FALSE in the callback, item popping
 * will be cancelled.
 *
 * @see elm_naviframe_item_pop_cb_set()
 *
 * @since 1.8
 */
typedef Eina_Bool (*Elm_Naviframe_Item_Pop_Cb)(void *data, Elm_Object_Item *it);

/**
 * @brief Add a new Naviframe object to the parent.
 *
 * @param parent Parent object
 * @return New object or @c NULL, if it cannot be created
 *
 * @ingroup Naviframe
 */
EAPI Evas_Object     *elm_naviframe_add(Evas_Object *parent);

EAPI void             elm_naviframe_item_pop_to(Elm_Object_Item *it);

EAPI void             elm_naviframe_item_promote(Elm_Object_Item *it);

EAPI void             elm_naviframe_item_style_set(Elm_Object_Item *it, const char *item_style);

EAPI const char      *elm_naviframe_item_style_get(const Elm_Object_Item *it);

EAPI void             elm_naviframe_item_title_enabled_set(Elm_Object_Item *it, Eina_Bool enabled, Eina_Bool transition);

EAPI Eina_Bool        elm_naviframe_item_title_enabled_get(const Elm_Object_Item *it);

EAPI void             elm_naviframe_item_pop_cb_set(Elm_Object_Item *it, Elm_Naviframe_Item_Pop_Cb func, void *data);

Elm_Object_Item *elm_naviframe_item_push(Evas_Object *obj, const char *title_label, Evas_Object *prev_btn, Evas_Object *next_btn, Evas_Object *content, const char *item_style);
/**
 * @brief Simple version of item_push.
 *
 * @see elm_naviframe_item_push
 */
static inline Elm_Object_Item *
elm_naviframe_item_simple_push(Evas_Object *obj, Evas_Object *content)
{
   Elm_Object_Item *it;
   it = elm_naviframe_item_push(obj, NULL, NULL, NULL, content, NULL);
   elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
   return it;
}
