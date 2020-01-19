
GButton btn(BTN_PIN);

void initRoutine()
{
  LEDS.setBrightness(max_bright); // Set the generic maximum brightness value.

#if LED_CK

  LEDS.addLeds<LED_CHIPSET, LED_PIN, LED_CK, LED_COLOR_ORDER>(leds, MAX_LEDS);

#else

  LEDS.addLeds<LED_CHIPSET, LED_PIN, LED_COLOR_ORDER>(leds, MAX_LEDS); //Для светодиодов WS2812B

#endif

  set_max_power_in_volts_and_milliamps(POWER_V, POWER_I); //Настройка блока питания

  random16_set_seed(4832); // Awesome randomizer of awesomeness
  random16_add_entropy(analogRead(2));

#if IR_ON == 1

  ledMode = EEPROM.read(STARTMODE);
  // Location 0 is the starting mode.
#if MAX_LEDS < 255
  NUM_LEDS = EEPROM.read(STRANDLEN); // Need to ensure NUM_LEDS < MAX_LEDS elsewhere.
#else
  NUM_LEDS = (uint16_t)EEPROM.read(STRANDLEN + 1) << 8 + // Need to ensure NUM_LEDS < MAX_LEDS elsewhere.
                                                         EEPROM.read(STRANDLEN);
#endif
  meshdelay = EEPROM.read(STRANDEL); // This is our notamesh delay for cool delays across strands.

  if ((EEPROM.read(ISINIT) != INITVAL) || // Check to see if Arduino has been initialized, and if not, do so.
      (NUM_LEDS > MAX_LEDS) ||
      ((ledMode > maxMode) && (ledMode != 100)))
  {
    EEPROM.write(STARTMODE, INITMODE); // Initialize the starting mode to 0.
#if MAX_LEDS < 255
    EEPROM.write(STRANDLEN, INITLEN); // Initialize the starting length to 20 LED's.
#else
    EEPROM.write(STRANDLEN, (uint16_t)(INITLEN)&0x00ff);   // Initialize the starting length to 20 LED's.
    EEPROM.write(STRANDLEN + 1, (uint16_t)(INITLEN) >> 8); // Initialize the starting length to 20 LED's.
#endif
    EEPROM.write(ISINIT, INITVAL);   // Initialize the starting value (so we know it's initialized) to INITVAL.
    EEPROM.write(STRANDEL, INITDEL); // Initialize the notamesh delay to 0.
    ledMode = INITMODE;
    NUM_LEDS = INITLEN;
    meshdelay = INITDEL;
  }
#else
  ledMode = INITMODE;
  NUM_LEDS = LED_NUM;
  meshdelay = INITDEL;
#endif

  PRINT(F("Initial delay: "));
  PRINT(meshdelay * 100);
  PRINTLN(F("ms delay."));
  PRINT(F("Initial strand length: "));
  PRINT(NUM_LEDS);
  PRINTLN(F(" LEDs"));

#if APP_BLACKSTART == 1
  solid = CRGB::Black; //Запуск с пустого поля
  newMode = ledMode;
  ledMode = 100;
  StepMode = 1;
#else
#ifdef APP_MY_MODE
  switch (demorun)
  {
  case 3:
    ledMode = pgm_read_byte(my_modes + current_mode);
    break; // демо 3
  case 4:
    ledMode = pgm_read_byte(my_modes + random8(my_mode_count));
    break; // демо 4
  }
#endif
#endif
  gCurrentPalette = gGradientPalettes[0];
  gTargetPalette = gGradientPalettes[0];
  strobe_mode(ledMode, 1);

  if (APP_DEMO_MODE)
  {
    PRINT(F("DEMO MODE "));
    PRINTLN(APP_DEMO_MODE);
  }
}

bool onFlag = true;

void changeMode(uint8_t mode)
{
  if (mode != newMode)
  {
    newMode = mode;
    modeChanged(newMode);
  }
}

