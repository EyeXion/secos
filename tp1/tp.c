/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <segmem.h>
#include <string.h>

#define NB_MAX_SEG_DESC 6

extern info_t *info;
extern uint32_t __kernel_end__;

seg_desc_t * gdt;

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

void init_seg_desc(uint64_t limit, uint64_t base, uint64_t type, uint64_t s, uint64_t dpl, uint64_t p, uint64_t d){
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
    gdt[i].g = d;

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
    uint32_t gdt_addr = (uint32_t) &__kernel_end__;
    gdt_addr = gdt_addr%(8*8) + gdt_addr;
    debug("Addr GDT : 0x%x\n", gdt_addr);
    gdt = (seg_desc_t *) gdt_addr;

    gdt_reg_t gdtr;
    gdtr.limit = 0;
    gdtr.desc = gdt;
    set_gdtr(gdtr);
}

void tp()
{

    //Question 1
    //En dessous, on retrieve les infos de la gdtr à la mano, mais y'a la struct donc ça sert à rien
  /*  uint32_t operand_get_gdtr;
    get_gdtr(operand_get_gdtr);
    debug("0x%x\n", operand_get_gdtr);
    gdt_reg_t * gdtr = (gdt_reg_t *) (operand_get_gdtr >> (8*2));
    uint16_t limit = operand_get_gdtr & 0x0000ffff;
    debug("GDTR Base addr : 0x%x\n", gdtr);
    debug("GDT limit : 0x%x\n", limit);  */

  show_gdt();

  printf("\n\t#### INIT OF OUR GDT #####\n\n");
  init_gdt();

  init_seg_desc(0xfff0,0,0,0,0,0,0); //1er segment null
  init_seg_desc(0xfffff,0,SEG_DESC_CODE_XR,1,0,1,1); //Segment code
  init_seg_desc(0xfffff,0,SEG_DESC_DATA_RW,1,0,1,1); //Segment data
  //Init registres de segements
  set_ss(gdt_krn_seg_sel(2));
  printf("%d\n", gdt_krn_seg_sel(2));
  set_ds(gdt_krn_seg_sel(2));
  set_es(gdt_krn_seg_sel(2));
  set_fs(gdt_krn_seg_sel(2));
  set_gs(gdt_krn_seg_sel(2));
  set_cs(gdt_krn_seg_sel(1));
  show_gdt();

  //Question 3
  init_seg_desc(31,0x600000,SEG_DESC_DATA_RW,1,0,1,1); //Segment data
  show_gdt();

  char  src[64];
  char *dst = 0;
  memset(src, 0xff, 64);
  set_es(gdt_krn_seg_sel(3));
  _memcpy8(dst, src, 32);
}
