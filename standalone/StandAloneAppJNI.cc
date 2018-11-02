//
#include <jni.h>
#include <iostream>
#include "StandAloneApp.h"

using namespace std;

JNIEXPORT void JNICALL Java_StandAloneApp_sayHello(JNIEnv *env, jobject thisObj) {
  cout << "Hello World from C++!" << endl;
  return;
}
