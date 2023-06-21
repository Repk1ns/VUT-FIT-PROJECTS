**ISA 2020/2021 - Discord Bot**
==========
Aplikace isabot je HTTP klient komunikující s platformou Discord.


Bot se připojí na Discord, zjistí client ID, Guild ID a  Channel ID kanálu "isa-bot". Na tom začne opakovat zprávy, které v tomto kanálu příjdou.
To vše pomocí HTTP dotazů a následném zpracování HTTP odpovědí.
Bot neopakuje zprávy od sebe samého a od uživatelů s řetězcem "bot" ve jméně.


Aplikaci je možnost sestavit pomocí příkazu *make*.
Pro spuštění testů je možno použít *make test*. Pro spuštění testu je nutno mít nainstalováno PHP.
Pro vyčištění adresáře je možno použít *make clean*

## Upozornění
Na serveru Merlin a Eva jsou starší verze OpenSSL. Při sestavení aplikace *isabot* je možno vidět warnings, protože TLSv1_2_method() pro CTX strukturu je deprecated.

# Odevzdané soubory:
* Makefile
* README.md
* manual.pdf
* main.cpp
* connection.cpp
* connection.h
* input.cpp
* input.h
* parse.cpp
* parse.h
* requests.cpp
* requests.h
* errors.cpp
* errors.h
* tester.php