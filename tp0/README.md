# TP0 - Familiarisation avec le noyau secos

Le but du TP est de se familiariser avec le noyau secos.

Le noyau est linké grâce au LD-script "utils/linker.lds" qui définit l'agencement mémoire du noyau lorsqu'il va être chargé par le bootloader GRUB.

```bash
(tp0)$ readelf -l kernel.elf

Elf file type is EXEC (Executable file)
Entry point 0x302010
There are 3 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x000094 0x00300000 0x00300000 0x0000c 0x0000c RWE 0x4
  LOAD           0x0000a0 0x00300010 0x00300010 0x00000 0x02000 RW  0x10
  LOAD           0x0000b0 0x00302010 0x00302010 0x013f8 0x01810 RWE 0x20
```

Le fichier ELF nous indique que le noyau est chargé à l'adresse physique `0x300000`.

Lors du démarrage, le noyau vous affiche sur le port série, la zone mémoire qu'il occupe:

```bash
(tp0)$ make qemu
secos-a241db6-59e4545 (c) Airbus
kernel mem [0x302010 - 0x303820]
```

Si vous regardez le code "start.c", vous découvrirez l'affichage de ces informations à l'aide des symbols `__kernel_start__` et `__kernel_end__`.

Vous pouvez modifier le fichier "tp0/tp.c" pour commencer le TP.

Par défaut, les fichiers de TP permettent d'accéder à un objet global pré-initialisé `info`. C'est un pointeur dont la structure est définie dans "include/info.h". Pour l'instant il ne contient que le champ `mbi` provenant de Grub. Ce champ donne accès à la structure `multiboot_info` (version 1) qui contient de nombreuses informations sur le système. Vous trouverez sa définition dans "include/mbi.h" et "include/grub_mbi.h".


## Questions

### Question 1

**Pourquoi le noyau indique `0x302010` et pas `0x300000` comme adresse de début ? Indice: essayer de comprendre linker.lds, regardez également le code de "entry.s"**

On voit dans le code de linker.lds (qui va j'imagine définir les sections du fichier kernel.elf) que le début est en 0x300000, mais que le  __kernel_start_ est en 3éme pos après les sections .mbh (multiboot headr) et .stack . 
Aussi, dans entry.s, on voit qu'il y a des lignes au début : 

```
.section .stack, "aw", @nobits
.align 16
.space 0x2000

.text
```

On a donc un espace de 0x2000 entre la stack et le text (après bon c'est pas l'objet de la question ?), puis 16 bits (donc 0x10) d'alignement ce qui est exactement la différence entre `0x302010` et `0x300000` (peut-être que cela laisse la place pour le mbh + stack (`0x2000`) et ensuite 16 bits (`0x10`) pour l'alignement.

Au final on a le `__kernel_start__` a l'adresse `0x302010` car on a en premier dans kernel.elf les sections .mbh et .stack. (**a vérifier**)

See : http://www.bravegnu.org/gnu-eprog/lds.html

---

### Question 2

**A l'aide de la structure "multiboot_info", vous devez afficher la cartographie mémoire de la VM. Pour cela, utilisez les champs `mmap_addr` et `mmap_length`. Aidez-vous d'internet pour trouver des informations sur le standard multiboot. Le champ `mmap_addr` contient l'adresse du premier objet de type `multiboot_memory_map` qui vous permettra d'afficher des informations sur la mémoire.**


Voir ce lien pour la doc de grub sur le multiboot : https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Motivation

Donc j'ai trouvé le code directement dans la doc de Grub (donc j'ai pas trop triché je suis tombé dessus au final :)).

En gros le champ `mmap_length` est la taille totale de tout les objets de type `multiboot_memory_map` et `mmap_addr` est l'adresse du premier objet de ce type.

Chaque objet `multiboot_memory_map` représente une zone mémoire de la VM et un type lui est assigné (voir def de la strcuture `multiboot_memory_map` pour les types).

On va donc enumérer tout les `multiboot_memory_map` dans le for et affichet leur taille, leur type etc (length est la taille de la zone mémoire, size juste la taille de la structure `multiboot_memory_map`).


---

### Question 3

**Vous allez découvrir différentes zones de mémoire physique, certaines étant réservées, d'autres libres. Déclarez un pointeur d'entier par exemple et initialisez le avec une des adresses que vous avez trouvée. Essayez de lire/écrire la mémoire à cette adresse. Que se passe-t-il ? Avez-vous un "segmentation fault" ? Pourquoi ?**

Avec la question précedante, on voit qu'il y a des zones mémoire avec different type (ici 1 ou 2). Type 1 veut dire que la zone mémoire est libre alors que 2 veut dire que la zone est reservée.

La zone mémoire commençant à l'adresse `0x00` est de type 1 donc libre et la zone commençant en `0x09fc00` est reservée.

Pas de segfault pour rien du tout et l'ecriture est effective dans les 2 zones. (sûrement car on écrit dans la même physique directement).


---

### Question 4

**Essayez de lire/écrire en dehors de la RAM disponible (128MB). Que se passe-t-il ?**

On peut lire mais on a 0 en resultat dès qu'on depasse la taille de la RAM.

On a pas de segfault quand on écrit mais l'ecriture n'est pas prise en compte (on a tjrs 0 en lecture après).