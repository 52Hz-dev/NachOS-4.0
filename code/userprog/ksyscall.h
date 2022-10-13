/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"



void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

// int SysReadNum()
// {
//   char c;
//   char buffer[255];
//   int n = 0;
//   do 
//   {
//     c = kernel->synchConsoleIn->GetChar();
//     if (c == ' ' || c == '\n' || c == EOF || c == 7 || c == 8 || c == 9) break;
//     buffer[n++] = c;    
//   }while(1);
//   if (strcmp(buffer, "2147483647") == 0 ) return 2147483647UL;
//   if (n > 11) return 0;
//   else if (n ==  10)
//   {
//     for (int i = 0; i < 10; i++)
//     {
//       if (buffer[i] > "2147483647"[i]) return 0;
//     }
//   }
//   else if (n == 11)
//   {
//     if (buffer[0] == '-')
//     {
//       for (int i = 0; i < 10; i++)
//       {
//         if (buffer[i + 1] > "2147483648"[i]) return 0;
//       }
//     }
//     else return 0;
//   }
//   int num = 0;
//   for (int i = 0; i < n; i++)
//   {
//     if (i == 0 && buffer[0] == '-') continue;
//     if ((buffer[i] < '0' || buffer[i] > '9'))
//     {
//       cerr << "Invalid. Try again\n";
//     }
//     else num = num * 10 + (buffer[i] - '0');
//   }
//   if (buffer[0] == '-') num *= -1;
//   return num;
// }





#endif /* ! __USERPROG_KSYSCALL_H__ */
