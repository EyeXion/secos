/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <segmem.h>
#include <intr.h>

extern info_t *info;


void infinite_loop_idt(){
  while (1){
    int n = 4 - 4;
    int i = 1/n;
    printf("%d\n",i);
  }
}


void bp_handler(){
  debug("############ Break point triggered ###############\n");
  
  //print eip before BP, eip is located 32 bits before the context
  uint32_t eip_bp;
  asm volatile("mov 4(%%ebp), %0" : "=r" (eip_bp) ::);
  printf("EIP when BP triggered : 0x%x\n", eip_bp);


  //We want to clear the stack frame of bp_handler and switch to frame of bp_trigger
  asm volatile("mov %%ebp, %%esp" :::);
  asm volatile("pop %%ebp" :::);

  //Then we switch to the return addr that is below the context (after the previous instruction, the ret addr of bp_handler in now at esp)
  //Since its a interruption hanlder, we use iret instead of ret (even if for now we do everything in ring0 so its useless)
  asm volatile("iret" :::);
}


void bp_trigger(){
  //Breakpoint interruption instruction
  asm volatile("int3");  

  printf("Message test question 3.7\n");
}


void set_bp_int_desc(){
 idt_reg_t idtr;
 get_idtr(idtr);
 int_desc_t * IDT = idtr.desc;
 offset_t isr = (offset_t)bp_handler;
 debug("ISR BP : 0x%x\n", isr);
 int_desc(&IDT[3],gdt_krn_seg_sel(1), isr);
}

void show_idt(){
  idt_reg_t idtr;
  get_idtr(idtr);
  printf("Start IDT : 0x%x - Size IDT : 0x%x\n", idtr.addr, idtr.limit +1);
  int nb_int_desc = 0;
  int_desc_t * end_idt = (int_desc_t *) (idtr.addr) + idtr.limit +1;
  for (int_desc_t * int_desc = idtr.desc; int_desc < end_idt; int_desc+= sizeof(int_desc_t)) {
    uint64_t offset = (int_desc->offset_1) | (int_desc->offset_2 << 16);
    printf("IDT entry at 0x%x - RAW : 0x%x\n", int_desc, int_desc->raw);
    printf("\tOFFSET : 0x%x - ", offset);
    printf("\tSELECTOR : 0x%x - ", int_desc->selector);
    printf("\tIST : 0x%x - ", int_desc->ist);
    printf("\tTYPE : 0x%x - ", int_desc->type);
    printf("\tDPL : 0x%x - ", int_desc->dpl);
    printf("\tP : 0x%x\n", int_desc->p);
    nb_int_desc++;
  }
  printf("nb_int_desc : %d\n",nb_int_desc);
}


void tp()
{
  //infinite_loop_idt();
  show_idt();
  set_bp_int_desc();
  //uint64_t * p = (uint64_t*) 0x8b;
  //printf("Ce qui se trouve en 0x8b : 0x%x\n", *p);
  bp_trigger();
}
