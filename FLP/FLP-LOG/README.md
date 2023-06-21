# Funkcionální a logické programování 2022/2023
## Logický projekt - Turingův stroj
### Autor: Vojtěch Mimochodek (xmimoc01)

### Popis
Cílem tohoto projektu bylo vytvořit v jazyce Prolog simulátor nedeterministického Turingova stroje.
Tento program načte obsah ze vstupního souboru, ze kterého načte do interní paměti přechodová pravidla Turingova stroje a obsah pásky.
Výstupem je posloupnost výsledných konfigurací stroje.

### Implementace
Hlavním predikátem celého programu je predikát start. Z něj je na začátku volán predikát read_lines/1 který ze zadaného souboru
načte jeho obsah. Logika načítání obsahu souboru je převzata ze souboru input2.pl, který nám byl představen v rámci 1. cvičení.
Načtený obsah souboru je postupně skrze predikát add_lines_to_rules/2 rozparsován a řádky jsou pomocí predikát assertz/1 ukládány
do databáze přes dynamický predikát rule/4. Ten představuje přechodové pravidlo Turingova stroje. Při ukládání každého z pravidel
je provedena validace jeho správného formátu. Na posledním řádku je očekáván obsah pásky. Ta je z predikátu add_lines_to_rules/2
vrácena jako výsledek a předtím rovněž zvalidována. Díky načtené pásce a pravidlům je vytvořena počáteční konfigurace stroje,
která je jako parametr předána predikátu outer_turing_machine. Tento predikát zapouzdřuje predikát turing_machine/2. To kvůli tomu,
aby bylo možné detekovat abnormální zastavení. Chyba vyvolána abnormálním zastavením je vyvolána tehdy, pokud predikát turing_machine prozkoumá
veškerý stavový prostor a nenalezne správné východisko. Predikát turing_machine funguje tak, že je iterativně volán, 
dokud není nalezena koncová konfigurace.
Nalezení správného pravidla a jeho aplikace je zajištěna v predikátu apply_transition/2. Ten pomocí aktuální konfigurace nalezne
pravidlo a snaží se jej aplikovat. Výsledkem je nová konfigurace, která je vrácena predikátu turing_machine/2, kde je uložena do
seznamu výsledných konfigurací. Operace, které jsou vykonávány můžou být buď pohyb hlavy po pásce do leva, 
pohyb hlavy po pásce do prava nebo přepsání symbolu pod hlavou. Všechny operace pracují s konfiguracemi stroje, které jsou
v následujícím formátu: `[[levá_strana_pásky], aktuální_stav, aktuální_symbol_pod_hlavou, [pravá_strana_pásky]]`. Levá strana
pásky je navíc pro snadnou práci reverzována.
Pohyb do leva je řešen v predikátu move_left/3. V případě, že je levá
část pásky prázdná, je jasné, že se hlava stroje nachází úplně v levo, čímž pádem není pohyb do leva možný a je vráceno false. V opačném
případě se hlava posune do leva, aktuální symbol se přidá do pravé části pásky a první symbol z levé části se stává novým aktuálním symbolem.
Nový aktuální stav se přečte z aplikovaného pravidla. Pohyb do prava je řešen obdobně jako pohyb do leva, avšak s tím rozdílem, kdy je hlava
stroje úplně v pravo, není vráceno false, ale blank symbol z teoreticky nekonečného množství blank symbolů na pravé straně.
Pokud je nalezen koncových stav, je z predikátu turing_machine/2 (outer_turing_machine/2) vrácen list výsledných konfigurací, kterými stroj během výpočtu prošel.
Tento seznam je předán predikátu print_final_configs/1, který iterativně vypíše každou z konfigurací. Při výpisu je konfigurace nejprve
převedena do jednoho listu, tedy do formátu: `[levá_strana_pásky, stav, symbol_pod_hlavou, pravá_strana_pásky]`. Jelikož jsme pracovali
s levou stranou pásky reverzovanou, je potřeba ji pro správný výpis reverzovat zpět. Nad touto upravenou konfigurací je zavolán predikát
atomic_list_concat/1 pro unifikaci hodnot v listu do jednoho atomu. Ten je nakonec vypsán.
Po vypsání všech výsledných konfigurací je program ukončen pomocí halt/0.

### Sestavení a spuštění
Program je překládán pomocí nástroje Makefile.

> ``make`` 

Program je spouštěn pomocí spustitelného souboru `flp22-log` 

> ``./flp22-log < nazev_vstupniho_souboru > nazev_vystupniho_souboru``

nebo v případě čtení ze STDIN

> ``./flp22-log`` 

### Testování a měření
Projekt je doplněn o testovací případy ve složce `tests` a jednoduchý script v jazyce Python, který spustí program pro každý testovací soubor s koncovkou .in
a porovná jej s referenčním souborem s koncovkou .out.

Testovací script je možné spustit (například na serveru Merlin) pomocí ``python3 control.py``.

Testovací soubory jsou následující:

- `at1.in`
Tento soubor testuje, zdali simulace správně vyvolá chybu abnormálního zastavení. 
V testu se nachází jediné pravidlo, které vede k vystoupení z pásky z levé strany.
Doba výpočtu: 0.021s

- `at2.in`
Tento soubor testuje, zdali simulace správně vyvolá chybu abnormálního zastavení. 
V testu se nachází několik pravidel jenž vedou do koncového stavu. Páska však neobsahuje počáteční
blank symbol, ze kterého vychází první pravidlo.
Doba výpočtu: 0.019s

- `at3.in`
Tento soubor testuje, zdali simulace správně vyvolá chybu abnormálního zastavení. 
Test obsahuje podobný příklad jako je v zadání projektu, avšak bez pravidla jenž vede do koncového stavu.
Doba výpočtu: 0.021s

- `wf1.in`
Testo soubor kontroluje, zdali jsou zadaná pravidla ve správném formátu.
Doba výpočtu: 0.031s

- `wf2.in`
Tento soubor kontroluje, zdali je vstupní páska ve správném formátu.
Doba výpočtu: 0.022s

- `tm1.in`
Tento testovací scénař obsahuje vzorový Turingův stroj ze zadání.
Doba výpočtu: 0.022s

- `tm2.in`
Tento testovací scénař obsahuje Turingův stroj, který přijímá jazyk `L = { w ∈ {a,b,c}* | #a(w) = #b(w) = #c(w) }`.
Vstupní řetězec k testování je ` aaabbbccc`.
Doba výpočtu: 0.024s

- `tm3.in`
Tento testovací scénař obsahuje Turingův stroj, který přijímá jazyk `L = { w ∈ {a,b,c}* | #a(w) > 2*#b(w) }`.
Vstupní řetězec k testování je ` aabbacacabaabcaa`
Doba výpočtu: 0.026s

- `tm4.in`
Tento testovací scénař obsahuje Turingův stroj, který přijímá jazyk `L = { a^(2)^(n) | n >= 0 }`.
Vstupní řetězec k testování je ` aaaa`
Doba výpočtu: 0.020s

Časy výpočtu byly měřeny na serveru Merlin pomocí nástroje `time`.

### Závěr
Podařilo se mi úspěšně naimplementovat simulátor Turingova stroje v jazyce Prolog, který nalezne posloupnost konfigurací stroje,
pokud existuje řešení. V případě chybného stroje nebo nesprávného vstupního řetězce vyvolá vyjímku `Abnormaly terminated`. Rovněž kontroluje,
zdali jsou pravidla a vstupní páska/řetězec ve správném formátu. V případě, že stroj cyklí, program nevypisuje nic a cyklí.