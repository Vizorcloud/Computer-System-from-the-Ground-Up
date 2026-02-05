/* File: strings.c
 * ---------------
 * Purpose: Strings Module Library Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 5 2025
 */
#include "strings.h"
#include <stdbool.h>

void *memcpy(void *dst, const void *src, size_t n) {
    /* Copy contents from src to dst one byte at a time */
    char *d = dst;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

void *memset(void *dst, int val, size_t n) {
    unsigned char *ptr = dst;
    for (size_t i = 0; i < n; i++) {
        ptr[i] = (unsigned char)val;
    }
    return dst;
}

size_t strlen(const char *str) {
    /* Implementation a gift to you from lab3 */
    size_t n = 0;
    while (str[n] != '\0') {
        n++;
    }
    return n;
}

int strcmp(const char *s1, const char *s2) {
    int i = 0;

    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i]; 
        }
        i++;
    }
    
    return (unsigned char)s1[i] - (unsigned char)s2[i];
}

size_t strlcat(char *dst, const char *src, size_t dstsize) {
    size_t dstLength = 0;

    while (dstLength < dstsize && dst[dstLength] != '\0') {
        dstLength++;
    }

    if (dstLength == dstsize) {
        return dstsize + strlen(src);
    }

    size_t maxAppend = dstsize - dstLength - 1;
    size_t i = 0;

    for (; i < maxAppend && src[i] != '\0'; i++) {
        dst[dstLength + i] = src[i]; 
    }

    dst[dstLength + i] = '\0';

    return dstLength + strlen(src);
}

bool isDigit(unsigned char character) {
    return character >= '0' && character <= '9';
}

bool isAlpha(unsigned char character) {
    return (character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z');
}

bool isHex(unsigned char character) {
    return isDigit(character) || (character >= 'A' && character <= 'F') || (character >= 'a' && character <= 'f');
}

unsigned long toDigit(unsigned char character) {
    return character - '0';
}

unsigned char toUpper(unsigned char character) {
    if (character >= 'a' && character <= 'z') {
        return character - ('a' - 'A');  // convert to uppercase
    }
    return character;
}

unsigned long toHex(unsigned char character) {
    if (isDigit(character)) {
        return toDigit(character);
    }
    
    unsigned char upperChar = toUpper(character);
    return upperChar - 55;
}

unsigned long strtonum(const char *str, const char **endptr) {
    unsigned long returnNum = 0;
    unsigned char charIndex = 0;
    
    if (!isDigit(str[0])) {
        if (endptr != NULL) {
            *endptr = &str[charIndex];
        }
        return 0;
    } else if (str[0] == '0' && str[1] == 'x') { 
        charIndex = 2; // Skip the 0x
        while (str[charIndex] != '\0' && isHex(str[charIndex])) {
            returnNum = returnNum * 16 + toHex(str[charIndex]);          
            charIndex++;
        }
    } else {    
        while (str[charIndex] != '\0' && isDigit(str[charIndex])) {
            returnNum = returnNum * 10 + toDigit(str[charIndex]);          
            charIndex++;
        }
    }
    
    if (endptr != NULL) {
        *endptr = &str[charIndex];
    } 

    return returnNum;
}
