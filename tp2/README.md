# TP2 - Les interruptions et les exceptions

Le but du TP est de bien comprendre les interruptions et les exceptions. Vous pouvez modifier directement "tp.c".

Ce TP est également l'occasion d'utiliser les options de "trace" de Qemu, permettant de savoir ce qu'il se passe dans le CPU durant l'exécution de la VM.

Dans le fichier "utils/config.mk" vous pouvez ajouter `$(QDBG)` sur la ligne de définition de QOPT, afin de faire prendre en compte les options de trace au lancement de Qemu.

Par défaut, seules quelques traces sont activées, elles permettent de voir les exceptions générées.

Notre noyau dispose déjà d'une IDT. Elle est configurée dans `kernel/core/intr.c: intr_init()`.

Elle nous a permis d'intercepter les #GP du TP1.


## Questions

### Question 1

**Lisez "intr.c" et "idt.s" afin de bien comprendre l'implémentation des interruptions dans notre noyau.**

L'IDT contient des descripteurs d'interruptions `int_desc` qui sont initialisés avec leur propre handler d'interruption `isr`.

La valeur d'isr correspond à des petites fonctions que l'on peut voir comme des trampolines, localisées dans "idt.s". Ils servent à empiler le numéro de l'interruption et à aligner la pile lorsqu'il manque un code d'erreur pour l'évènement survenu.

Chaque trampoline, saute dans `idt_common` qui appelle le gestionnaire d'interruptions de haut niveau de notre noyau `intr_hdlr`.

---

### Question 2

**Activez les interruptions dans une boucle infinie. Que constatez-vous ?**

Rien de spécial, on fait une division par 0 en boucle, rien ne se passe, l'interruption se fait une fois et on arrive sur une fatal error (le programme ne reprend donc pas, ce qui était attendu compte tenu de notre implem).

---

### Question 3

**Gestion furtive des breakpoints #BP (Breakpoint Exception). Le but est de ne pas modifier `intr_hdlr` mais d'intercepter les #BP en amont depuis "tp.c".**


ON aurait pu voir BP est l'inteeruption 3 dans le code ou bien dans la doc Intel (table des vecteurs d'interruption)

---

### Question 3.1

**Localisez l'IDT et affichez son adresse de chargement.**

---

### Question 3.2

**Écrivez une fonction `bp_handler` affichant un message de debug à l'écran.**

---

### Question 3.3

**Écrivez une fonction `bp_trigger` déclenchant un breakpoint grâce à l'instruction `int3`.**

---

### Question 3.4

**Modifiez le descripteur d'interruption `(int_desc_t)` de #BP afin d'appeler `bp_handler()` la place du trampoline déjà installé. Faites un appel explicite à `bp_trigger()` dans `tp()`. Que constatez-vous ?**

On va set l'isr de BP dans l'IDT à la fonction bp_handler. Pour ce faire, j'ai regardé dans idt.s la position dans l'IDT de BP (index 3). Donc il suffit de set le desc de l'index 3 à notre fonction.

Quand on trigger manuellement, on a notre message de debug de bp_handler (donc bon signe) puis toujours la fatal exception. Néamoins le code de l'interruption est devenu int6 au lieu de int3 qui est celui de invalid opcode avec un eip = 0x08:0x8b


---

### Question 3.5

**Modifiez la fonction `bp_handler()` afin que le noyau ne génère plus de faute et rende la main à la fonction `tp()` :**
 - **Pensez à la pile au moment de l'arrivée de l'interruption**
 - **Que doit faire exactement la fonction `bp_handler()` lorsqu'elle se termine ?**
 - **N'oubliez pas qu'elle n'est pas une simple fonction mais un gestionnaire d'interruption.**


Voir https://pages.cs.wisc.edu/~remzi/Classes/354/Fall2012/Handouts/Handout-CallReturn.pdf pour explications


- Au moment de l'arrivée de l'interruption, on est dans `bp_trigger()`,  donc la stack frame est celle de bp_trigger().
-Par contre comme le hanlder est une fonction, alors on va avoir une autre stack frame pour la fonction `bp_hanlder()`. la fonction `bp_handler()` doit donc à la main changer de con texte (mov %EBP, %ESP / pop %EBP) afin de switch à la stackframe de `bp_trigger()` qui ne s'était pas terminée.
- Puis on fait un jmp vers cette addr de retour (pop EIP, ou un RET ?).
- Comme c'est un gestionnaire d'interruption, il faut utiliser iret au lieu de ret afin de pouvoir remettre le contexte bien dans les registres de controle (même si iret fait rien y'a pas de pile noyau).

---

### Question 3.6

**Affichez l'EIP sauvegardé dans la pile au moment où #BP est générée. A quelle adresse cela correspond-il ?**

Quand on affiche eip, on obtient : 0x30408c. On avait l'addr de l'isr de BP à 0x30404a. Donc eip pointe vers l'insruction qui suit le début du handler.

---

### Question 3.7

**Affichez un message de debug dans la fonction `bp_trigger()` après le déclenchement du breakpoint. Que constatez-vous ? Essayez de corriger le problème afin que le message s'affiche correctement.**

**Quelles conclusions tirez-vous du développement en C d'un gestionnaire d'interruption ? Pourquoi l'assembleur semble-t-il plus approprié ?**

Le message s'affiche bien donc je sais pas. Par contre assembleur plus approprié car bcp d'assembleur inline dans le code donc autant le faire en asm direct (notamment pour le iret)
Aussi, on a pas à se soucier des ret normaux des fonctions C, on peut direct coder avec des iret et tout le bazar


