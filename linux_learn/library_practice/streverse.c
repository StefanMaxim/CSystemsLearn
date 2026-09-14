#include "stlength.h"
#include<stdlib.h>

char* streverse(char* str)
{
    int len = stlength(str);
    if (len <= 0)
    {
        return str;
    }
    else{
        int i; //iterating variable
        char* rev; //the pointer to the reversed string, on heap

        rev = malloc((len + 1) * (sizeof(char))); //plus 1 for null terminator
        for (i = 0; i < len; i++)
        {
            rev[i] = str[len-i-1];
        }
        rev[len] = '\0'; // len, not len-1 bc length is not counting null term
        return rev;
    }

}