// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "openfile.h"
#include "filesys.h"

#define MaxFileLength 255
FileSystem *filesystem;
OpenFile *openfile;
// Input: - User space address (int)
// - Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space
char *User2System(int virtAddr, int limit)
{
	int i; // index
	int oneChar;
	char *kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; // need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);
	// printf("\n Filename u2s:");
	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		// printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}
// Input: - User space address (int)
// - Limit of buffer (int)
// - Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}
// Increase PC
void IncreasePC()
{
	int counter = kernel->machine->ReadRegister(PCReg);
	kernel->machine->WriteRegister(PrevPCReg, counter);
	counter = kernel->machine->ReadRegister(NextPCReg);
	kernel->machine->WriteRegister(PCReg, counter);
	kernel->machine->WriteRegister(NextPCReg, counter + 4);
}
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "\nReceived Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case NoException:
		return;
	case PageFaultException:
		DEBUG('a', "No valid translation found");
		SysHalt();
		IncreasePC();
		break;
	case ReadOnlyException:
		DEBUG('a', "Write attempted to page marked read-only");
		SysHalt();
		IncreasePC();
		break;
	case BusErrorException:
		DEBUG('a', "Translation resulted in an invalid physical address");
		SysHalt();
		IncreasePC();
		break;
	case AddressErrorException:
		DEBUG('a', "Unaligned reference or one that was beyond the end of the address space");
		SysHalt();
		IncreasePC();
		break;
	case OverflowException:
		DEBUG('a', "Integer overflow in add or sub");
		SysHalt();
		IncreasePC();
		break;
	case IllegalInstrException:
		DEBUG('a', "Unimplemented or reserved instr");
		SysHalt();
		IncreasePC();
		break;
	case NumExceptionTypes:
		DEBUG('a', "Number exception type");
		SysHalt();
		IncreasePC();
		break;
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
			SysHalt();
			ASSERTNOTREACHED();
			IncreasePC();
			break;
		case SC_Create:
		{
			int fileId;
			char *file_name;
			DEBUG(dbgSys, "\nCreate system call...");
			DEBUG('a', "\nReading virtual address of filename");
			// Lay tham so ten tap tin tu thanh ghi r4
			fileId = kernel->machine->ReadRegister(4);
			// MaxFileLength la 32
			file_name = User2System(fileId, MaxFileLength + 1);
			if (file_name == NULL)
			{
				DEBUG(dbgSys, "\n Not enough memory in system");
				kernel->machine->WriteRegister(2, -1); // tr ve loi cho nguoi dung
				delete file_name;
				IncreasePC();
				return;
			}
			DEBUG(dbgSys, "\n Finish reading filename");
			if (!filesystem->Create(file_name, 0))
			{
				kernel->machine->WriteRegister(2, -1);
				delete file_name;
				IncreasePC();
				return;
			}
			kernel->machine->WriteRegister(2, 1);
			delete file_name;
			IncreasePC();
			break;
		}
		case SC_Add:
		{
			DEBUG(dbgSys, "\nAdd system call");
			int num1=kernel->machine->ReadRegister(4);
			int num2=kernel->machine->ReadRegister(5);
			int result =num1+num2;
			kernel->machine->WriteRegister(2,result);
			IncreasePC();
			break;
		}
		case SC_Sub:
		{
			int op1;
			int op2;
			int res;
			op1 = kernel->machine->ReadRegister(4);
			op2 = kernel->machine->ReadRegister(5);
			res = op1 - op2;
			kernel->machine->WriteRegister(2, int(res));
			IncreasePC();
			break;
		}
		case SC_Exit:
		{
			DEBUG(dbgSys, "Exit");
			int val;
			val = kernel->machine->ReadRegister(4);
			kernel->currentThread->Finish();
			IncreasePC();
			SysHalt();
			break;
		}
		case SC_Remove:
		{
			DEBUG(dbgSys, "Remove system call.\n");
			int virAddr = kernel->machine->ReadRegister(4);
			char *filename = User2System(virAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				DEBUG(dbgAddr, "Not enough memory in system\n");
				kernel->machine->WriteRegister(2, -1); // return -1 to user program

				IncreasePC();
				break;
			}
			bool success = filesystem->Remove(filename);
			if (!success)
			{
				// Fail to remove file
				DEBUG(dbgAddr, "Can't remove file\n");
				kernel->machine->WriteRegister(2, -1); // return -1 to user program

				delete filename;
				IncreasePC();
				break;
			}
			else
			{
				// Success to remove file
				kernel->machine->WriteRegister(2, 1); // return 1 to user program
				delete filename;
				IncreasePC();
				break;
			}
			break;
		}
		case SC_Open:
		{
			DEBUG(dbgSys, "Open system call.\n");
			int fileId = kernel->machine->ReadRegister(4);
			char *file_name = User2System(fileId, MaxFileLength + 1);
			OpenFile *file_open = filesystem->Open(file_name);
			if (file_open == NULL)
			{
				DEBUG(dbgSys, "Can't open file.\n");
				kernel->machine->WriteRegister(2, -1);
			}
			else
			{
				DEBUG(dbgSys, "Open file successful.\n");
				kernel->machine->WriteRegister(2, int(fileId));
			}
			delete file_open;
			delete file_name;
			IncreasePC();
			break;
		}
		case SC_Read:
		{
			DEBUG(dbgSys, "\nRead system call");
			int bufferId; // buffer address
			int size;	  // Get file size
			int fileId;
			char *buffer;
			bufferId = kernel->machine->ReadRegister(4);
			size = kernel->machine->ReadRegister(5);
			fileId = kernel->machine->ReadRegister(6);
			char *file_name = User2System(fileId, MaxFileLength + 1);
			OpenFile *file_read = filesystem->Open(file_name);
			if (file_read == NULL)
			{
				DEBUG(dbgSys, "\nCan't open file");
				kernel->machine->WriteRegister(2, -1); // Return -1 for syscall.
				IncreasePC();
				break;
			}
			int numBytes;
			numBytes = file_read->Read(buffer, size);
			kernel->machine->WriteRegister(2, numBytes);
			if (numBytes < size)
			{
				DEBUG(dbgSys, "\nFile isn't long enough. Read:" << kernel->machine->ReadRegister(2));
			}
			DEBUG(dbgSys, "\n Read file successful :" << kernel->machine->ReadRegister(2));
			System2User(bufferId, numBytes, buffer); // Tra ve doan da doc cho user
			delete file_name;
			delete file_read;
			IncreasePC();
			break;
		}
		case SC_Write:
		{
			DEBUG(dbgSys, "\nWrite system call");
			int bufferId;
			int size;
			int fileId;
			bufferId = kernel->machine->ReadRegister(4);
			size = kernel->machine->ReadRegister(5);
			fileId = kernel->machine->ReadRegister(6);
			char *buffer = User2System(bufferId, size + 1);
			char *file_name = User2System(fileId, MaxFileLength + 1);
			OpenFile *file_write = filesystem->Open(file_name);
			if (file_write == NULL)
			{
				DEBUG(dbgSys, "\nCan't open file");
				kernel->machine->WriteRegister(2, -1);
			}
			else if (!file_write->Write(buffer, size))
			{
				DEBUG(dbgSys, "\nCan't write file");
				kernel->machine->WriteRegister(2, -1);
			}
			else
			{
				DEBUG(dbgSys, "\nWrite file successful");
				kernel->machine->WriteRegister(2, -1);
			}
			delete buffer;
			delete file_name;
			delete file_write;
			IncreasePC();
			break;
		}
		case SC_Seek:
		{
			DEBUG(dbgSys, "\nSeek system call");
			int pos = kernel->machine->ReadRegister(4);
			int fileId = kernel->machine->ReadRegister(5);
			char *file_name = User2System(fileId, MaxFileLength + 1);
			OpenFile *file_open = filesystem->Open(file_name);
			if (file_open == NULL)
			{
				DEBUG(dbgSys, "\nCan't open file");
				kernel->machine->WriteRegister(2, -1);
			}
			else if (file_open->Length() < pos)
			{
				DEBUG(dbgSys, "\nFile length is'n long enough");
				kernel->machine->WriteRegister(2, -1);
			}
			else
			{
				// file_open->Seek(pos);
				DEBUG(dbgSys, "\nSeek cuccessful");
				kernel->machine->WriteRegister(2, 1);
			}
			delete file_name;
			delete file_open;
			IncreasePC();
			break;
		}
		case SC_Close:
		{
			DEBUG(dbgSys, "\nClose system call");
			int fileId = kernel->machine->ReadRegister(4);
			char *file_name = User2System(fileId, MaxFileLength + 1);
			OpenFile *file_close = filesystem->Open(file_name);
			if (file_close == NULL)
			{
				DEBUG(dbgSys, "\nCan't open file");
				kernel->machine->WriteRegister(2, -1);
			}
			else
			{
				delete file_name;
				delete file_close;
				DEBUG(dbgSys, "\nClose file successful");
				kernel->machine->WriteRegister(2, 1);
			}
			IncreasePC();
			break;
		}
		case SC_ReadNum:
		{
			DEBUG(dbgSys, "\nRead_Num system call");
			char c;
			char buffer[255];
			int n = 0;
			c = kernel->GetConsole_Char();
			if (c == '-')
			{
				buffer[0] = '-';
				n++;
				c = kernel->GetConsole_Char();
			}
			do
			{
				if (c == '\n')
					break;
				if (c < '0' || c > '9')
				{
					DEBUG(dbgSys, "\nNot a number");
					kernel->machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				buffer[n] = c;
				c = kernel->GetConsole_Char();
				n++;
			} while (true);
			if(n>10){
				DEBUG(dbgSys,"\nOverflow");
				kernel->machine->WriteRegister(2,-1);
				IncreasePC();
				break;
			}
			long int num = 0;
			if (buffer[0] != '-')
			{
				for (int i = 0; i < n; i++)
				{
					num = num * 10 + (buffer[i] - '0');
				}
			}
			else
			{
				for (int i = 1; i < n; i++)
				{
					num = num * 10 + (buffer[i] - '0');
				}
				num *=-1;
			}
			if(num>2147483647 || num<-2147483647){
				DEBUG(dbgSys,"\nOverflow");
				kernel->machine->WriteRegister(2,-1);
				IncreasePC();
				break;
			}
			kernel->machine->WriteRegister(2, num);
			IncreasePC();
			break;
		}
		case SC_PrintNum:
		{
			DEBUG(dbgSys, "\nPrint_Num system call");
			char buffer[255];
			int n = 0;
			int Num = kernel->machine->ReadRegister(4);
			int tmp = Num;
			if (Num == 0)
			{
				kernel->PutConsole_Char('0');
				IncreasePC();
				break;
			}
			else
			{
				while (Num != 0)
				{
					buffer[n] = abs(Num % 10) + '0';
					n++;
					Num = Num / 10;
				}
				if (tmp < 0){
					buffer[n] = '-';
				for (int i = n; i >= 0; i--)
				{
					kernel->PutConsole_Char(buffer[i]);
				}
				}
				else{
						buffer[n] = '-';
				for (int i = n-1; i >= 0; i--)
				{
					kernel->PutConsole_Char(buffer[i]);
				}
				}
				kernel->machine->WriteRegister(2, tmp);
				IncreasePC();
				break;
			}
			IncreasePC();
			break;
		}
		case SC_ReadChar:
		{
			DEBUG(dbgSys, "\nRead_Char system call");
			char ch = kernel->GetConsole_Char();
			kernel->machine->WriteRegister(2, (char)ch);
			IncreasePC();
			break;
		}
		case SC_PrintChar:
		{
			DEBUG(dbgSys, "\nPrint_Char system call");
			kernel->PutConsole_Char(kernel->machine->ReadRegister(4));
			IncreasePC();
			break;
		}
		case SC_RandomNum:
		{
			DEBUG(dbgSys, "\nRandom_Num system call");
			srand(time(NULL));
			int num = rand();
			kernel->machine->WriteRegister(2, num);
			IncreasePC();
			break;
		}
		case SC_ReadString:
		{
			DEBUG(dbgSys, "\nRead_String system call");
			int virAddr = kernel->machine->ReadRegister(4);
			int size = kernel->machine->ReadRegister(5);
			char buffer[255];
			if (size > MaxFileLength)
			{
				DEBUG(dbgSys, "String length is invalid: " << kernel->machine->ReadRegister(5));
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				break;
			}
			for (int i = 0; i < size; i++)
			{
				buffer[i] = kernel->GetConsole_Char();
			}
			System2User(virAddr, size, buffer);
			kernel->machine->WriteRegister(2, 1);
			IncreasePC();
			break;
		}
		case SC_PrintString:
		{
			DEBUG(dbgSys, "\nPrint_String system call");
			int bufferId=kernel->machine->ReadRegister(4);
			char* buffer=User2System(bufferId,MaxFileLength+1);
			int size=0;
			while(buffer[size]!='\0'){
				size++;
			}
			for(int i=0;i<size;i++){
				kernel->PutConsole_Char(buffer[i]);
			}
			IncreasePC();
			delete buffer;
			break;
		}
			//  	char *buffer;
			// 	int len;
			// len=gSynchConsole->Read(buffer,256);
			// buffer= new char[255];
			// if (openf_id > i || openf_id < 0 || openf_id == 1) // go wrong <-- if try open `out of domain` fileSystem (10 openfile)
			// {												   // or try to read stdout
			// 	printf("Try to open invalib file");
			// 	kernel->machine->WriteRegister(2, -1);
			// 	break;
			// }

			// if (filesystem->openfile[openf_id] == NULL)
			// {
			// 	kernel->machine->WriteRegister(2, -1);
			// 	break;
			// }

			// char *buf = User2System(virtAddr, charcount);

			// if (openf_id == 0) // read from stdin
			// {
			// 	int sz = gSynchConsole->Read(buf, charcount);
			// 	System2User(virtAddr, sz, buf);
			// 	kernel->machine->WriteRegister(2, sz);

			// 	delete[] buf;
			// 	break;
			// }

			// int before = filesystem->openfile[openf_id]->GetCurrentPos();
			// if ((filesystem->openfile[openf_id]->Read(buf, charcount)) > 0)
			// {
			// 	// copy data from kernel to user space
			// 	int after = filesystem->openfile[openf_id]->GetCurrentPos();
			// 	System2User(virtAddr, charcount, buf);
			// 	kernel->machine->WriteRegister(2, after - before + 1); // after & before just used for returning
			// }
			// else
			// {
			// 	kernel->machine->WriteRegister(2, -1);
			// }
			// delete[] buf;
			// break;
			//  case SC_PrintInt:
			//  	int number;
			// 	int temp;
			// 	char c;
			// 	int countDigits,indexStart,Max_Size_Buffer;
			// 	int i;
			// 	countDigits=0;
			// 	indexStart=0;
			// 	Max_Size_Buffer=11;
			//  	number=machine->ReadRegister(4);
			//  	if(number==0){
			// 		putchar('0');
			//  	}
			// case SC_ReadInt:
			// 	long long res;
			// 	char c;
			// 	bool isNegative;
			// 	bool isEnd,flagZero;
			// 	isEnd=false;
			// 	isNegative=false;
			// 	flagZero=false;
			// 	res=0;
			// 	while((c=kernel->synchConsoleIn->GetChar())==' '){

			// 	}
			// 	if(c=='-')
			// 		isNegative=true;
			// 	else if(c>='0'&&c<='9')
			// 	res =res*10 +(c-'0');
			// 	else if(c=='\n'){
			// 		DEBUG(dbgSys,"\nNguoi dung chua nhap so");
			// 		printf("\nNguoi dung chua nhap so");
			// 		res=0;
			// 		kernel->machine->WriteRegister(2,int(res));
			// 		IncreasePC();
			// 		return;
			// 	}
			// 	else{
			// 		DEBUG(dbgSys,"\nSai so nguyen");
			// 		res=0;
			// 		kernel->machine->WriteRegister(2,int(res));
			// 		printf("Khong phai so nguyen");
			// 	}
			// 	while((c=kernel->synchConsoleIn->GetChar())=='\n'){
			// 		if(c<='9' && c>='0' && !isEnd){
			// 			res=res*10+(c-'0');
			// 			flagZero=true;
			// 		}
			// 		else if(c==' '){
			// 			isEnd=true;
			// 		}
			// 		else{
			// 			DEBUG(dbgSys,"\n Not interger");
			// 			res=0;
			// 			kernel->machine->WriteRegister(2,int(res));
			// 			IncreasePC();
			// 			break;
			// 		}
			// 		if(res>100){
			// 			DEBUG(dbgSys,"\nOverflow");
			// 			res=0;
			// 			kernel->machine->WriteRegister(2,int(res));
			// 			IncreasePC();
			// 			break;
			// 		}
			// 		if(isNegative) res=res*(-1);
			// 		kernel->machine->WriteRegister(2,int(res));
			// 		kernel->machine->WriteRegister(2,int(res));
			// 			IncreasePC();
			// 			break;
			// 	}
			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;

			ASSERTNOTREACHED();

			break;

		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}
	// ASSERTNOTREACHED();
}