void nextMode()
{
  uint8_t m = newMode;
#ifdef APP_MY_MODE
  if (++current_mode >= (my_mode_count - 1))
    current_mode = 0;
  m = pgm_read_byte(my_modes + current_mode);
#else
  if (++newMode >= maxMode)
    m = 0;
  ;
#endif
  SetMode(m);
}

void previousMode()
{
  uint8_t m = newMode;
#ifdef APP_MY_MODE
  if (current_mode <= 0)
  {
    current_mode = my_mode_count - 1;
  }
  else
  {
    current_mode = current_mode - 1;
  }
  m = pgm_read_byte(my_modes + current_mode);
#else
  if (--newMode <= 0)
    m = maxMode;
#endif
  SetMode(m);
}

void setBrightness(int v)
{
  max_bright = constrain(v, 0, 255);
  FastLED.setBrightness(max_bright);
}

void handleRoutine()
{
#if (BTN_USE == 1)
  static bool stepFlag = false;
  static bool brightDir = true;
  btn.tick();

  if (btn.isSingle())
  {
    PRINTLN("btn.isSingle");
    nextMode();
  }
  if (btn.isDouble())
  {
    PRINTLN("btn.isDouble");
    previousMode();
  }
  if (btn.isTriple())
  {
    onFlag = !onFlag;
    setBrightness(onFlag ? max_bright : 0);
    FastLED.show();
  }
  if (btn.hasClicks())
  {
    PRINTLN("btn.hasClicks");
    if (btn.getClicks() == 4)
      glitter = !glitter;
  }
  if (stepFlag && btn.isRelease())
  {
    stepFlag = false;
    brightDir = !brightDir;
  }
  if (btn.isStep())
  {
    stepFlag = true;
    max_bright += (brightDir ? 20 : -20);
    setBrightness(max_bright);
  }
#endif

#if (IR_ON == 1 || KEY_ON == 1 || BTN_USE == 1)
  getirl(); // Обработка команд с пульта и аналоговых кнопок
#endif

  if (onFlag)
  {
    demo_check(); // Работа если включен демонстрационный режим

    EVERY_N_MILLISECONDS(50)
    { // Плавный переход от одной палитры в другую
      uint8_t maxChanges = 24;
#if APP_CHANGE_ON == 1
      if (StepMode == MAX_LEDS)
#endif
        nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, maxChanges);
    }

#if APP_PALETTE_TIME > 0
    if (palchg)
    {
      EVERY_N_SECONDS(APP_PALETTE_TIME)
      { // Смена палитры
        if (palchg == 3)
        {
          if (gCurrentPaletteNumber < (gGradientPaletteCount - 1))
            gCurrentPaletteNumber++;
          else
            gCurrentPaletteNumber = 0;
          PRINT(F("New Palette: "));
          PRINTLN(gCurrentPaletteNumber);
        }
        gTargetPalette = gGradientPalettes[gCurrentPaletteNumber]; // We're just ensuring that the gTargetPalette WILL be assigned.
      }
    }
#endif

#if APP_DIRECT_TIME > 0
    EVERY_N_SECONDS(APP_DIRECT_TIME)
    { // Меняем направление
      thisdir = thisdir * -1;
    }
#endif

    EVERY_N_MILLIS_I(thistimer, thisdelay)
    {                                 // Sets the original delay time.
      thistimer.setPeriod(thisdelay); // This is how you update the delay value on the fly.
      KolLed = NUM_LEDS;              // Выводим Эффект на все светодиоды
      strobe_mode(ledMode, 0);        // отобразить эффект;
#if APP_CHANGE_ON == 1
      if (StepMode < NUM_LEDS)
      {                    // требуется наложить новый эффект
        KolLed = StepMode; // Выводим Эффект на все светодиоды
        if (StepMode > 10)
          strobe_mode(newMode, 0); // отобразить эффект;
#if APP_CHANGE_SPARK == 4
        sparkler(rand_spark);
#else
        sparkler(APP_CHANGE_SPARK);                        // бенгальский огонь
#endif
      }
#endif
    }

#if APP_CHANGE_ON == 1
    EVERY_N_MILLISECONDS(APP_CHANGE_TIME * 1000 / NUM_LEDS)
    { // Движение плавной смены эффектов
      if (StepMode < NUM_LEDS)
      {
        StepMode++;
        if (StepMode == 10)
          strobe_mode(newMode, 1);
        if (StepMode >= NUM_LEDS)
        {
          ledMode = newMode;
          StepMode = MAX_LEDS;
          PRINTLN(F("End SetMode"));
        }
        nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, NUM_LEDS);
      }
    }
