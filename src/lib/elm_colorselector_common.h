/**
 * @addtogroup Colorselector
 *
 * @{
 */

typedef struct _Elm_Color_RGBA Elm_Color_RGBA;
struct _Elm_Color_RGBA
{
   unsigned int r;
   unsigned int g;
   unsigned int b;
   unsigned int a;
};

typedef struct _Elm_Custom_Palette Elm_Custom_Palette;
struct _Elm_Custom_Palette
{
   const char *palette_name;
   Eina_List  *color_list;
};

/* TIZEN_ONLY(20160705): Move Elm_Colorselector_Mode enum to
                         elm_colorselector_common.h to mark enum members
                         as deprecated. */
typedef enum
{
   ELM_COLORSELECTOR_PALETTE = 0, /**< Only color palette is displayed, default */
   ELM_COLORSELECTOR_COMPONENTS, /**< Only color selector is displayed */
   ELM_COLORSELECTOR_BOTH, /**< Both Palette and selector is displayed */
   ELM_COLORSELECTOR_PICKER, /**< Only color picker is displayed */
   ELM_COLORSELECTOR_PLANE, /**< @deprecated This mode is not supported. If you use this, nothing will be shown */
   ELM_COLORSELECTOR_PALETTE_PLANE, /**< @deprecated This mode is not supported. If you use this, it will be shown same with ELM_COLORSELECTOR_PALETTE mode */
   ELM_COLORSELECTOR_ALL /**< @deprecated This mode is not supported. If you use this, it will be shown same with ELM_COLORSELECTOR_PALETTE mode */
} Elm_Colorselector_Mode;
/* END */

EAPI void elm_colorselector_palette_item_color_get(const Elm_Object_Item *it, int *r, int *g, int *b, int *a);

EAPI void elm_colorselector_palette_item_color_set(Elm_Object_Item *it, int r, int g, int b, int a);

EAPI Eina_Bool elm_colorselector_palette_item_selected_get(const Elm_Object_Item *it);

EAPI void elm_colorselector_palette_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/**
 * @}
 */
