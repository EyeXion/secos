# TP1 - La segmentation

Le but du TP est de bien comprendre la segmentation.

Vous pouvez directement modifier "tp.c".

Allez lire le fichier "kernel/include/segmem.h". Vous y trouverez plein d'informations, de structures et de macros utiles pour la résolution du TP.

:warning: **QEMU ne supporte pas "complètement" la segmentation.**

Il est nécessaire d'utiliser KVM à la place. Vous devez donc modifier au préalable le fichier utils/config.mk à la ligne:

```bash
 QEMU := $(shell which qemu-system-x86_64)
```

par

```bash
 QEMU := $(shell which kvm)
```


## Questions

### Question 1

**Grub a démarré notre noyau en mode protégé. Il a donc configuré une GDT avant d'exécuter notre point d'entrée. Affichez le contenu de cette GDT. Que constatez-vous ?**

**Servez-vous des outils présents dans notre OS (`get_gdtr(), seg_desc_t et gdt_reg_t`)**

Tout d'abord, nous allons chercher l'adresse du gdtr grace à `get_gdtr()` puis nous accedons à la struc du type `struct global_descriptor_table_register (aka gdt_reg_t)

https://www.felixcloutier.com/x86/sgdt

D'après la donc au dessus, on a avec la commande sgdt (celle utilisée dans `get_gdtr()`) : `If the operand-size attribute is 16 or 32 bits, the 16-bit limit field of the register is stored in the low 2 bytes of the memory location and the 32-bit base address is stored in the high 4 bytes.`


Donc on va extraire l'adresse de la gdt et sa limite (sa taille) directement depuis le resultat.



---

### Question 2

**Configurez votre propre GDT contenant des descripteurs ring 0:**
 - **Code, 32 bits RX, flat, indice 1**
 - **Données, 32 bits RW, flat, indice 2**

**Vous pouvez placer ces descripteurs où vous le souhaitez dans la GDT. Attention de bien respecter les restrictions matérielles :**
 - **La GDT doit avoir une adresse de base alignée sur 8 octets**
 - **Le premier descripteur (indice 0) doit être NULL**

**Chargez cette GDT, puis initialisez les registres de segments (cs/ss/ds/...) avec les bons sélecteurs afin qu'ils pointent vers vos nouveaux descripteurs.**


Segment flat : On ne specifie pas de base addr (sinon c'est un peu galère je pense, comme y'a rien dans le sujet.)

Comme la limite n'est pas précisée, on va mettre la même que les autres à savoir 0xfffff (comme on est sur du 32 bits ça passe).

Les registres de segments pointent vers le descripteur de segment direcement.
---

### Question 3

**Essayez d'exécuter le code suivant :**

```c
  #include <string.h>

  char  src[64];
  char *dst = 0;

  memset(src, 0xff, 64);
```

**Configurez un nouveau descripteur de données à l'index de votre choix :**
 - **data, ring 0**
 - **32 bits RW**
 - **base 0x600000**
 - **limite 32 octets**

**Chargez le registre de segment "es" de manière à adresser votre nouveau descripteur de données. Puis exécutez le code suivant :**

```c
  _memcpy8(dst, src, 32);
```

**Que se passe-t-il ? Pourquoi n'y a-t-il pas de faute mémoire alors que le pointeur `dst` est NULL ?**

**Effectuez à présent une copie de 64 octets. Que se passe-t-il ?**

No idea, besoin de demander explications.

Obs : quand on set es sur le nouveau segement AVANT le memeset, on se retrouve aec une fatal error (Exception: General protection).
Si on ne le set pas ou si on le set après, aucune erreur.