#endif

    if (glitter)
      addglitter(10); // If the glitter flag is set, let's add some.
#if APP_CANDLE_KOL > 0
    if (candle)
      addcandle();
#endif

    if (background)
      addbackground(); // Включить заполнение черного цвета фоном
  }

  // Skip Key processing

#if (IR_ON == 1 || KEY_ON == 1 || BTN_USE == 1)
  if ((IR_Time_Mode > 0) && //Идет отчет времени
      ((millis() - IR_Time_Mode) >= 2000))
  { //И прошло больше 2 секунд
    IR_Time_Mode = 0;
    if (IR_New_Mode <= maxMode)
      SetMode(IR_New_Mode);
    IR_New_Mode = 0;
  }
#endif

#if IR_ON == 1
  while (!irrecv.isIdle())
    ; // if not idle, wait till complete

  if (irrecv.decode(&results))
  {
    /* respond to button */

    if (!Protocol)
    {
      Protocol = 1; // update the values to the newest valid input

#if IR_REPEAT == 1
      if (results.value != 0xffffffff) //Если не повтор то вставить новую команду
        Command = results.value;
      else
        Protocol = 2;
#else
      Command = results.value;
#endif
    }
    irrecv.resume(); // Set up IR to receive next value.
  }
#endif

  static uint32_t showTimer = 0;
  if (onFlag && millis() - showTimer >= 10)
  {
    showTimer = millis();
    FastLED.show();
  }
}

//-------------------OTHER ROUTINES----------------------------------------------------------
void strobe_mode(uint8_t mode, bool mc)
{ // mc stands for 'Mode Change', where mc = 0 is strobe the routine, while mc = 1 is change the routine

  if (mc)
  {
    fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0)); // Clean up the array for the first time through. Don't show display though, so you may have a smooth transition.
    PRINT(F("Mode: "));
    PRINTLN(mode);
    PRINTLN(millis());
#if PALETTE_TIME > 0
    if (palchg == 0)
      palchg = 3;
#else
    if (palchg == 0)
      palchg = 1;
