#include "syscall.h"
int
main(){
    char buffer[255];
    ReadString(buffer,6);
    PrintString(buffer);
    Halt();
}