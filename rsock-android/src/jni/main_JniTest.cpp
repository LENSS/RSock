//
// Created by Chen on 2/5/18.
//

#include "main_JniTest.h"
#include "daemon/daemon.h"
#include <string.h>


JNIEXPORT jint JNICALL Java_main_JniTest_daemonRun(JNIEnv *env, jobject, jobjectArray stringArray) {

    int stringCount = env->GetArrayLength(stringArray);
    jint ret = 0;
    char** argv = NULL;

    if (stringCount > 0) {
        argv = new char*[stringCount];

        for (int i=0; i<stringCount; i++) {
            jstring string = (jstring) (env->GetObjectArrayElement(stringArray, i));
            const char *rawString = env->GetStringUTFChars(string, 0);
            argv[i] = new char[strlen(rawString)+1];
            strcpy(argv[i], rawString);

            // Don't forget to call `ReleaseStringUTFChars` when you're done.
            env->ReleaseStringUTFChars(string, rawString);
        }

        ret = (jint) daemon_main(stringCount, argv);

        // release argv;
        for (int i=0; i<stringCount; i++)
            delete[] argv[i];

        delete[] argv;
    }

    return ret;
}