#endif
  }

  switch (mode)
  { // First time through a new mode, so let's initialize the variables for a given display.

  case 0:
    if (mc)
    {
      thisdelay = 10;
      palchg = 0;
    }
    blendwave();
    break;
  case 1:
    if (mc)
    {
      thisdelay = 10;
      palchg = 0;
    }
    rainbow_beat();
    break;
  case 2:
    if (mc)
    {
      thisdelay = 10;
      allfreq = 2;
      thisspeed = 1;
      thatspeed = 2;
      thishue = 0;
      thathue = 128;
      thisdir = 1;
      thisrot = 1;
      thatrot = 1;
      thiscutoff = 128;
      thatcutoff = 192;
    }
    two_sin();
    break;
  case 3:
    if (mc)
    {
      thisdelay = 20;
      allfreq = 4;
      bgclr = 0;
      bgbri = 0;
      startindex = 64;
      thisinc = 2;
      thiscutoff = 224;
      thisphase = 0;
      thiscutoff = 224;
      thisrot = 0;
      thisspeed = 4;
      wavebright = 255;
    }
    one_sin_pal();
    break;
  case 4:
    if (mc)
    {
      thisdelay = 10;
    }
    noise8_pal();
    break;
  case 5:
    if (mc)
    {
      thisdelay = 10;
      allfreq = 4;
      thisspeed = -1;
      thatspeed = 0;
      thishue = 64;
      thathue = 192;
      thisdir = 1;
      thisrot = 0;
      thatrot = 0;
      thiscutoff = 64;
      thatcutoff = 192;
    }
    two_sin();
    break;
  case 6:
    if (mc)
    {
      thisdelay = 20;
      allfreq = 10;
      bgclr = 64;
      bgbri = 4;
      startindex = 64;
      thisinc = 2;
      thiscutoff = 224;
      thisphase = 0;
      thiscutoff = 224;
      thisrot = 0;
      thisspeed = 4;
      wavebright = 255;
    }
    one_sin_pal();
    break;
  case 7:
    if (mc)
    {
      thisdelay = 10;
      numdots = 2;
      thisfade = 16;
      thisbeat = 8;
      thisdiff = 64;
    }
    juggle_pal();
    break;
  case 8:
    if (mc)
    {
      thisdelay = 40;
      thisindex = 128;
      thisdir = 1;
      thisrot = 0;
      bgclr = 200;
      bgbri = 6;
    }
    matrix_pal();
    break;
  case 9:
    if (mc)
    {
      thisdelay = 10;
      allfreq = 6;
      thisspeed = 2;
      thatspeed = 3;
      thishue = 96;
      thathue = 224;
      thisdir = 1;
      thisrot = 0;
      thatrot = 0;
      thiscutoff = 64;
      thatcutoff = 64;
    }
    two_sin();
    break;
  case 10:
    if (mc)
    {
      thisdelay = 20;
      allfreq = 16;
      bgclr = 0;
      bgbri = 0;
      startindex = 64;
      thisinc = 2;
      thiscutoff = 224;
      thisphase = 0;
      thiscutoff = 224;
      thisrot = 0;
      thisspeed = 4;
      wavebright = 255;
    }
    one_sin_pal();
    break;
  case 11:
    if (mc)
    {
      thisdelay = 50;
      mul1 = 5;
      mul2 = 8;
      mul3 = 7;
    }
    three_sin_pal();
    break;
  case 12:
    if (mc)
    {
      thisdelay = 10;
    }
    serendipitous_pal();
    break;
  case 13:
    if (mc)
    {
      thisdelay = 20;
      allfreq = 8;
      bgclr = 0;
      bgbri = 4;
      startindex = 64;
      thisinc = 2;
      thiscutoff = 224;
      thisphase = 0;
      thiscutoff = 224;
      thisrot = 0;
      thisspeed = 4;
      wavebright = 255;
    }
    one_sin_pal();
    break;
  case 14:
    if (mc)
    {
      thisdelay = 10;
      allfreq = 20;
      thisspeed = 2;
      thatspeed = -1;
      thishue = 24;
      thathue = 180;
      thisdir = 1;
      thisrot = 0;
      thatrot = 1;
      thiscutoff = 64;
      thatcutoff = 128;
    }
    two_sin();
    break;
  case 15:
    if (mc)
    {
      thisdelay = 50;
      thisindex = 64;
      thisdir = -1;
      thisrot = 1;
      bgclr = 100;
      bgbri = 10;
    }
    matrix_pal();
    break;
  case 16:
    if (mc)
    {
      thisdelay = 10;
    }
    noise8_pal();
    break; // By: Andrew Tuline
  case 17:
    if (mc)
    {
      thisdelay = 10;
    }
    plasma(11, 23, 4, 18);
    break;
  case 18:
    if (mc)
    {
      thisdelay = 20;
      allfreq = 10;
      thisspeed = 1;
      thatspeed = -2;
      thishue = 48;
      thathue = 160;
      thisdir = -1;
      thisrot = 1;
      thatrot = -1;
      thiscutoff = 128;
      thatcutoff = 192;
    }
    two_sin();
    break;
  case 19:
    if (mc)
    {
      thisdelay = 50;
      palchg = 0;
      thisdir = 1;
      thisrot = 1;
      thisdiff = 1;
    }
    rainbow_march();
    break;
  case 20:
    if (mc)
    {
      thisdelay = 10;
      mul1 = 6;
      mul2 = 9;
      mul3 = 11;
    }
    three_sin_pal();
    break;
  case 21:
    if (mc)
    {
      thisdelay = 10;
      palchg = 0;
      thisdir = 1;
      thisrot = 2;
      thisdiff = 10;
    }
    rainbow_march();
    break;
  case 22:
    if (mc)
    {
      thisdelay = 20;
      palchg = 0;
      hxyinc = random16(1, 15);
      octaves = random16(1, 3);
      hue_octaves = random16(1, 5);
      hue_scale = random16(10, 50);
      x = random16();
      xscale = random16();
      hxy = random16();
      hue_time = random16();
      hue_speed = random16(1, 3);
      x_speed = random16(1, 30);
    }
    noise16_pal();
    break;
  case 23:
    if (mc)
    {
      thisdelay = 20;
      allfreq = 6;
      bgclr = 0;
      bgbri = 0;
      startindex = 64;
      thisinc = 2;
      thiscutoff = 224;
      thisphase = 0;
      thiscutoff = 224;
      thisrot = 0;
      thisspeed = 4;
      wavebright = 255;
    }
    one_sin_pal();
    break;
  case 24:
    if (mc)
    {
      thisdelay = 10;
    }
    plasma(23, 15, 6, 7);
    break;
  case 25:
    if (mc)
    {
      thisdelay = 20;
      thisinc = 1;
      thisfade = 2;
      thisdiff = 32;
    }
    confetti_pal();
    break;
  case 26:
    if (mc)
    {
      thisdelay = 10;
      thisspeed = 2;
      thatspeed = 3;
      thishue = 96;
      thathue = 224;
      thisdir = 1;
      thisrot = 1;
      thatrot = 2;
      thiscutoff = 128;
      thatcutoff = 64;
    }
    two_sin();
    break;
  case 27:
    if (mc)
    {
      thisdelay = 30;
      thisindex = 192;
      thisdir = -1;
      thisrot = 0;
      bgclr = 50;
      bgbri = 0;
    }
    matrix_pal();
    break;
  case 28:
    if (mc)
    {
      thisdelay = 20;
      allfreq = 20;
      bgclr = 0;
      bgbri = 0;
      startindex = 64;
      thisinc = 2;
      thiscutoff = 224;
      thisphase = 0;
      thiscutoff = 224;
      thisrot = 0;
      thisspeed = 4;
      wavebright = 255;
    }
    one_sin_pal();
    break;
  case 29:
    if (mc)
    {
      thisdelay = 20;
      thisinc = 2;
      thisfade = 8;
      thisdiff = 64;
    }
    confetti_pal();
    break;
  case 30:
    if (mc)
    {
      thisdelay = 10;
    }
    plasma(8, 7, 9, 13);
    break;
  case 31:
    if (mc)
    {
      thisdelay = 10;
      numdots = 4;
      thisfade = 32;
      thisbeat = 12;
      thisdiff = 20;
    }
    juggle_pal();
    break;
  case 32:
    if (mc)
    {
      thisdelay = 30;
      allfreq = 4;
      bgclr = 64;
      bgbri = 4;
      startindex = 64;
      thisinc = 2;
      thiscutoff = 224;
      thisphase = 0;
      thiscutoff = 128;
      thisrot = 1;
      thisspeed = 8;
      wavebright = 255;
    }
    one_sin_pal();
    break;
  case 33:
    if (mc)
    {
      thisdelay = 50;
      mul1 = 3;
      mul2 = 4;
      mul3 = 5;
    }
    three_sin_pal();
    break;
  case 34:
    if (mc)
    {
      thisdelay = 10;
      palchg = 0;
      thisdir = -1;
      thisrot = 1;
      thisdiff = 5;
    }
    rainbow_march();
    break;
  case 35:
    if (mc)
    {
      thisdelay = 10;
    }
    plasma(11, 17, 20, 23);
    break;
  case 36:
    if (mc)
    {
      thisdelay = 20;
      thisinc = 1;
      thisfade = 1;
    }
    confetti_pal();
    break;
  case 37:
    if (mc)
    {
      thisdelay = 20;
      palchg = 0;
      octaves = 1;
      hue_octaves = 2;
      hxy = 6000;
      x = 5000;
      xscale = 3000;
      hue_scale = 50;
      hue_speed = 15;
      x_speed = 100;
    }
    noise16_pal();
    break;
  case 38:
    if (mc)
    {
      thisdelay = 10;
    }
    noise8_pal();
    break;
  case 39:
    if (mc)
    {
      thisdelay = 10;
      palchg = 0;
    }
    fire();
    break;
  case 40:
    if (mc)
    {
      thisdelay = 10;
      palchg = 0;
    }
    candles();
    break;
  case 41:
    if (mc)
    {
      thisdelay = 10;
    }
    colorwaves();
    break;
  case 100:
    if (mc)
    {
      palchg = 0;
    }
    fill_solid(leds, NUM_LEDS, solid);
    break; //Установить цвет
  case 200:
    fill_solid(leds, MAX_LEDS, CRGB::Black);
    fill_solid(leds, NUM_LEDS, CRGB(255, 255, 255));
    break; //Зажеч гирлянду длинной NUM_LEDS (регулировка длинны гирлянды)
  case 201:
    fill_solid(leds, MAX_LEDS, CRGB::Black);
    fill_solid(leds, meshdelay, CRGB(255, 255, 255));
    break; //Зажеч гирлянду длинной meshdelay
  default:
    ledMode = 0;
    break; //нет такого режима принудительно ставим нулевой

  } // switch mode

  if (mc)
  {
    if (palchg == 0)
    {
      PRINTLN(F("Change palette off"));
    }
    else if (palchg == 1)
    {
      PRINTLN(F("Change palette Stop"));
    }
    else if (palchg == 3)
    {
      PRINTLN(F("Change palette ON"));
    }
  }

} // strobe_mode()

