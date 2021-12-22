# TP4 - La pagination

Le but du TP est de bien comprendre la pagination.

Vous pouvez directement modifier "tp.c".

Allez lire les fichiers "kernel/include/cr.h" et "kernel/include/pagemem.h". Vous trouverez plein d'informations, de structures et de macros utiles pour la résolution du TP.


## Rappels sur les tableaux et pointeurs en C:

Lorsque vous déclarez un tableau, le compilateur alloue la mémoire pour ce tableau automatiquement à l'endroit où vous le déclarez:

```c
int tab_a[10];

void fonction()
{
   int tab_b[10];
}
```

Dans cet exemple, `tab_a` est une variable globale non initialisée. Le compilateur et plus précisément le linker la placera dans la section ".bss". Le tableau aura une place de 10 entiers une fois le programme chargé en mémoire.

Le tableau `tab_b`, est une variable locale de `fonction()`. Le compilateur génère les instructions qui permettent d'allouer sa mémoire au moment de l'appel à `fonction()`, en déplaçant le pointeur de pile pour laisser de la place "localement", durant le temps de l'exécution de la fonction, aux 10 éléments de `tab_b`.

Dans le cas des pointeurs, nous pouvons les utiliser comme des tableaux pour adresser chaque élément, mais nous choisissons l'adresse à laquelle se trouvent les éléments. Dans vos programmes utilisateurs, vous faisiez généralement des `malloc()`. Dans notre noyau il n'y a pas de `malloc()` et nous faisons ce que nous voulons de la mémoire. Nous connaissons les zones de mémoire occupées par le code et les données du noyau, qu'il ne faut surtout pas écraser sous peine de voir le noyau crasher. Nous connaissons également la taille de la RAM et nous pouvons donc choisir des adresses disponibles en toute connaissance de cause.

```c
void fonction()
{
  int *tab_c = (int*)0x1234;
}
```

