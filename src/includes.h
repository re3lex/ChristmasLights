#include <ArduinoOTA.h>
#include <WiFiManager.h>
#include "ESPAsyncWebServer.h"
#include <rom/rtc.h>

//local libs
#include <GyverButton.h>

//local header files
#include "utils/logger.h"
#include "wi_fi.h"
#include "index_page.h"
#include "utils.h"


// Display routines
#include "routines/getirl.h"
#include "routines/confetti_pal.h"
#include "routines/gradient_palettes.h"
#include "routines/juggle_pal.h"
#include "routines/matrix_pal.h"
#include "routines/noise16_pal.h"
#include "routines/noise8_pal.h"
#include "routines/one_sin_pal.h"
#include "routines/plasma.h"
#include "routines/rainbow_march.h"
#include "routines/rainbow_beat.h"
#include "routines/serendipitous_pal.h"
#include "routines/three_sin_pal.h"
#include "routines/two_sin.h"
#include "routines/blendwave.h"
#include "routines/fire.h"
#include "routines/candles.h"
#include "routines/colorwave.h"

#include "routine.h"

// Service logic
#include "web_server.h"
#include "OTA.h"
#include "tasks.h"