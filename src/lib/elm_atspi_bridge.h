#ifdef EFL_BETA_API_SUPPORT
#ifdef EFL_EO_API_SUPPORT
#include "elm_atspi_bridge.eo.h"
//TIZEN_ONLY(20160527) - Add direct reading feature
typedef void (*Elm_Atspi_Say_Cb)(void *data);
EAPI void elm_atspi_bridge_utils_say(const char* text, Eina_Bool interruptable, const Elm_Atspi_Say_Cb func, const void *data);
EAPI void elm_atspi_bridge_utils_reading_cb_register(Eo* obj, const Elm_Atspi_Say_Cb func, const void *data);
EAPI Eina_Bool elm_atspi_bridge_utils_reading_cb_call(Eo* obj);
//
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_atspi_bridge.eo.legacy.h"
#endif
#endif