Dans cet exemple, tab_c est un pointeur sur entiers (ou une table d'entiers), pointant vers la zone mémoire à l'adresse `0x1234`. La variable `tab_c` elle-même se trouve dans la pile de `fonction()`. Mais à l'inverse d'un tableau, les éléments se trouvent à partir de `0x1234`. Peu impote que la variable tab_c soit une variable locale, ce qui compte c'est la zone mémoire adressée par le pointeur.

Le compilateur ne sait pas la taille de la zone mémoire adressée et potentiellement vous pouvez accéder à toute la mémoire à partir de cette adresse:

Dans une application classique, déclarer un tel pointeur provoquerait à coup sur un crash de l'application car l'adresse `0x1234` n'est jamais disponible pour une application (ex. sous Linux). Dans notre noyau cela ne pause pas de problème, nous utilisons la mémoire "physique" pour l'instant et nous n'avons pas de notion de tâche avec des espaces
d'adressage.


## Questions

### Question 1

**Affichez la valeur courante de `CR3`.**

---

### Question 2

**Allouez un PGD `(pde32_t*)` à l'adresse physique `0x600000` et mettez à jour `CR3` avec cette adresse.**

---

### Question 3

**Activez la pagination (`CR0`). Que se passe-t-il ? Pouquoi ?**

L'OS se relance en boucle. 
On a activé la pagination avec une PGD qui ne fait aucun sens (on a rien initialisé)
Donc dès que le noyau va tenter de faire un truc, on va avoir des fautes car aucunes addresses sont bonnes --> le noyu crash (par contre je sais pk il reboot)

---

### Question 4

**Comme pour le PGD, allouez une PTB `(pte32_t*)` à l'adresse `0x601000`.**

**Initialisez la mémoire virtuelle en "identity mapping": les adresses virtuelles doivent être les mêmes que les adresses physiques. Pour cela il va falloir:**

 - **Bien étudier les plages d'adresses physiques occupées par le noyau (`readelf -e kernel.elf`, regardez les program headers).**
 - **Préparer au moins une entrée dans le PGD pour la PTB.**
 - **Préparer plusieurs entrées dans la PTB.**

L'objectif ici est de lancer la pagination en mode identity mapping --> en gros découper la mémoire en pages et faire correspondre mem phy et virt directement.

Il faut donc créer des tables de pages qui vont pointer vers les zones du noyau ?

Mais en fait ce qu'il faut faire : 

- créer PTB
- puis ensuite une ou 2 PTB, mapper le code directement en créer les entrée 
on peut le faire dansun boucle en bouclant sur la PTB car au final l'offset de l'assr physique sera celui de l'addr virt (see https://wiki.osdev.org/Identity_Paging for ref)
- Mettre la PTB dans le première entrée de la PGD
- Puis mettre en place pagination avec CR0

la première PTB est pour le kernel au final (et un peu plus  ?) car le code du kernel est au début de la mémoire.


Note : quand on set une PTB comme entrée de la PGD, il faut prendre seulement les bits e poids forts moins les 12 premiers bits car ils ne rentrent pas et en servent à rien (voir cours, commme on est alignés sur 4KO partout on s'en fout des 12 bits de poids faible) 

---

### Question 5

**Une fois la pagination activée, essayez d'afficher le contenu d'une entrée de votre PTB. Que se passe-t-il ?**

On a un page fault ... (mais c'est pas normal ?)
On a : 

`#PF details: p:0 wr:0 us:0 id:0 addr 0x601000`

Donc on accède à 0x601000 (la première entrée de la PTB) mais la page n'est pas présente ?

Ah oui normal : on a mappé qu'une seule PTB dans la PGD et donc seulement 4MO de memoire.
Donc quand on veut accéder à 0x601000, cette addr n'est pas mappée dans la mémoire encore.
Faut créer une deuxième PTB qui va mapper les 4MO suivant de la mémoire et là normalement on est bon.

Quand on met une 2ème PTB on peut accéder à la mem en 0x601000 donc plus de Page fault

---

### Question 6

**Trouvez la solution pour être capable de modifier les entrées de votre PTB une fois la pagination activée.**

D'après TutoOS (http://a.michelizza.free.fr/pmwiki.php?n=TutoOS.Mm3), on peut soit faire une strucutre de controle à côte mais prise de tête, ou faire une entrée dans le PGD de loopback afin de pouvoir l'addresser elle même).

---

### Question 7

**Faites en sorte que l'adresse virtuelle `0xc0000000` vous permette de modifier votre PGD.**

Donc en reprenant l'idée du dessus, il faut faire en sorte de bien chosir l'index d'entrée de la PGD pour la modification (pas necessairement le dernier index) afin qu'on puisse adresser la première entrée de la PGD avec l'adresse 0xc0000000

Les 10 bits de poids forts de cette adresse sont l'offset dans la PGD. Ensuite les 10 bits qui suivent seront l'offset dans la PGD au final (donc quelle table on modifie), puis les 12 derniers bits (les 10 derniers au final) pour quelle pte dans la PTB selectionnée on modifie.

Pour modifier un pde et non un pte, il suffit de metrre 2 fois dans les 20 premiers bits le même offset (on rentre 2 fois à la suite dans l'entrée qui nous fait boucler)


Mais après la question c'est juste de modifier la PGD, donc on va faire en sorte d'avoir un pde qui pointe vers une ptb avec dont l'entrée qui va bien (calculée avec les décalages) va repointer au début de la PGD. --> on va faire ça, plus simple à tester après je pense ?


---

### Question 8

**Faites en sorte que les adresses virtuelles `0x700000` et `0x7ff000` mappent l'adresse physique `0x2000`. Affichez la chaîne de caractères à ces adresses virtuelles.**

La PTB1 est celle qui en théorie est déjà en train de mapper les addresses virt 0x7ff000 et 0x700000 en mode identity mapping pour l'instant.

Il faut donc trouver les index de ces adresses dans la PTB1, puis en changer le contenu pour qu'elles aillent vers l'adresse phy 0x2000.

Même méthode qu'avant, faut prendre les adresses virt, leur faire avoir le bon décalage afin de trouver l'offset (l'index) dans la PTB2 qui leur correspond, puis changer le contenu des pte correspondantes (rien de fou quand on a fait le reste au final)

Mecanisme utile pour la mémoire partagée.

---

### Question 9

**Effacez la première entrée du PGD. Que constatez-vous ? Expliquez pourquoi ?**

ça ne plante pas ! C'est du au fait que le flag PCD (page cache disable) est à 0 donc les tables de pages restent en cache donc elles existent encore même si on les supprime --> ça plante pas (le cache c'est le TLB).
