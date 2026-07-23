//
// Created by viski on 15.02.2025.
//

#ifndef MULTIRIUM_CDIALOG_H
#define MULTIRIUM_CDIALOG_H

#include <jni.h>
#include "raknet/NetworkTypes.h"

static class CDialog {
public:
    static jobject thiz;
    static jclass clazz;
    static bool bIsShow;

    static void show(int id, int style, char *caption, char *info, char *button1, char *button2);

    static void rpcShowPlayerDialog(RPCParameters *rpcParams);
};


#endif //MULTIRIUM_CDIALOG_H
