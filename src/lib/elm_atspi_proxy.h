#ifdef EFL_BETA_API_SUPPORT

enum _Elm_Atspi_Proxy_Type
{
   ELM_ATSPI_PROXY_TYPE_SOCKET,
   ELM_ATSPI_PROXY_TYPE_PLUG
};
typedef enum _Elm_Atspi_Proxy_Type Elm_Atspi_Proxy_Type;

#ifdef EFL_EO_API_SUPPORT
#include "elm_atspi_proxy.eo.h"
#endif
#endif
