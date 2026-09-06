#include <stdint.h>
#include "fs.h"
#include "net.h"
#include "exec.h"
#include "mem.h"
#include "shell.h"

#define VGA_MEMORY ((volatile uint16_t*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static void clear_screen(void){for(uint32_t y=0;y<VGA_HEIGHT;++y)for(uint32_t x=0;x<VGA_WIDTH;++x)VGA_MEMORY[y*VGA_WIDTH+x]=0x0700|' ';}
static void print_at(const char*t,uint32_t r){uint32_t c=0;while(t[c]&&c<VGA_WIDTH){VGA_MEMORY[r*VGA_WIDTH+c]=0x0F00|(uint8_t)t[c];c++;}}
void kmain(uint32_t multiboot_info){
    (void)multiboot_info;clear_screen();print_at("Beta OS",1);print_at("x86_64 kernel online.",2);print_at("Initializing storage, memory, networking, userspace, and shell...",4);
    mem_init();fs_init();exec_init();net_init();
    const char release[]="NAME=Beta OS\nARCH=x86_64\n";
    if(!fs_find("/etc/os-release")) (void)fs_create("/etc/os-release",release,sizeof(release)-1);
    (void)exec_register("/bin/init",0,0);
    shell_run();for(;;)__asm__ volatile("hlt");
}
