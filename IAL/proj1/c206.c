
/* c206.c **********************************************************}
{* Téma: Dvousměrně vázaný lineární seznam
**
**                   Návrh a referenční implementace: Bohuslav Křena, říjen 2001
**                            Přepracované do jazyka C: Martin Tuček, říjen 2004
**                                            Úpravy: Kamil Jeřábek, září 2019
**
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datová abstrakce reprezentován proměnnou
** typu tDLList (DL znamená Double-Linked a slouží pro odlišení
** jmen konstant, typů a funkcí od jmen u jednosměrně vázaného lineárního
** seznamu). Definici konstant a typů naleznete v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu
** s výše uvedenou datovou částí abstrakce tvoří abstraktní datový typ
** obousměrně vázaný lineární seznam:
**
**      DLInitList ...... inicializace seznamu před prvním použitím,
**      DLDisposeList ... zrušení všech prvků seznamu,
**      DLInsertFirst ... vložení prvku na začátek seznamu,
**      DLInsertLast .... vložení prvku na konec seznamu,
**      DLFirst ......... nastavení aktivity na první prvek,
**      DLLast .......... nastavení aktivity na poslední prvek,
**      DLCopyFirst ..... vrací hodnotu prvního prvku,
**      DLCopyLast ...... vrací hodnotu posledního prvku,
**      DLDeleteFirst ... zruší první prvek seznamu,
**      DLDeleteLast .... zruší poslední prvek seznamu,
**      DLPostDelete .... ruší prvek za aktivním prvkem,
**      DLPreDelete ..... ruší prvek před aktivním prvkem,
**      DLPostInsert .... vloží nový prvek za aktivní prvek seznamu,
**      DLPreInsert ..... vloží nový prvek před aktivní prvek seznamu,
**      DLCopy .......... vrací hodnotu aktivního prvku,
**      DLActualize ..... přepíše obsah aktivního prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na další prvek seznamu,
**      DLPred .......... posune aktivitu na předchozí prvek seznamu,
**      DLActive ........ zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce
** explicitně uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam 
** předá někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako
** procedury (v jazyce C procedurám odpovídají funkce vracející typ void).
**/

#include "c206.h"

int solved;
int errflg;

