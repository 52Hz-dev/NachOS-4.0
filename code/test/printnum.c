#include"syscall.h"
int main(){
    int num=ReadNum();
    PrintNum(num);
    Halt();
}