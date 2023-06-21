![logo](logo.png "logo")
# **IPK - Počítačové komunikace a sítě**
## **Projekt 1 - HTTP resolver doménových jmen**  
**Autor: Vojtěch Mimochodek**  
**Login: xmimoc01**  

Server jehož cílem je překlad doménových jmen je naprogramován v jazyku Python3. Komunikace je zajištěna pomocí protokolu HTTP s podporou dvou operací GET a POST. Je využivána lokální resolver stanice, na které server běží.

## **Použití:**

- Příkaz pro spuštění serveru: make run PORT=1234

- PORT je číslo v rozmezí 0 - 65535

- Klávesová zkratka CTRL+C pro ukončení serveru



## **Implementace:**

Celý projekt je naprogramován v souboru server.py

- `Import`  
    Využívány jsou knihovny následující knihovny:
    - socket - pro práci se sockety
    - re - pro práci s regexy, které jsou ve scriptu využívány k ošetření chyb
    - sys - systémová knihovna pro zpracování argumentů
- `Funkce`  
    - V projektu je naprogramována funkce *send_err*, která zajišťuje odeslání HTTP hlavičky s chybovým kódem v případě chyby.
    Přijímá string ve tvaru HTTP, který zakóduje z UTF-8 do bajtů a odešle klientovi jako HTTP Response.
- `Server` 
    - Samotný server běží v nekonečné smyčce *while True:*
    - Před while cyklem proběhne deklarace *s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)* pro práci se sockety. Nastavení HOST na localhost (127.0.0.1) a PORT na argument předaný z příkazu make run PORT=. Je zavolána funkce *bind()* na zadaný HOST a PORT. A zahájeno naslouchání skrze *listen()* pro příjem socketů.
    - Na začátku while cyklu je volána funkce *accept()* pro příjmutí socketu. Do proměnné *data* se nahrají data z příjmutého socketu. Ty se dále dekódují pomocí *decode()*. 
- `GET a POST`
    - Obdržená data z requestu jsou parsována pomocí funkce *str.split()* Díky ní server zjistí o jakou metodu se jedná, jaký typ (A nebo PTR), konkrétní doménové jméno či IP adresu. V případě typu A se dané jméno přeloží na IP adresu pomocí funkce *gethostbyname()*, upraví do patřičného HTTP tvaru - *"HTTP/1.1 200 OK\r\n\r\n + ODPOVED \r\n\r\n"*, kde ODPOVED je ve tvaru DOTAZ:A=VYSLEDEK. V případě typu PTR se IP adresa přeloží na hledané jméno pomocí funkce *gethostbyaddr()*. Jedná-li se o metodu POST, která obsahuje soubor s více požadavky, je využito For cyklu pro zpracování všech těchto požadavků. 
    - Během celého tohoto procesu probíhají kontroly, které mají za úkol ošetřit všechny chybové stavy.
    - Po každém odeslání Response klientovi je socket uzavřen pomocí *conn.close()*.
    - Při metodě POST chybné požadavky v textovém souboru jsou ignorovány. Vrací se pouze řádky, které jdou úspěšně přeložit. Nastane-li situace, kdy jsou všechny řádky špatné, vrací se chyba 400 Bad Request.

## **Request a použití curl jako klienta**
## Forma requestu přijímaná do proměnné *data*
`GET /resolve?name=apple.com&type=A HTTP/1.1`  
`POST /dns-query HTTP/1.1`
## Použití klienta
- GET
Request:

`curl localhost:5353/resolve?name=www.fit.vutbr.cz\&type=A`  

Response:  

    www.fit.vutbr.cz:A=147.229.9.23 

- POST
Request:
`curl --data-binary @queries.txt -X POST http://localhost:5353/dns-query`  

Obsah queries.txt:  

    www.fit.vutbr.cz:A  
    www.google.com:A  
    www.seznam.cz:A  
    147.229.14.131:PTR  
    ihned.cz:A  

Response:  

    www.fit.vutbr.cz:A=147.229.9.23
    www.google.com:A=216.58.201.68
    www.seznam.cz:A=77.75.74.176
    147.229.14.131:PTR=dhcpz131.fit.vutbr.cz
    ihned.cz:A=46.255.231.42


# **Reference:**
[Socket Programming in Python](https://realpython.com/python-sockets/)