#ifndef __CONFIG__
#define __CONFIG__

#define PC          1
#define DEV         2

#define __BOARD__   DEV

#ifdef _WIN32
  #define __WINDOWS__
  #define  _CRT_SECURE_NO_WARNINGS
  #include <Windows.h>
  #include <conio.h>
  #undef __BOARD__
  #define __BOARD__ PC
  // For Windows 64-bit, use 'long long'
  // #define S3CELL long long
#endif

#ifdef __LINUX__
  #include <time.h>
  #undef __BOARD__
  #define __BOARD__ PC
#endif

#if __BOARD__ == PC
  #define STK_SZ          64
  #define LSTACK_SZ        8
  #define USER_SZ        (16*1024)
#else
  #include <Arduino.h>
  #define _NEEDS_ALIGN_    1
  #define __SERIAL__       1
  #define mySerial         Serial
  #define STK_SZ           7
  #define LSTACK_SZ        3
  #define USER_SZ        (1*1024)
#endif

#endif // __CONFIG__
