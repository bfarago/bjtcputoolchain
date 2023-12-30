//
//  main.cpp
//  bjtemu
//
//  Created by Farago Barna on 2023. 01. 13..
//

#include <iostream>
#include "App.hpp"
#include "Version.h"
#define MAXPATHLENGTH (1023)
#ifdef _MSC_VER
#include <windows.h>
#undef main
#else
#include <unistd.h>
#endif
App app;
int main(int argc, const char * argv[]) {

    const char* fname=NULL; 
    if (argc>1){
        fname= argv[1];
    }else{

#ifdef _MSC_VER
        TCHAR buf[MAXPATHLENGTH];
        if (GetCurrentDirectory(MAXPATHLENGTH, buf)) {
            printf("cwd: %S\n", buf);
        }
#else
        char buf[MAXPATHLENGTH];
        getcwd(buf,MAXPATHLENGTH);
        printf("cwd: %s",buf);
#endif
        printf("%s\n\n", argv[0]);
        printf("bjtemu version %d.%d (build:%03d)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
        printf("Usage:\n bjtemu runnable.bin\n\n");
        printf("Keyboard:\n\t ESC: exit\n");
        printf("\t Numeric /: stop execution\n");
        printf("\t Numeric *: continue execution\n");
        printf("\t SHIFT+DEL: soft reset\n");
        printf("\t DEL: reload original RAM content and reset\n");
        printf("\t SPACE: joystick fire Cursor arrows: joystick direction switches.\n");
        fname="Scoperunner_7.bin";
    }
    app.load(fname);
    app.run();
    
    return EXIT_SUCCESS;
}
