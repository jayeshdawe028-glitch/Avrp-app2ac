//
// Created by viski on 02.06.2025.
//

#include "OsArray.h"

/*
void OSArray<int>::OSArray(OSArray<int>* thiz, const OSArray<int>* other)
{
    uint64_t numEntries; // x21
    int* v5;            // x0
    unsigned int v6;    // w25
    unsigned int i;     // w23
    int* dataPtr;       // x26
    uint64_t v9;         // x28
    int* v10;           // x0
    int* v11;           // x22
    int* v12;           // x21

    *(uint64_t*)&thiz->numAlloced = 0LL;
    thiz->dataPtr = 0LL;

    numEntries = other->numEntries;
    if ((uint32_t)numEntries)
    {
        v5 = (int*)malloc(4 * numEntries);
        this->numAlloced = numEntries;
        this->dataPtr = v5;

        if (other->numEntries)
        {
            v6 = 0;
            for (i = 1;; ++i)
            {
                dataPtr = other->dataPtr;
                if ((unsigned int)numEntries >= v6 + 1)
                {
                    v12 = thiz->dataPtr;
                }
                else
                {
                    v9 = 4 * (v6 + 1) / 3 + 3;
                    v10 = (int*)malloc(4 * v9);
                    v11 = thiz->dataPtr;
                    v12 = v10;

                    if (v11)
                    {
                        memcpy(v10, thiz->dataPtr, 4LL * v6);
                        free(v11);
                        v6 = thiz->numEntries;
                    }

                    thiz->numAlloced = v9;
                    thiz->dataPtr = v12;
                }

                v12[v6] = dataPtr[i - 1];
                v6 = thiz->numEntries + 1;
                thiz->numEntries = v6;

                if (i >= other->numEntries)
                    break;

                LODWORD(numEntries) = thiz->numAlloced;
            }
        }
    }
}*/