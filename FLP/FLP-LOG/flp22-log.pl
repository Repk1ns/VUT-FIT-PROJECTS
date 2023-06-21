/*
* FLP 2022/2023
* Logicky projekt - Turinguv stroj
* Autor: Vojtech Mimochodek (xmimoc01)
* Datum: 18.4.2023
*/

/* 
* Načítání vstupu - input2.pl ze cvičení
*/

/** cte radky ze standardniho vstupu, konci na LF nebo EOF */
read_line(L,C) :-
	get_char(C),
	(isEOFEOL(C), L = [], !;
		read_line(LL,_),
		[C|LL] = L).


/** testuje znak na EOF nebo LF */
isEOFEOL(C) :-
	C == end_of_file;
	(char_code(C,Code), Code==10).


read_lines(Ls) :-
	read_line(L,C),
	( C == end_of_file, Ls = [] ;
	  read_lines(LLs), Ls = [L|LLs]
	).

print_lines([]).
print_lines([L|Ls]) :-
    writeln(L),
    print_lines(Ls).

/*
* Dynamický predikát pro uložení pravidel TS
*/
:- dynamic rule/4.

/*
* Funkce pro kontrolu, zda je pravidlo ve správném formátu
* Pravidla jsou ve formátu: <state> <symbol> <new_state> <new_symbol>
* <state> a <new_state> musí být velké písmeno
* <symbol> musí být malé písmeno nebo mezera, tzv. blank symbol
* <new_symbol> musí být malé písmeno, blank symbol nebo L, R značící posun doleva/doprava.
*/
valid_rule(A, B, C, D) :-
    is_upper(A),
    (is_lower(B) ; is_blank(B)),
    is_upper(C),
    (is_lower(D) ; is_left(D) ; is_right(D); is_blank(D)).

/*
* Funkce pro kontrolu, zdali je vstupní páska ve správném formátu.
* Obsahuje tedy pouze malá písmena nebo blank symboly.
*/
valid_tape([]).
valid_tape([H|T]) :-
	(is_lower(H) ; is_blank(H)) -> valid_tape(T) ; throw_error('Invalid tape format.').

/*
* Pomocné predikáty pro validaci*
* Validace zdali je zadané písmeno velké/malé/L/R/blank symbol.
*/
is_upper(X) :- char_type(X, upper).
is_lower(X) :- char_type(X, lower).
is_left('L').
is_right('R').
is_blank(' ').

/*
* Predikát pro zastavení simulace a vypsání chyby na výstup
*/
throw_error(M) :- writeln(M), halt.

/*
* Funkce pro přidávání načtených pravidel do databáze prologu.
* Pravidla jsou uložena pomocí dynamického predikátu rule/4.
* V případě, že je pravidlo ve špatném formátu, je program ukončen s chybovou hláškou.
*/
add_rules_from_lines([Tape], Tape) :- valid_tape(Tape).
add_rules_from_lines([Line|Rest], Tape) :-
	Line == [] -> throw_error('Invalid input - empty line between rules.') ;
    Line = [A, ' ', B, ' ', C, ' ', D|_],
    (valid_rule(A, B, C, D) -> assertz(rule(A, B, C, D)) ; throw_error('Invalid rule format.')),
    add_rules_from_lines(Rest, Tape).

/*
* Predikát pro aplikování pohybu do leva na TS.
* V případě, že se hlava stroje nachází úplně v levo, je vráceno false, jelikož z definice TS
* stroj nemůže posunout hlavu mimo pásku z leva.
*
* V opačném případě se hlava stroje posune do leva a na pásce se provede příslušná změna.
*/
move_left([[], _, _, _], _, _) :- false.
move_left([[HL|TL], _, CurrentSymbol, RightSideTape], NewState, NewConfig) :-
	NewConfig = [TL, NewState, HL, [CurrentSymbol|RightSideTape]].

/*
* Predikát pro aplikování pohybu do prava na TS.
* V případě, že se hlava stroje nachází úplně v pravo, čte se blank symbol.
* Při posunu hlavy úplně doprava jsou ukládány prázdné symboly na konci pásky z nekonečného množství blank symbolu v pravo.
* Při výpisu výsledků jsou tyto prázdné symboly z pravé strany pásky odstraněny.
*/
move_right([LeftSideTape, _, CurrentSymbol, []], NewState, NewConfig) :-
	NewConfig = [[CurrentSymbol|LeftSideTape], NewState, ' ', []].
move_right([LeftSideTape, _, CurrentSymbol, [HR|TR]], NewState, NewConfig) :-
	NewConfig = [[CurrentSymbol|LeftSideTape], NewState, HR, TR].

