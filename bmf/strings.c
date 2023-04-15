#ifndef __STRING_H__
#define __STRING_H__

// *********************************************************************
// String stuff
// *********************************************************************
char toLower(char c) { return (('A'<=c) && (c<='Z')) ? (c + 32) : c; }

// *********************************************************************
int string_len(char* str)
{
    int len = 0;
    while (*(str++)) { ++len; }
    return len;
}

// *********************************************************************
char *string_copy(char *dest, char *src)
{
    while (*src) { *(dest++) = *(src++); }
    *dest = 0;
    return dest;
}

// *********************************************************************
char *string_cat(char *dest, char *src)
{
    char *dst = dest;
    while (*dst) { dst++; }
    *dst = 0;
    string_copy(dst, src);
    return dest;
}

// *********************************************************************
void string_ccat(char *dest, char ch)
{
    char src[20];
    src[0] = ch;
    src[1] = (char)0;
    string_cat(dest, src);
}

// *********************************************************************
void string_padLeft(char *dest, int targetLen, char withChar)
{
    int len = targetLen - string_len(dest);
    if (0 < len) {
        char tmp[128];
        for (int i = 0; i < len; i++) {
            tmp[i] = withChar;
        }
        tmp[len] = (char)0;
        string_cat(tmp, dest);
        string_copy(dest, tmp);
    }
}

// *********************************************************************
void string_padRight(char *dest, int targetLen, char withChar)
{
    int len = string_len(dest);
    while (len < targetLen) {
        string_ccat(dest, withChar);
        ++len;
    }
}

// *********************************************************************
void string_makeCounted(char *dest)
{
    char tmp[128], *cp = tmp;
    *(cp++) = (char)string_len(dest);
    string_copy(cp, dest);
    string_copy(dest, tmp);
}

// *********************************************************************
void string_makeZ(char *dest)
{
    char *cp = dest, *cp1 = cp+1;
    int len = *cp;
    while (len > 0)
    {
        *(cp++) = *(cp1++);
        --len;
    }
    *cp = 0;
}

// *********************************************************************
int string_equals(char *str1, char *str2)
{
    while (*(str1++) == *(str2++)) {
        if (*(str1-1) == 0) { return 1; }
    }
    return 0;
}

// *********************************************************************
int string_equals_nocase(char *str1, char *str2) {
    while (toLower(*(str1++)) == toLower(*(str2++))) {
        if (*(str1-1) == 0) { return 1; }
    }
    return 0;
}

// *********************************************************************
int string_isEmpty(char *str) {
    return (*str == 0) ? 1 : 0;
}

// *********************************************************************
char *string_ltrim(char *str)
{
    while ((*str) && (*str <= ' ')) { ++str; }
    return str;
}

// *********************************************************************
void string_rtrim(char *str)
{
    char *cp = str + string_len(str) - 1;
    while ((str <= cp) && (*cp < ' ')) { *(cp--) = 0; }
}


#endif
