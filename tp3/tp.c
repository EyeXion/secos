/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <segmem.h>
#include <string.h>


#define NB_MAX_SEG_DESC 6
extern info_t *info;
seg_desc_t gdt[NB_MAX_SEG_DESC];
tss_t tss;

void show_gdt(){
    printf("\n\t####### NEW SHOW GDT######\n\n");
    gdt_reg_t grub_gdtr;
    get_gdtr(grub_gdtr);

    seg_desc_t * base_gdt = grub_gdtr.desc;
    offset_t end_gdt = grub_gdtr.addr + (grub_gdtr.limit + 1);


    for (seg_desc_t * desc_addr = base_gdt; desc_addr < (seg_desc_t *) end_gdt; desc_addr++){
        seg_desc_t seg = *(desc_addr);
        uint32_t base_seg = seg.base_1 | (seg.base_2 << 16) | (seg.base_3 << 24);
        uint32_t limit_seg = seg.limit_1 | (seg.limit_2 << 16);

        printf("### Seg desc located at 0x%x - RAW GDT : 0x%x\n", desc_addr, seg.raw);
        printf("\tBASE : 0x%x - ", base_seg);
        printf("LIMIT : 0x%x - ", limit_seg);
        printf("TYPE : 0x%x - ", seg.type);
        printf("S : 0x%x - ", seg.s);
        printf("DPL : 0x%x - ", seg.dpl);
        printf("P : 0x%x - ", seg.p);
        printf("AVL : 0x%x - ", seg.avl);
        printf("L : 0x%x - ", seg.l);
        printf("D : 0x%x - ", seg.d);
        printf("G : 0x%x\n", seg.g);
    }
}

void init_seg_desc(uint64_t limit, uint64_t base, uint64_t type, uint64_t s, uint64_t dpl, uint64_t p, uint64_t d, uint64_t g){
    int i;
    gdt_reg_t gdtr;
    get_gdtr(gdtr);
    if (gdtr.limit == 0){
        i = 0;
    }
    else{
        i = (gdtr.limit + 1)/sizeof(seg_desc_t);
    }
    gdt[i].raw = 0;
    printf("Index init seg : %d\n", i);
    gdt[i].base_1 = base;
    gdt[i].base_2 = base >> 16;
    gdt[i].base_3 = base >> 24;
    gdt[i].limit_1 = limit;
    gdt[i].limit_2 = limit >> 16;
    gdt[i].type = type;
    gdt[i].s = s;
    gdt[i].dpl = dpl;
    gdt[i].p = p;
    gdt[i].avl = 0;
    gdt[i].l = 0;
    gdt[i].d = d;
    gdt[i].g = g;

    if (i == 0){
        gdtr.limit = sizeof(seg_desc_t) - 1;
    }
    else{
        gdtr.limit = gdtr.limit + sizeof(seg_desc_t);
    }
    set_gdtr(gdtr);
}


void init_gdt(){
    // on doit calculer l'adresse de la GDT par rapport à la fin de l'espace du kernel, puis l'aligner sur 8 octets
    /* uint32_t gdt_addr = (uint32_t) &__kernel_end__;
    gdt_addr = gdt_addr%(8*8) + gdt_addr;
    debug("Addr GDT : 0x%x\n", gdt_addr);
    gdt = (seg_desc_t *) gdt_addr; */

    gdt_reg_t gdtr;
    gdtr.limit = 0;
    gdtr.desc = gdt;
    set_gdtr(gdtr);
}

void userland()
{
   asm volatile ("mov $0xaa, %eax");
}

void load_seg_regs(){
  //Question 3.1 - 3.2
  set_ds(gdt_usr_seg_sel(4));
  set_es(gdt_usr_seg_sel(4));
  set_fs(gdt_usr_seg_sel(4));
  set_gs(gdt_usr_seg_sel(4));
  //Question 3.2 - This crashes because we cant change it like that because we cannot set ss to a ring0 segment while cs (out current priv level) is still in ring3
  //set_ss(gdt_usr_seg_sel(4));
}

void jmp_to_userland(){
    //Question 3.3
   //fptr32_t ptr_user_fonc;
   //ptr_user_fonc.offset = (uint32_t)userland;
   //ptr_user_fonc.segment = (uint16_t)gdt_usr_seg_sel(3);
   // crash here because SS not set at the same time
   //set_cs(gdt_usr_seg_sel(3)); 
   //farjump(ptr_user_fonc);
   int ss_r3 = gdt_usr_seg_sel(4);
   int cs_r3 = gdt_usr_seg_sel(3);
   asm volatile("mov %esp, %eax"); //We put esp in eax
   asm volatile("push %0" :: "b"(ss_r3) :); // We push r3 ss
   asm volatile("push %eax"); //We push esp
   asm volatile("pushf"); // On push les eflags
   asm volatile("push %0" :: "b"(cs_r3) :); //On push cs r3
   asm volatile("push %0" :: "b"((offset_t)userland) :); //On push eip pour aller au userland 
   asm volatile("iret");
}


void init_tss(){
    tss.s0.esp = get_ebp();
    tss.s0.ss = gdt_krn_seg_sel(2);
    set_tr(gdt_krn_seg_sel(5));
}

void tp()
{
    init_gdt();
    init_seg_desc(0xfff0,0,0,0,0,0,0,0); //1er segment null Index0
    init_seg_desc(0xfffff,0,SEG_DESC_CODE_XR,1,0,1,1,1); //Segment code Ring 0 Index1
    init_seg_desc(0xfffff,0,SEG_DESC_DATA_RW,1,0,1,1,1); //Segment data Ring 0 Index2
    init_seg_desc(0xfffff,0,SEG_DESC_CODE_XR,1,3,1,1,1); //Segment Code Ring 3 Index3
    init_seg_desc(0xfffff,0,SEG_DESC_DATA_RW,1,3,1,1,1); //Segment data Ring 3 index4
    init_seg_desc(sizeof(tss_t),(offset_t)&tss,SEG_DESC_SYS_TSS_AVL_32,1,3,1,1,0); // Segment TSS Ring 0 Index 5
    show_gdt();
    load_seg_regs();
    tss.s0.esp = get_esp();
    tss.s0.ss = gdt_krn_seg_sel(2);
    //set_tr(gdt_usr_seg_sel(5));
    jmp_to_userland();
}