/*
* Predikát, který na základě vstupní konfigurace se pokusí nalézt pravidlo, které může být aplikováno
* a zkusí jej použít.
*
* Pravidla jsou vybírána na základě aktuálního stavu a aktuálního symbolu pod hlavou.
* Podle akce v nalezeném pravidle se vykoná příslušná akce. A to buď posun doprava, buď posun doleva 
* nebo přepsání symbolu pod hlavou.
*
* Výsledkem je nová konfigurace TS.
*/
apply_transition([LeftSideTape, State, CurrentSymbol, RightSideTape], NewConfig) :-
 	rule(State, CurrentSymbol, NewState, Action),
 	(is_left(Action) -> move_left([LeftSideTape, State, CurrentSymbol, RightSideTape], NewState, NewConfig) ;
 	is_right(Action) -> move_right([LeftSideTape, State, CurrentSymbol, RightSideTape], NewState, NewConfig) ;
 	is_lower(Action) -> NewConfig = [LeftSideTape, NewState, Action, RightSideTape] ;
	is_blank(Action) -> NewConfig = [LeftSideTape, NewState, Action, RightSideTape]).

/*
* Hlavní predikát pro simulaci TS.
* Jestliže vstupní predikát obsahuje symbol 'F', je zřejmé, že se TS nachází v koncovém stavu a simulace končí.
* Jinak simulace pokračuje. Konfigurace je předána predikátu apply_transition, který na základě aplikace pravidla, které nalezne
* vrátí novou konfiguraci. Ta je uložena do pole všech výsledných konfigurací. Tento predikát se dále iterativně volá vždy s novou konfigurací.
*
* Při nalezení koncového stavu jsou vráceny všechny nalezené konfigurace.
*
* Kvůli správnému detekování abnormálního zastavení je predikát obalen do pomocného predikátu outer_turing_machine/2
*/

outer_turing_machine(InitialConfig, FinalConfigs):-
    turing_machine(InitialConfig, FinalConfigs).

outer_turing_machine(_, _):-
    throw_error('Abnormaly terminated.'). 

turing_machine([LeftSideTape, 'F', CurrentSymbol, RightSideTape], [[LeftSideTape, 'F', CurrentSymbol, RightSideTape]]).
turing_machine(Config, [Config|RestOfConfigs]) :-
    apply_transition(Config, NewConfig), 
	turing_machine(NewConfig, RestOfConfigs).

/*
* Pomocné predikáty pro odstranění blank symbolů z pravé strany pásky.
* Při výpisu výsledků může být na pravé straně pásky vlivem operace posunu doprava vygenerováno větší množství blank symbolů.
* Z důvodu úhlednějšího výpisu na výstup, jsou odstraňovány.
*/
remove_spaces_from_end(Input, Output) :-
    reverse(Input, ReversedInput),
    remove_spaces_from_start(ReversedInput, ReversedOutput),
    reverse(ReversedOutput, Output).

remove_spaces_from_start([' '|Tail], Result) :-
    remove_spaces_from_start(Tail, Result).
remove_spaces_from_start(Other, Other).

/*
* Pomocný predikátor pro sjednocení prvků konfigurace do jednoho seznamu.
* Obsah levé strany pásky je reverzován, jelikož se v průběhu výpočtu z důvodu jednodušší manipulace
* pracovalo s levou stranou pásky obráceně.
*/
combine_list([LeftTape, State, Head, RightTape], Combined) :-
	reverse(LeftTape, ReversedLeftTape),
    append(ReversedLeftTape, [State], Temp1),
	append([Head], RightTape, Temp2),
	append(Temp1, Temp2, Combined).

/*
* Predikát pro výpis výsledných konfigurací.
* Iterativně projde všechny výsledné konfigurace a vypíše je na výstup.
* 
* Konfigurace jsou ve formátu [[leva_strana_pasky], stav, pozice_hlavy, [prava_strana_pasky]].
* Konfigurace je pomocí predikátu combine_list převedena do formátu [leva_strana_pasky, stav, pozice_hlavy, prava_strana_pasky].
* Následně je volán predikát atomic_list_concat k unifikaci seznamu do jednoho atomu. Ten je pak vypisován na výstup.
*/
print_final_configs([]) :- !.
print_final_configs([Config|RestOfConfigs]) :-
    combine_list(Config, Combined),
	remove_spaces_from_end(Combined, ClearResult),
	atomic_list_concat(ClearResult, Result),
	format('~w~n', Result),
    print_final_configs(RestOfConfigs).

/*
* Pomocný predikát, který ze vstupní pásky získá počíteční konfiguraci TS.
* Tvar konfigurace: [[leva_strana_pasky], stav, pozice_hlavy, [prava_strana_pasky]], viz zadani.
*/
initial_config([H|T], [[], 'S', H, T]).

/*
* Hlavní funkce programu.
* Nejprve je načten obsah vstupního souboru, nalezena počáteční konfigurace a následně spuštěna simulace TS a výpis výsledných konfigurací.
*/
start :-
    read_lines(Lines),
    add_rules_from_lines(Lines, Tape),
	initial_config(Tape, InitialConfig),
    outer_turing_machine(InitialConfig, FinalConfigs),
	print_final_configs(FinalConfigs), halt.
	