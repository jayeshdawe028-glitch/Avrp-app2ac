//
// Created by viski on 02.06.2025.
//

#ifndef MULTIRIUM_OSARRAY_H
#define MULTIRIUM_OSARRAY_H

#include "main.h"

template <typename T>
class OSArray {
public:
    unsigned int numAlloced;
    unsigned int numEntries;
    T *dataPtr;

    void Clear() {
        if (dataPtr) {
            free(dataPtr);
            dataPtr = nullptr;
        }
        numEntries = 0;
        numAlloced = 0;
    }

    OSArray() : numAlloced(0), numEntries(0), dataPtr(nullptr) {}

    OSArray(const OSArray<T>& other) {
        numAlloced = other.numEntries;
        numEntries = 0;
        dataPtr = nullptr;

        if (numAlloced > 0) {
            dataPtr = (T*)malloc(sizeof(T) * numAlloced);
            for (unsigned int i = 0; i < other.numEntries; ++i) {
                if (numEntries >= numAlloced) {
                    unsigned int newAlloced = (numEntries * 4) / 3 + 3;
                    T* newData = (T*)malloc(sizeof(T) * newAlloced);
                    memcpy(newData, dataPtr, sizeof(T) * numEntries);
                    free(dataPtr);
                    dataPtr = newData;
                    numAlloced = newAlloced;
                }
                dataPtr[numEntries++] = other.dataPtr[i];
            }
        }
    }

    ~OSArray() {
        if (dataPtr) free(dataPtr);
    }
};
VALIDATE_SIZE(OSArray<int>, (VER_x32 ? 0xC : 0x10));


#endif //MULTIRIUM_OSARRAY_H