void DLError() {
/*
** Vytiskne upozornění na to, že došlo k chybě.
** Tato funkce bude volána z některých dále implementovaných operací.
**/	
    printf ("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* globální proměnná -- příznak ošetření chyby */
    return;
}

void DLInitList (tDLList *L) {
/*
** Provede inicializaci seznamu L před jeho prvním použitím (tzn. žádná
** z následujících funkcí nebude volána nad neinicializovaným seznamem).
** Tato inicializace se nikdy nebude provádět nad již inicializovaným
** seznamem, a proto tuto možnost neošetřujte. Vždy předpokládejte,
** že neinicializované proměnné mají nedefinovanou hodnotu.
**/
    
	L->First = NULL;
    L->Act = NULL;
    L->Last = NULL;
}

void DLDisposeList (tDLList *L) {
/*
** Zruší všechny prvky seznamu L a uvede seznam do stavu, v jakém
** se nacházel po inicializaci. Rušené prvky seznamu budou korektně
** uvolněny voláním operace free. 
**/

	tDLElemPtr tDLElemPOM = L->First; // Pomocna promenna
    tDLElemPtr tDLElemNEXT;

    while(tDLElemPOM != NULL) // Projde seznam a zrusi postupne vsechny prvky
    {
        tDLElemNEXT = tDLElemPOM->rptr;
        free(tDLElemPOM);
        tDLElemPOM = tDLElemNEXT;
    }

    // uvedeni seznamu do stavu v jakem byl po inicializaci
    L->First = NULL;
    L->Act = NULL;
    L->Last = NULL;
}

void DLInsertFirst (tDLList *L, int val) {
/*
** Vloží nový prvek na začátek seznamu L.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
	
	tDLElemPtr NewtDLElem = (tDLElemPtr) malloc(sizeof(struct tDLElem));
    if(NewtDLElem == NULL)
    {
        DLError();
        return;
    }

    tDLElemPtr tDLElemBefore;
    NewtDLElem->data = val; // Nastaveni dat prvku na hodnotu val

    if(L->First != NULL) // Jak seznam neni prazdny
    {
        tDLElemBefore = L->First;
        L->First = NewtDLElem;
        NewtDLElem->lptr = NULL;
        NewtDLElem->rptr = tDLElemBefore;
        tDLElemBefore->lptr = NewtDLElem;
    }
    else // Seznam je prazdny
    {
        L->First = NewtDLElem;
        L->Last = NewtDLElem;
        NewtDLElem->lptr = NULL;
        NewtDLElem->rptr = NULL;
    }
}

void DLInsertLast(tDLList *L, int val) {
/*
** Vloží nový prvek na konec seznamu L (symetrická operace k DLInsertFirst).
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/ 	
	
	tDLElemPtr NewtDLElem = (tDLElemPtr) malloc(sizeof(struct tDLElem));
    if(NewtDLElem == NULL)
    {
        DLError();
        return;
    }
    NewtDLElem->data = val;

    if(L->First != NULL)
    {
        L->Last->rptr = NewtDLElem;
        NewtDLElem->lptr = L->Last;
        NewtDLElem->rptr = NULL;
        L->Last = NewtDLElem;
    }
    else
    {
        L->First = NewtDLElem;
        L->Last = NewtDLElem;
        NewtDLElem->rptr = NULL;
        NewtDLElem->lptr = NULL;
    }
}

void DLFirst (tDLList *L) {
/*
** Nastaví aktivitu na první prvek seznamu L.
** Funkci implementujte jako jediný příkaz (nepočítáme-li return),
** aniž byste testovali, zda je seznam L prázdný.
**/
	
	L->Act = L->First;
}

void DLLast (tDLList *L) {
/*
** Nastaví aktivitu na poslední prvek seznamu L.
** Funkci implementujte jako jediný příkaz (nepočítáme-li return),
** aniž byste testovali, zda je seznam L prázdný.
**/
	
	L->Act = L->Last;
}

void DLCopyFirst (tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu prvního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/

	if(L->First == NULL) // Kontrola prazdneho seznamu
    {
        DLError();
        return;
    }

    *val = L->First->data; // Data z prvniho prvku na adresu kam ukazuje val
}

void DLCopyLast (tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu posledního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
	
	if(L->Last == NULL) // Kontrola prazdneho seznamu
    {
        DLError();
        return;
    }

    *val = L->Last->data;
}

void DLDeleteFirst (tDLList *L) {
/*
** Zruší první prvek seznamu L. Pokud byl první prvek aktivní, aktivita 
** se ztrácí. Pokud byl seznam L prázdný, nic se neděje.
**/
	
	tDLElemPtr tDLElemPOM;

    if(L->First != NULL) 
    {
        tDLElemPOM = L->First;

        if(L->First == L->Act)
        {
			L->Act = NULL;
        }
        if(L->First != L->Last) 
        {
            L->First->rptr->lptr = NULL;
            L->First = L->First->rptr;
        }
        else
        {
            L->First = NULL; 
            L->Last = NULL;
        }
        free(tDLElemPOM);
    }
}

void DLDeleteLast (tDLList *L) {
/*
** Zruší poslední prvek seznamu L. Pokud byl poslední prvek aktivní,
** aktivita seznamu se ztrácí. Pokud byl seznam L prázdný, nic se neděje.
**/ 
	
	tDLElemPtr tDLElemPOM;

    if(L->First != NULL) 
       {
            tDLElemPOM = L->Last;

            if(L->Last == L->Act)
            {
            	L->Act = NULL;
            }
            if(L->First != L->Last) 
            {
                L->Last->lptr->rptr = NULL;
                L->Last = L->Last->lptr;
            }
            else
            {
                L->First = NULL;
                L->Last = NULL;
            }
            free(tDLElemPOM);
       }
}

void DLPostDelete (tDLList *L) {
/*
** Zruší prvek seznamu L za aktivním prvkem.
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** posledním prvkem seznamu, nic se neděje.
**/
	
	if ((L->Act != NULL) && (L->Act != L->Last))
    {
        tDLElemPtr tDLElemPOM = L->Act->rptr;
        tDLElemPtr tDLElemBefore = tDLElemPOM->rptr;
        
        if(tDLElemBefore)
        {
        	tDLElemBefore->lptr = L->Act;
        	L->Act->rptr = tDLElemBefore;
        }
        else
        {
            L->Act->rptr = NULL;
            L->First = L->Act;
        }
        free(tDLElemPOM);
    }
}

void DLPreDelete (tDLList *L) {
/*
** Zruší prvek před aktivním prvkem seznamu L .
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** prvním prvkem seznamu, nic se neděje.
**/
	
    if ((L->Act != NULL) && (L->Act != L->First))
    {
        tDLElemPtr tDLElemPOM = L->Act->lptr;
        tDLElemPtr tDLElemBefore = tDLElemPOM->lptr;

        if(tDLElemBefore)
        {
        	tDLElemBefore->rptr = L->Act;
        	L->Act->lptr = tDLElemBefore;
        }
        else
        {
        	L->Act->lptr = NULL;
            L->First = L->Act;
        }
        free(tDLElemPOM);
    }
}

void DLPostInsert (tDLList *L, int val) {
/*
** Vloží prvek za aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se neděje.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
	
	tDLElemPtr tDLElemPOM;
	tDLElemPtr tDLElemAfter;

        if (L->Act != NULL) 
        {
        	tDLElemPOM = malloc (sizeof (struct tDLElem));
			if (tDLElemPOM == NULL)
 	    	{
 	    		DLError();
 	    		return;
			}
			else 
			{
				tDLElemAfter = L->Act->rptr;
				tDLElemPOM->data = val;
				tDLElemPOM->lptr = L->Act;
				tDLElemPOM->rptr = tDLElemAfter;
				L->Act->rptr = tDLElemPOM;
			if (tDLElemAfter != NULL) 
			{
				tDLElemAfter->lptr = tDLElemPOM;
			}
		}
	}
}

void DLPreInsert (tDLList *L, int val) {
/*
** Vloží prvek před aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se neděje.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
	
	
	tDLElemPtr tDLElemPOM;
	tDLElemPtr tDLElemBefore;

        if (L->Act != NULL) 
        {
        	tDLElemPOM = malloc (sizeof (struct tDLElem));
			if (tDLElemPOM == NULL)
 	    	{
 	    		DLError();
 	    		return;
			}
			else 
			{
				tDLElemBefore = L->Act->lptr;
				tDLElemPOM->data = val;
				tDLElemPOM->lptr = tDLElemBefore;
				tDLElemPOM->rptr = L->Act;
			if (tDLElemBefore != NULL) 
			{
				tDLElemBefore->rptr = tDLElemPOM;
			}
		}
	}	
}

void DLCopy (tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu aktivního prvku seznamu L.
** Pokud seznam L není aktivní, volá funkci DLError ().
**/
		
	
    if (L->Act == NULL) // Kontrola prazdneho seznamu
	{
		DLError(); // Error v pripade prazdneho seznamu
		return;
	}

	*val = L->Act->data;
}

void DLActualize (tDLList *L, int val) {
/*
** Přepíše obsah aktivního prvku seznamu L.
** Pokud seznam L není aktivní, nedělá nic.
**/
	
	if (L->Act != NULL) // Jestlize neni aktivni nedela nic
    {
        L->Act->data = val; // Prepsani aktivniho prvku
    }
}

void DLSucc (tDLList *L) {
/*
** Posune aktivitu na následující prvek seznamu L.
** Není-li seznam aktivní, nedělá nic.
** Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
**/
	
	if(L->Act != NULL) // Seznam musí být aktivní
    {
        L->Act = L->Act->rptr; // Nastavime aktivitu na další prvek
    }
}


void DLPred (tDLList *L) {
/*
** Posune aktivitu na předchozí prvek seznamu L.
** Není-li seznam aktivní, nedělá nic.
** Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
**/
	
	if (L->Act != NULL) // Seznam musi byt aktivni
	{
		L->Act = L->Act->lptr; // Nastavime aktivitu na predchozi prvek
	}
}

int DLActive (tDLList *L) {
/*
** Je-li seznam L aktivní, vrací nenulovou hodnotu, jinak vrací 0.
** Funkci je vhodné implementovat jedním příkazem return.
**/
	
	return (L->Act != NULL) ? 1 : 0;
}

/* Konec c206.c*/
