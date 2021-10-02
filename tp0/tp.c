/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

extern info_t *info;
extern uint32_t __kernel_start__;
extern uint32_t __kernel_end__;

void tp() {
  debug("kernel mem [0x%x - 0x%x]\n", &__kernel_start__, &__kernel_end__);
  debug("MBI flags 0x%x\n", info->mbi->flags);

  /* ######## QUESTION 2 ######## */

  multiboot_info_t *mbi = info->mbi; // Extract mbi from info
  multiboot_memory_map_t *mmap;

  printf("mmap_addr = 0x%x, mmap_length = 0x%x\n", (unsigned)mbi->mmap_addr,
         (unsigned)mbi->mmap_length);

  // Loop to iterate over all the multiboot_memory_map_t objects and print their
  // info
  for (mmap = (multiboot_memory_map_t *)mbi->mmap_addr;
       (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length; mmap++) {
    printf(" size = 0x%x, base_addr = 0x%x,"
           " length = 0x%x, type = 0x%x\n",
           (unsigned)mmap->size, (unsigned)(mmap->addr), (unsigned)(mmap->len),
           (unsigned)mmap->type);
  }

  /* ######## QUESTION 3 ######## */

  int *memzone1 = (int *)0x00;     // Type 1 (available) mem zone
  int *memzone2 = (int *)0x09fc00; // Type 2 (reserved) meme zone

  printf("Reading in memory addr 0x00 : %d\n", *memzone1);
  printf("Reading in memory addr 0x09fc00 : %d\n", *memzone2);

  printf("Writing \"2\" in memory addr 0x00\n");
  *memzone1 = 2;
  printf("Reading in memory addr 0x00 : %d\n", *memzone1);

  printf("Writing \"3\" in memory addr 0x09fc00\n");
  *memzone2 = 3;
  printf("Reading in memory addr 0x00 : %d\n", *memzone2);

  /* ######## QUESTION 4 ######## */

  int *memzone3 = (int *)0x8000020;

  printf("Reading outside the RAM (128MB + 32 bits) (0x8000020) : %d\n",
         *memzone3);

  printf("Writing \"4\" outside the RAM (128MB + 32 bits) (0x8000020)\n");
  *memzone3 = 4;
  printf("Reading outside the RAM (128MB + 32 bits) (0x8000020) : %d\n",
         *memzone3);
}
