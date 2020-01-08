#if PRINT_DEBUG
#define PRINT(s)       \
  {                     \
    Serial.print(s); \
  }
#define PRINTLN(...)             \
  {                              \
    Serial.println(__VA_ARGS__); \
  }
#define PRINTF(...)          \
  {                             \
    Serial.printf(__VA_ARGS__); \
  }
#else
#define PRINTS(s)
#define PRINT(s, v)
#define PRINTX(s, x)
#endif