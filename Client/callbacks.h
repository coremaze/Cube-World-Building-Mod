#ifndef CALLBACKS_H
#define CALLBACKS_H

#define RegisterCallback(callbackRegistrarName, handlerFunction) \
        {\
        typedef void (*RegisterCallback_t)( ... );\
        auto RegisterCallback = (RegisterCallback_t)GetProcAddress(LoadLibraryA("CallbackManager.dll"), callbackRegistrarName);\
        RegisterCallback(handlerFunction);\
        }


#endif
