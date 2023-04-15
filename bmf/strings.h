#ifndef __STRING_H__
#define __STRING_H__

// *********************************************************************
// String stuff
// *********************************************************************
extern char toLower(char c);
extern int string_len(char* str);
extern char *string_copy(char *dest, char *src);
extern char *string_cat(char *dest, char *src);
extern void string_ccat(char *dest, char ch);
extern void string_padLeft(char *dest, int targetLen, char withChar);
extern void string_padRight(char *dest, int targetLen, char withChar);
extern void string_makeCounted(char *dest);
extern void string_makeZ(char *dest);
extern int string_equals(char *str1, char *str2);
extern int string_equals_nocase(char *str1, char *str2);
extern int string_isEmpty(char *str);
extern char *string_ltrim(char *str);
extern void string_rtrim(char *str);

#endif
