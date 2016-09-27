
#ifndef _HELPER_H_
#define _HELPER_H_

#include <stdlib.h>


namespace MyHelper
{
    bool StrEqual( const char* str1, const char* str2 );
    // Include string terminator
    unsigned int GetStrLength( const char* str );
    unsigned int getIndexOfMax( const unsigned int* uintArray, const unsigned int length );
    void randomizeArray( unsigned int* arr, const unsigned int length, const unsigned numSwap );
}

#endif