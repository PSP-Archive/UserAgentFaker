/*
 * memory.c ver. 1.1
 *
 *  Created on: 2011/09/03
 *      Author: noname
 */

#include <pspkernel.h>
#include "memory.h"


/*---------------------------------------------------------------------------
  メモリ確保
    int size: 利用メモリサイズ

    return: 確保したメモリへのポインタ
            エラーの場合はNULLを返す
---------------------------------------------------------------------------*/
void* mem_alloc(int size)
{
  SceUID* p;
  int h_block;

  if(size == 0) return NULL;

  h_block = sceKernelAllocPartitionMemory(MEM_USER, "mem_alloc", PSP_SMEM_Low, size + sizeof(SceUID*), NULL);
  if(h_block < 0) return NULL;

  p = sceKernelGetBlockHeadAddr(h_block);
  *p = h_block;

  return (void *)(p + 1);
}

/*---------------------------------------------------------------------------
  メモリ解放
    *ptr: 確保したメモリへのポインタ
    return: エラーの場合は負の値を返す
---------------------------------------------------------------------------*/
s32 mem_free(void *ptr)
{
  if(ptr == NULL)
    return 0;

  return sceKernelFreePartitionMemory((SceUID)*((u32 *)ptr - 1));
}
