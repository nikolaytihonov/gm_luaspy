#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <stdint.h>
#include "GarrysMod/Lua/Interface.h"
#include "GarrysMod/Lua/LuaBase.h"

using namespace GarrysMod;
using namespace GarrysMod::Lua;

typedef struct
{
	HMODULE hMod;
	MODULEINFO iMod;
	uintptr_t ptr;
	uintptr_t offset;
} DLLFunction;

uintptr_t VPhysicalAddress(MODULEINFO iMod,uintptr_t func)
{
	uintptr_t base = (uintptr_t)iMod.lpBaseOfDll;
	return (uintptr_t)(func - base);
}

bool VFindModule(DLLFunction* func)
{
	HMODULE hMods[1024];
	DWORD cbNeeded;
	
	if( EnumProcessModules(GetCurrentProcess(), hMods, sizeof(hMods), &cbNeeded))
    {
        for ( int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
        {
			GetModuleInformation(GetCurrentProcess(),hMods[i],&func->iMod, sizeof(MODULEINFO));
			//Check if function in module space
			if( (func->offset = VPhysicalAddress(func->iMod,func->ptr) ) <= func->iMod.SizeOfImage && func->ptr >= (uintptr_t)func->iMod.lpBaseOfDll)
			{
				func->hMod = hMods[i];
				return true;
			}
		}
	}
	return false;
}

int FuncPtr(lua_State* state)
{
	LUA->CheckType(1,Type::FUNCTION);
	
	DLLFunction func;
	char dllpath[256];
	
	func.ptr = (uintptr_t)LUA->GetCFunction(1);
	
	if(VFindModule(&func))
	{
		LUA->CreateTable();
			LUA->PushUserdata((void*)func.ptr);
			LUA->SetField(-2,"ptr");
	
			LUA->PushUserdata((void*)func.offset);
			LUA->SetField(-2,"offset");
			
			LUA->CreateTable();
				LUA->PushUserdata((void*)func.iMod.lpBaseOfDll);
				LUA->SetField(-2,"base");
				
				LUA->PushUserdata((void*)func.iMod.SizeOfImage);
				LUA->SetField(-2,"size");
				
				GetModuleFileName(func.hMod,dllpath,256);
				LUA->PushString(dllpath);
				LUA->SetField(-2,"path");
			LUA->SetField(-2,"mod");
	}	
	else
	{
		LUA->PushNil();
	}
		
	return 1;
}

GMOD_MODULE_OPEN()
{
	LUA->PushSpecial(0);
		LUA->CreateTable();
			LUA->PushCFunction(FuncPtr);
			LUA->SetField(-2,"FuncPtr");
		LUA->SetField(-2,"luaspy");
	LUA->Pop(2);
	return 0;
}

GMOD_MODULE_CLOSE()
{
	LUA->PushNil();
	LUA->SetField(INDEX_GLOBAL,"luaspy");
	return 0;
}