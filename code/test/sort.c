#include "syscall.h"
int a[100];
main(){
    int i;
    int j;
    int type;
    int n;
    PrintString("Input n: ");
    n=ReadNum();
     if(n>100){
         PrintString("Invalid n.");
         Halt();
     }
     do{
     PrintString("1.Ascending\n2.Descending\n Your choose: ");
     type=ReadNum();
     if(type!=1&&type!=2){
         PrintString("Invalid type. Try again\n");
     }
     else{
         break;
     }
     }
     while(1);
    for(i=0;i<n;i++){
        PrintString("Input number: ");
        a[i]=ReadNum();
    }
    for (i = 0; i < n - 1; i++){
 
    // //      Last i elements are already in place

          for (j = 0; j < n - i - 1; j++)
             if (a[j] > a[j + 1]){
                  int tmp=a[j];
                  a[j]=a[j+1];
                  a[j+1]=tmp;
              }
       }
    if(type==1){  
        PrintString("Ascending array: ");        
     for(i=0;i<n;i++){
         PrintNum(a[i]);
     }
     PrintString("\n End.\n");
    }
    else{
        PrintString("Descending array: ");        
     for(i=n-1;i>=0;i--){
         PrintNum(a[i]);
     }
        PrintString("\n End.\n");
    }
     Halt();
}