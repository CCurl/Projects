#ifndef __CONFIG__
#define __CONFIG__

#ifdef _WIN32
  #define __WINDOWS__
  #define  _CRT_SECURE_NO_WARNINGS
  #include <Windows.h>
  #define __PC__
#endif

#ifdef __LINUX__
  #include <time.h>
  #define __PC__
#endif

#ifdef __PC__
  #define STK_SZ          16
  #define LSTACK_SZ       16
  #define USER_SZ        (64*1024)
  #define VARS_SZ        (64*1024)
#else
  /* Dev Board */
  #define _NEEDS_ALIGN_   1
  #define STK_SZ          8
  #define LSTACK_SZ       4
  #define USER_SZ        (16*1024)
  #define VARS_SZ        (4*1024)
  #define __SERIAL__      1
  #define mySerial        Serial
  #define ILED            13
#endif

#endif
