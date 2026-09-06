#include "shell.h"
#include "fs.h"
#include "net.h"
#include "exec.h"
#include "mem.h"
#include <stdint.h>
#include <stddef.h>

#define VGA_MEMORY ((volatile uint16_t *)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint32_t row,col;
static inline uint8_t inb(uint16_t p){uint8_t v;__asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p));return v;}
static void putc(char c){if(c=='\n'){row=(row+1)%VGA_HEIGHT;col=0;return;}VGA_MEMORY[row*VGA_WIDTH+col]=0x0F00|(uint8_t)c;col=(col+1)%VGA_WIDTH;if(!col)row=(row+1)%VGA_HEIGHT;}
static void puts(const char*s){while(*s)putc(*s++);}
static size_t len(const char*s){size_t n=0;while(s[n])++n;return n;}
static int eq(const char*a,const char*b){size_t i=0;while(a[i]&&b[i]&&a[i]==b[i])++i;return a[i]==b[i];}
static const char*spaces(const char*s){while(*s==' ')++s;return s;}
static const char*word(const char*s,char*out,size_t n){size_t i=0;s=spaces(s);while(*s&&*s!=' '&&i+1<n)out[i++]=*s++;out[i]=0;return s;}
static char keymap(uint8_t s){static const char map[]="\0\0""1234567890-=""\0""qwertyuiop[]""\0""asdfghjkl;'`""\\""zxcvbnm,./";if(s<2||s>53)return 0;return map[s];}
static uint8_t keyboard_get(void){while(!(inb(0x64)&1u))__asm__ volatile("pause");return inb(0x60);}
static void clear_screen(void){for(uint32_t y=0;y<VGA_HEIGHT;++y)for(uint32_t x=0;x<VGA_WIDTH;++x)VGA_MEMORY[y*VGA_WIDTH+x]=0x0700|' ';row=col=0;}
static void print_file(const char*name){const fs_file_t*f=fs_find(name);if(!f){puts("cat: file not found");return;}for(size_t i=0;i<f->size;++i)putc((char)f->data[i]);}
static void command(const char*cmd){
    char op[16],arg[64];const char*p=word(cmd,op,sizeof(op));p=word(p,arg,sizeof(arg));p=spaces(p);size_t text_len=len(p);if(text_len>FS_DATA_MAX)text_len=FS_DATA_MAX;
    if(eq(op,"help"))puts("help clear uname ls cat write rm sync mem net pkg");
    else if(eq(op,"clear"))clear_screen();
    else if(eq(op,"uname"))puts("Beta OS x86_64");
    else if(eq(op,"ls")){char list[256];size_t n=fs_list(list,sizeof(list));if(!n)puts("<empty>");else puts(list);}
    else if(eq(op,"cat")){if(!arg[0])puts("usage: cat <file>");else print_file(arg);}
    else if(eq(op,"write")){if(!arg[0]||!p[0])puts("usage: write <file> <text>");else puts(fs_write(arg,p,text_len)==0?"write: ok":"write: failed");}
    else if(eq(op,"rm")){if(!arg[0])puts("usage: rm <file>");else puts(fs_remove(arg)==0?"rm: ok":"rm: failed");}
    else if(eq(op,"sync"))puts(fs_sync()==0?"filesystem: synced":"filesystem: not persistent");
    else if(eq(op,"mem")){puts("memory allocator: ");puts(mem_used()?"active":"ready");}
    else if(eq(op,"net"))puts(net_is_ready()?"network core: ready (loopback)":"network core: offline");
    else if(eq(op,"pkg")){puts("pkg: repository configuration loaded");puts("OpenBSD-current packages need a compatible userspace");}
    else if(!op[0])return;else puts("unknown command");
}
void shell_run(void){char buffer[80];size_t length=0;puts("Beta OS shell\nType 'help' for commands.\n\n");puts(fs_is_persistent()?"storage: persistent ATA disk\n":"storage: memory-only\n");puts("beta# ");for(;;){uint8_t s=keyboard_get();if(s&0x80u)continue;if(s==28){buffer[length]=0;command(buffer);putc('\n');length=0;puts("beta# ");}else if(s==14){if(length){--length;if(col){--col;putc(' ');--col;}}}else{char c=keymap(s);if(c&&length+1<sizeof(buffer)){buffer[length++]=c;putc(c);}}}}
