; Beta OS x86_64 entry point.
; GRUB loads this ELF kernel through the Multiboot2 protocol.
; GRUB enters in 32-bit protected mode; we create paging and enter long mode.
BITS 32
section .multiboot
align 8
mb2_header:
    dd 0xE85250D6
    dd 0
    dd mb2_header_end - mb2_header
    dd -(0xE85250D6 + (mb2_header_end - mb2_header))
    dw 0
    dw 0
    dd 8
mb2_header_end:
section .text
align 16
global _start
extern kmain
_start:
    cli
    mov esp, stack_top
    mov eax, pml4_table
    mov cr3, eax
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    lgdt [gdt64.pointer]
    jmp 0x08:long_mode_entry
BITS 64
long_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov edi, ebx
    call kmain
.hang:
    cli
    hlt
    jmp .hang
section .rodata
align 8
gdt64:
    dq 0
    dq 0x00AF9A000000FFFF
    dq 0x00AF92000000FFFF
.pointer:
    dw $ - gdt64 - 1
    dq gdt64
section .bss
align 4096
pml4_table:
    dq pdpt_table | 0x003
    times 511 dq 0
pdpt_table:
    dq page_directory | 0x003
    times 511 dq 0
page_directory:
    dq 0x0000000000000083
    times 511 dq 0
align 16
stack_bottom:
    resb 16384
stack_top:
