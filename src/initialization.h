#define qsubd(x, b) ((x > b) ? wavebright : 0) // A digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b) ((x > b) ? x - b : 0)      // Unsigned subtraction macro. if result <0, then => 0.

#define NOTAMESH_VERSION 103 // Just a continuation of seirlight and previously aalight.

#include "FastLED.h" // https://github.com/FastLED/FastLED
//#include "commands.h" // The commands.

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#if (BTN_USE == 1)
uint8_t IR_New_Mode = 0;   //Выбор эффекта
uint32_t IR_Time_Mode = 0; //время последнего нажатия
#endif


// Fixed definitions cannot change on the fly.
#define MAX_LEDS LED_NUM


// Initialize changeable global variables.
#if MAX_LEDS < 255
uint8_t NUM_LEDS; // Number of LED's we're actually using, and we can change this only the fly for the strand length.
uint8_t KolLed;
#else
uint16_t NUM_LEDS; // Number of LED's we're actually using, and we can change this only the fly for the strand length.
uint16_t KolLed;
#endif

int max_bright = 255; // Overall brightness definition. It can be changed on the fly.

struct CRGB leds[MAX_LEDS]; // Initialize our LED array. We'll be using less in operation.

CRGBPalette16 gCurrentPalette; // Use palettes instead of direct CHSV or CRGB assignments
CRGBPalette16 gTargetPalette;  // Also support smooth palette transitioning
CRGB solid = CRGB::Black;      // Цвет закраски

extern const TProgmemRGBGradientPalettePtr gGradientPalettes[]; // These are for the fixed palettes in gradient_palettes.h
extern const uint8_t gGradientPaletteCount;                     // Total number of fixed palettes to display.
uint8_t gCurrentPaletteNumber = 0;                              // Current palette number from the 'playlist' of color palettes
uint8_t currentPatternIndex = 0;                                // Index number of which pattern is current
uint32_t demo_time = 0;                                         // время демо режима

TBlendType currentBlending = LINEARBLEND; // NOBLEND or LINEARBLEND

// EEPROM location definitions.
#define STARTMODE 0 // EEPROM location for the starting mode.
#define STRANDLEN 1 // EEPROM location for the actual Length of the strand, which is < MAX_LEDS
#define STRANDEL 3  // EEPROM location for the mesh delay value.
#define ISINIT 4    // EEPROM location used to verify that this Arduino has been initialized

#define INITVAL 0x55    // If this is the value in ISINIT, then the Arduino has been initialized. Startmode should be 0 and strandlength should be
#define INITMODE 0      // Startmode is 0, which is black.
#define INITLEN LED_NUM // Start length LED's.
#define INITDEL 0       // Starting mesh delay value of the strand in milliseconds.

uint16_t meshdelay; // Timer for the notamesh. Works with INITDEL.

uint8_t ledMode = 0; // номер текущего режима
#if APP_CHANGE_ON == 1
uint8_t newMode = 0; // номер нового режима
#if MAX_LEDS < 255
uint8_t StepMode = MAX_LEDS; // Текущий шаг перехода от нового режима до старого
#else
uint16_t StepMode = MAX_LEDS; // Текущий шаг перехода от нового режима до старого
#endif
#endif

uint8_t demo_duration = APP_DEMO_TIME; //in seconds!
uint8_t demorun = APP_DEMO_MODE; // 0 = regular mode, 1 = demo mode, 2 = shuffle mode.
#define maxMode 41           // Maximum number of modes.

uint8_t Protocol = 0; // Temporary variables to save latest IR input
uint32_t Command = 0;

// Общие переменные ----------------------------------------------------------------------
uint8_t allfreq = 32;        // Меняет частоту. Переменная для эффектов one_sin_pal и two_sin.
uint8_t bgclr = 0;           // Общий цвет фона. Переменная для эффектов matrix_pal и one_sin_pal.
uint8_t bgbri = 0;           // Общая фоновая яркость. Переменная для эффектов matrix_pal и one_sin_pal.
bool glitter = APP_GLITER_ON; // Флаг включения блеска
bool background = APP_BACKGR_ON; // Флаг включения заполнения фона

