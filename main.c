#include <stdio.h>
#include <string.h>

#include <pspkernel.h>
#include <systemctrl_se.h>
#include <systemctrl.h>
#include <pspsysmem_kernel.h>

#include "memory.h"

/*----------------------------------------------------------------------------*/

PSP_MODULE_INFO("UserAgentFaker", PSP_MODULE_KERNEL, 0, 1 );

int module_start( SceSize arglen, void *argp );
int module_stop( void );

#define BUFFERSIZE (0x200 - sizeof(SceUID*))
char *user_agent;
STMOD_HANDLER previous;

void ClearCaches(void)
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

void Patch(SceModule2 *mod){
	u32 text_addr = mod->text_addr;
	u32 text_end = mod->text_addr + mod->text_size;
	
	for(; text_addr < text_end; text_addr += 4){
		if(_lw(text_addr) == 0xAE020004 && _lw(text_addr + 4) == 0xAFB40020){
			text_addr = text_addr - 0x10;
			break;
		}
	}
	
	if(text_addr < text_end){
		_sw(0x3C020000 | ((int)user_agent >> 16), text_addr);
		_sw(0x34420000 | ((int)user_agent & 0xFFFF), text_addr+4);
		ClearCaches();
	}
}

int OnModuleStart(SceModule2 *mod)
{
	if(strcmp(mod->modname, "htmlviewer_plugin_module") == 0){
		Patch(mod);
	}

	return previous ? previous(mod) : 0;
}


/*----------------------------------------------------------------------------*/

int module_start( SceSize arglen, void *argp ){
	user_agent = mem_alloc(BUFFERSIZE);
	strcpy(user_agent, argp);
	*(strrchr(user_agent, '/')+1) = '\0';
	strcat(user_agent, "UserAgentFaker.txt");
	SceUID fd = sceIoOpen(user_agent, PSP_O_RDONLY, 0777);
	if (fd < 0){
		sceIoClose(fd);
		return -1;
	}
	memset(user_agent, 0, BUFFERSIZE);
	int read_byte = sceIoRead(fd, user_agent, BUFFERSIZE);
	if( read_byte < 0 ) {
		sceIoClose(fd);
		return -1;
	}
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);
	
	return 0;
}

int module_stop( void ){
	return 0;
}