void demo_check()
{

  if (demorun)
  {
    if ((millis() - demo_time) >= (demo_duration * 1000L))
    {
      uint8_t mode = newMode;                                    //Наступило время смены эффекта
      demo_time = millis();                                      //Запомним время
      gCurrentPaletteNumber = random8(0, gGradientPaletteCount); //Случайно поменяем палитру
#if APP_CHANGE_ON == 1
      switch (demorun)
      {
      case 2:
        mode = random8(0, maxMode); // демо 2
        changeMode(mode);
        break;
#ifdef APP_MY_MODE
      case 3:
        if (current_mode >= (my_mode_count - 1))
          current_mode = 0; // демо 3
        else
          current_mode++;
        mode = pgm_read_byte(my_modes + current_mode);
        changeMode(mode);
        break;
      case 4:
        mode = pgm_read_byte(my_modes + random8(my_mode_count)); // демо 4
        changeMode(mode);
        break;
#endif
      default:
        if (newMode >= maxMode)
          changeMode(0); // демо 1
        else
        {
          mode = newMode + 1;
          changeMode(mode);
        }
        break;
      }
      StepMode = 1;
#if APP_CHANGE_SPARK == 4
      rand_spark = random8(3) + 1;
#endif

      PRINTLN(F("Start SetMode"));
#else
      gTargetPalette = gGradientPalettes[gCurrentPaletteNumber]; //Применим палитру
      PRINT(F("New Palette: "));
      PRINTLN(gCurrentPaletteNumber);
      switch (demorun)
      {
      case 2:
        ledMode = random8(0, maxMode); // демо 2
        break;
#ifdef APP_MY_MODE
      case 3:
        if (tek_my_mode >= (my_mode_count - 1))
          tek_my_mode = 0; // демо 3
        else
          tek_my_mode++;
        ledMode = pgm_read_byte(my_mode + tek_my_mode);
        break;
      case 4:
        ledMode = pgm_read_byte(my_mode + random8(my_mode_count)); // демо 4
        break;
#endif
      default:
        if (ledMode >= maxMode)
          ledMode = 0; // демо 1
        else
          ledMode++;
        break;
      }
      strobe_mode(ledMode, 1); // Does NOT reset to 0.
#if APP_CANDLE_KOL > 0
      PolCandle = random8(APP_CANDLE_KOL);
#endif
#endif
    } // if lastSecond

  } // if demorun

} // demo_check()