#if APP_CANDLE_KOL > 0
bool candle = APP_CANDLE_ON; // Флаг включения свечей
#endif
uint8_t palchg = 3;       // Управление палитрой  3 - менять палитру автоматически иначе нет
uint8_t startindex = 0;   // С какого цвета начинать. Переменная для эффектов one_sin_pal.
uint8_t thisbeat;         // Переменная для эффектов juggle_pal.
uint8_t thiscutoff = 192; // Если яркость ниже этой, то яркость = 0. Переменная для эффектов one_sin_pal и two_sin.
uint16_t thisdelay = 0;   // Задержка delay
uint8_t thisdiff = 1;     // Шаг изменения палитры. Переменная для эффектов confetti_pal, juggle_pal и rainbow_march.
int8_t thisdir = 1;       // Направление движения эффекта. принимает значение -1 или 1.
uint8_t thisfade = 224;   // Скорость затухания. Переменная для эффектов confetti_pal и juggle_pal.
uint8_t thishue = 0;      // Оттенок Переменная для эффектов two_sin.
uint8_t thisindex = 0;    // Указатель ан элемент палитры
uint8_t thisinc = 1;      // Изменение начального цвета после каждого прохода. Переменная для эффектов confetti_pal и one_sin_pal.
int thisphase = 0;        // Изменение фазы. Переменная для эффектов one_sin_pal, plasma и two_sin.
uint8_t thisrot = 1;      // Измение скорости вращения волны. В настоящее время 0.
int8_t thisspeed = 4;     // Изменение стандартной скорости
uint8_t wavebright = 255; // Вы можете изменить яркость волн / полос, катящихся по экрану.

#ifdef APP_MY_MODE
const PROGMEM uint8_t my_modes[] = {APP_MY_MODE}; //массив выбранных режимов
const uint8_t my_mode_count = sizeof(my_modes); //колличество выбрано режимов
uint8_t current_mode = 0;                       //Указатель на текущий режим
#endif

#if APP_CHANGE_SPARK == 4
uint8_t rand_spark = 0;
#endif

long summ = 0;

void strobe_mode(uint8_t newMode, bool mc);
void bootme();
void meshwait();
void getirl();
void demo_check();

#if APP_CANDLE_KOL > 0
////////////////////////////////////////////////////////////// Свечи
DEFINE_GRADIENT_PALETTE(candle_Pal){
    0, 255, 0, 0,        //red
    90, 255, 255, 255,   //full white
    180, 255, 255, 0,    //bright yellow
    255, 255, 255, 255}; //full white

uint8_t PolCandle = 1; //Положение свечи

void addcandle()
{
  uint16_t poz = PolCandle;
  CRGBPalette16 myPal = candle_Pal;

  if (NUM_LEDS > 5)
  {
    uint8_t kol = NUM_LEDS / 10; //Количество свечей

    for (uint8_t x = 0; x < kol; x++)
    {
      leds[poz] = ColorFromPalette(myPal, random8(255));
      poz += APP_CANDLE_KOL;
    }
  }
}
#endif

///////////////////////////////////////////////////////////////// Черный фон
void addbackground()
{
#if MAX_LEDS < 255
  uint8_t i;
#else
  uint16_t i;
#endif

  for (i = 0; i < NUM_LEDS; i++)
    if ((leds[i].r < 5) &&
        (leds[i].g < 5) &&
        (leds[i].b < 5))
      leds[i].b += CRGB(5, 5, 5);
}

////////////////////////////////////////////////////////////////// Блеск
void addglitter(fract8 chanceOfGlitter)
{
  if (random8() < chanceOfGlitter)
  {
#if MAX_LEDS < 255
    leds[random8(NUM_LEDS)] += CRGB::White;
#else
    leds[random16(NUM_LEDS)] += CRGB::White;
#endif
  }
}

////////////////////////////////////////////////////////////////// Бенгальский огонь
void sparkler(uint8_t n) //Бенгальский огонь
// 0 - нет эффектов
// 1 - Бенгальский огонь
// 2 - 1 яркий светодиод
// 3 - метеорит
// 4 - случайный эффект
{
  uint8_t kol = 3;

  if (KolLed < 10)
    kol = KolLed / 3;
  if (kol >= 2)
  {
    uint8_t nn = n;
    switch (nn)
    {
    case 1:
      for (uint8_t x = 0; x < kol; x++)
        leds[KolLed - random8(kol * 2)] = CRGB::White;
      break; //Бенгальский
    case 2:
      leds[KolLed - 1] = CRGB::White;
      break; //1 яркий
    case 3:
      leds[KolLed] = CRGB::White;
      leds[KolLed - 1] = CRGB::Red;
      leds[KolLed - 2] = CRGB::Violet;
      break; //Метеорит
    }
  }
}
