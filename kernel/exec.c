#include "exec.h"

static program_t programs[EXEC_MAX_PROGRAMS];
static size_t program_count;
static uint64_t u64(const uint8_t *p){return (uint64_t)p[0]|((uint64_t)p[1]<<8)|((uint64_t)p[2]<<16)|((uint64_t)p[3]<<24)|((uint64_t)p[4]<<32)|((uint64_t)p[5]<<40)|((uint64_t)p[6]<<48)|((uint64_t)p[7]<<56);}
static uint16_t u16(const uint8_t*p){return (uint16_t)p[0]|((uint16_t)p[1]<<8);}
void exec_init(void){program_count=0;}
int exec_register(const char*name,uintptr_t entry,size_t size){if(!name||program_count>=EXEC_MAX_PROGRAMS)return -1;programs[program_count]=(program_t){name,entry,size};++program_count;return 0;}
const program_t*exec_find(const char*name){for(size_t i=0;i<program_count;++i){const char*a=programs[i].name;size_t j=0;while(a[j]&&name[j]&&a[j]==name[j])++j;if(!a[j]&&!name[j])return &programs[i];}return 0;}
int exec_validate_elf64(const uint8_t*image,size_t size){
    if(!image||size<64||image[0]!=0x7f||image[1]!='E'||image[2]!='L'||image[3]!='F'||image[4]!=2||image[5]!=1)return 0;
    return u16(image+18)==0x3e && u16(image+16)==2 && u64(image+24)!=0;
}
int exec_load_elf64(const uint8_t*image,size_t size,uint8_t*dest,size_t dest_size,uintptr_t*entry){
    if(!exec_validate_elf64(image,size)||!dest||!entry)return -1;
    uint64_t phoff=u64(image+32),entry_addr=u64(image+24);uint16_t phentsz=u16(image+54),phnum=u16(image+56);
    if(phentsz<56||phnum>64||phoff>size||phnum>(size-phoff)/phentsz)return -2;
    uint64_t min=~0ull,max=0;int loadable=0;
    for(uint16_t i=0;i<phnum;++i){const uint8_t*p=image+phoff+(uint64_t)i*phentsz;uint32_t type=(uint32_t)u64(p);uint64_t off=u64(p+8),vaddr=u64(p+16),filesz=u64(p+32),memsz=u64(p+40);if(type!=1||!memsz)continue;if(off>size||filesz>size-off||filesz>memsz)return -3;if(vaddr<min)min=vaddr;if(vaddr+memsz< vaddr||vaddr+memsz>max)max=vaddr+memsz;loadable=1;}
    if(!loadable||max-min>dest_size)return -4;
    for(uint16_t i=0;i<phnum;++i){const uint8_t*p=image+phoff+(uint64_t)i*phentsz;if((uint32_t)u64(p)!=1)continue;uint64_t off=u64(p+8),vaddr=u64(p+16),filesz=u64(p+32),memsz=u64(p+40),d=vaddr-min;if(d+memsz>dest_size)return -5;for(uint64_t j=0;j<memsz;++j)dest[d+j]=(j<filesz)?image[off+j]:0;}
    *entry=(uintptr_t)(entry_addr-min);return 0;
}
