/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <cr.h>
#include <pagemem.h>

extern info_t *info;
pde32_t * PGD1 = (pde32_t*)0x600000;
pte32_t * PTB0  = (pte32_t*)0x601000;
pte32_t * PTB1 = (pte32_t*)0x602000;
pte32_t * PTB2_looptoPGD = (pte32_t*)0x603000;

void set_id_mapping(){
  //We set the first PTB with ID id_mapping (1024 entries per PTB). Kernel code mainly
  for (int i = 0; i<1024; i++){
    pg_set_entry(&PTB0[i], (PG_RW | PG_KRN), i); //We put i as offset, but if 1 = 1; ofsset = 4KO cuz alignement on 4KO (but automatic cuz 1 will be interpreted as 1 << 12)
  }

  //Set PTB1 as first entry of PGD1 (we dont use the 12 LSB cuz 4KO alignement)
  pg_set_entry(&PGD1[0], (PG_RW | PG_KRN), (int)PTB0 >> 12);


  //We set the second PTB in order to answer question 5 (access adress 0x601000)
  for (int i = 0; i <1024; i++){
    pg_set_entry(&PTB1[i], (PG_RW | PG_KRN), i + 1024*1); //1024 * 1 cuz second PTB, so offsets are 4KO further than 0
  }

  pg_set_entry(&PGD1[1], (PG_RW | PG_KRN), (int)PTB1 >> 12);


  //Question 7

  //We want to have the adress 0xc0000000 to be mapped to the first entry of the PGD1-
  int index_pte_loop = (0xc0000000)>>12 & 0x3FF; // We take the bits related to pte index by shifting 12 bits to get rid of the offset in a page and mask to get rid of pdg index
  int index_pde_loop = (0xc0000000)>>(12 + 10) & 0x3FF; // We take the 10 MSB that are the ones we use to navigate in the PDG and mask to keep 10 bits only.

  pg_set_entry(&PTB2_looptoPGD[index_pte_loop], (PG_RW | PG_KRN), (int)PGD1>>12);
  pg_set_entry(&PGD1[index_pde_loop], (PG_RW | PG_KRN), (int)PTB2_looptoPGD>>12);
  
  //Activate paging
  set_cr0(get_cr0() | CR0_PG);

  debug("FIRST ENTRY OF PGD : 0x%x\n", PGD1[0].raw);
  debug("FIRST ENTRY OF PGD WITH LOOP : 0x%x\n", ((pde32_t*)(0xc0000000))->raw);


  //Question 8

  //We compute the indexes (offset) in PTE1 that are linked to virt addr 0x700000 and 0x7ff000
  int index_pte1 = (0x700000) >>12 & 0x3FF;
  int index_pte2 = (0x7ff000) >> 12 & 0x3FF;

  pg_set_entry(&PTB1[index_pte1], (PG_RW | PG_KRN), 0x2000>>12); //Like usual 12 LSB not taken into account
  pg_set_entry(&PTB1[index_pte2], (PG_RW | PG_KRN), 0x2000>>12);

  debug("STRING AT 0x700000 virt addr : %s\n", (char *)0x700000);
  debug("STRING AT 0x7ff000 virt addr : %s\n", (char *)0x7ff000);


  //Question 9
  PGD1[0].raw = 0;
  debug("TEST READ STRING AFTER DELETING PGD FIRST ENTRY : %s\n", (char *)0x7ff000);
  

}

void tp()
{
  //Question 1
  debug("CR3 : 0x%x\n", get_cr3());

  //Question2
  debug("RAW PGD1 : 0x%x\n",PGD1->raw);
  set_cr3((uint32_t)0x600000);
  
  //Question 3
  //set_cr0(get_cr0() | CR0_PG);
  
  //Question 4 - 8
  set_id_mapping();
  debug("FIRST PTB0 ENTRY : 0x%x\n", PTB0[0].raw);
} 
