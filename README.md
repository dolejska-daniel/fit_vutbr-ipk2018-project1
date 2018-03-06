
# IPK project 1, 2018
> Daniel Dolejška (xdolej08@stud.fit.vutbr.cz)

# Obsah
1. [Možnosti překladu](#moznosti-prekladu)
	1. [make](#make-makefile-)
	2. [Přímý překlad](#primy-preklad)
2. [Chybové kódy](#chybove-kody)
3. [Požadavky](#pozadavky)
4. [Omezení](#omezeni)
5. [Implementace](#implementace)
	1. [Klient](#klient)
		1. [Příklady použití](#priklady-pouziti)
	2. [Server](#server)
		1. [Speciální odpovědi](#specialni-odpovedi)

# Možnosti překladu
Níže jsou popsány možnosti překladu ať už za použití `make` či přímo `gcc`.

## make (Makefile)
Překlad probíhá vždy s parametry `-Wall -Wextra -pedantic`. Překlad pomocí `make` dále podporuje následující možnosti modifikující podrobnosti překladu:

| Příkaz překladu     | Výsledný soubor             | Další informace |
|---------------------|-----------------------------|-----------------|
| `make`, `make all`  | `ipk-client`, `ipk-server`  | Přeloží zdrojové soubory klientské (client.c) i serverové (server.c) části aplikace **bez definice** [speciálních konstant](#primy-preklad). |
| `make debug`        | `ipk-client`, `ipk-server`  | Přeloží zdrojové soubory klientské (client.c) i serverové (server.c) části aplikace a **definuje všechny** [speciální konstanty](#primy-preklad). |
| `make server`       | `ipk-server`                | Přeloží zdrojový soubor pouze serverové (server.c) části aplikace **bez definice** [speciálních konstant](#primy-preklad). |
| `make debug-server` | `ipk-server`                | Přeloží zdrojový soubor pouze serverové (server.c) části aplikace a **definuje všechny** [speciální konstanty](#primy-preklad). |
| `make client`       | `ipk-client`                | Přeloží zdrojový soubor pouze klientské (client.c) části aplikace **bez definice** [speciálních konstant](#primy-preklad). |
| `make debug-client` | `ipk-client`                | Přeloží zdrojový soubor pouze klientské (client.c) části aplikace a **definuje všechny** [speciální konstanty](#primy-preklad). |


## Přímý překlad
Přímý překlad například pomocí `gcc` podporuje definici speciálních konstant, které dovolí výpis užitečných informací při ladění programu. Programy (jak klient tak server) podporují následující konstanty:

| Název konstanty       | Funkce |
|-----------------------|--------|
| `DEBUG_PRINT_ENABLED` | Dovolí tisk zpráv popisující aktuální chování programu na `stderr`. |
| `DEBUG_LOG_ENABLED`   | Dovolí tisk obsahu některých důležitých proměnných za běhu programu na `stderr`. |
| `DEBUG_ERR_ENABLED`   | Dovolí tisk zpráv blíže popisujících chybovou situaci, pokud k ní dojde na `stderr`. |

Konstanty je možné definovat při překladu následovně:
```bash
gcc -DDEBUG_LOG_ENABLED client.c -o ipk-client`
```

# Chybové kódy
Návratové hodnoty programu a jejich význam:

**Společné**:
  - **1** - Chyba při vytváření socketu
  - **5** - Chybná kombinace přepínačů nebo nepodporovaný přepínač
  - **6** - Hodnota argumentu přepínače je chybná
  - **7** - Chybí povinný přepínač

**Klient**
  - **2** - Chyba při vyhledávání hostitele (vyhledání hostname/IP) 
  - **3** - Chyba při vytváření spojení s hostitelem (operace `connect`)

**Server**:
  - **2** - Chyba při operaci `bind`
  - **3** - Chyba při operaci `listen`

# Požadavky
**Serverová část** programu vyžaduje pro správnou funkcionalitu práva čtení pro soubor `/etc/passwd`.

# Omezení
**Klient** vyžaduje zadání vyhledávacího řetězce bezprostředně za přepínače `-n`, `-f` či `-l`. Zároveň, pokud budou tyto přepínače umístěny na jiné místo, než na konec, dojde k chybnému zpracování přepínačů a téměř jistě bude program ukončen.

**Server** dokáže obsloužit pouze jednoho klienta najednou. V případě více současných spojení, server nejdříve zpracuje požadavek prvního klienta v řadě a až pak se věnuje ostatním.

# Implementace
## Klient
Po zpracování přepínačů z příkazové řádky klient odesílá jeden řídící packet obsahující _typ vyhledávání_ (`info`, `home`, `list`) a _obsah vyhledávání_ oddělené dvojtečkou - `{info|home|list}:[vyraz]` (př.: `info:xdolej08`, `home:xdolej08`, `list:xd` či `list:`).

Následně čeká na odpověď serveru. Přímo vypisuje všechny (úspěšné) přicházející data, pokud se neshodují s _klíčovými slovy_, dokud není spojení ukončeno. V případě odpovědi indikující chybu (či prázdný výsledek vyhledávání) vypisuje přednastavená chybová hlášení v závislosti na typu chyby.

### Příklady použití
Předpokladem je spuštěný server na místním počítači běžící na portu 56789.

**Přepínač `-n` (úspěšné vyhledání)**
```bash
xdolej08@merlin: ~/_VUT/IPK$ ./ipk-client -h 127.0.0.1 -p 56789 -n xdolej08
Dolejska Daniel,FIT BIT 2r
xdolej08@merlin: ~/_VUT/IPK$
```

**Přepínač `-n` (neúspěšné vyhledání)**
```bash
xdolej08@merlin: ~/_VUT/IPK$ ./ipk-client -h 127.0.0.1 -p 56789 -n xdolej10
There were found no users matching your specifications.
xdolej08@merlin: ~/_VUT/IPK$
```
_Zpráva zobrazená při neúspěšném vyhledávání je pro všechny přepínače stejná._

**Přepínač `-f` (neúspěšné vyhledání)**
```bash
xdolej08@merlin: ~/_VUT/IPK$ ./ipk-client -h 127.0.0.1 -p 56789 -f xdolej08
/homes/eva/xd/xdolej08
xdolej08@merlin: ~/_VUT/IPK$
```

**Přepínač `-l` (úspěšné vyhledání)**
```bash
xdolej08@merlin: ~/_VUT/IPK$ ./ipk-client -h 127.0.0.1 -p 56789 -l xdole
xdolec02
xdolej08
...
xdolez81
xdolez82
xdolej08@merlin: ~/_VUT/IPK$
```

## Server
Po připojení klienta server čeká na kontrolní data ve výše uvedeném formátu. Identifikuje typ vyžádaných informací a provede zpracování souboru `/etc/passwd`:

Program prochází soubor znak po znaku od začátku do konce. Porovnává několik prvních znaků každého řádku (do znaku `:`) se zaslaným vyhledávacím řetězcem od klienta. V případě, že se znaky neshodují jsou další znaky až po znak nového řádku `\n` ignorovány. Následně celý proces začíná od začátku. Pokud se ovšem znaky shodují je daný řádek uložen do pole pro pozdější odeslání klientovi.

Po kompletním zpracování souboru je výsledek ověřen - pokud uživatel vyžádal `info` (přepínač `-n`) či `home` (přepínač `-f`) a počet výsledků je buď nulový či větší než jeden, je odeslána odpověď indikující, že vyhledávání nenašlo požadovaný záznam.

Pokud uživatel vyžádal `list` (přepínač `-l`) a počet výsledků je alespoň 1, jsou postupně odeslány všechny nalezené záznamy. Jinak je znovu odeslána odpověď indikující, prázdný výsledek.

Po odeslání všech dat odesílá server speciální odpověď, která značí, že již odeslal všechny data. Následně spojení ukončuje. Díky nastavení `setsockopt(..., SO_LINGER, ...)` korektně odešle odpověď o ukončení před uzavřením spojení.

### Speciální odpovědi
V případě, že je stream prázdný, odesílá server speciální odpověď `!empty` - označující, že vyhledávání bylo úspěšné ovšem nebyly nalezeny žádné výsledky, které by odpovídaly vyhledávacímu řetězci zaslaného klientem.

V případě, že došlo k chybě při filtrování výsledků či jiné chybě, ať už v průběhu zpracování požadavku či při odesílání dat, odesílá server speciální odpověď `!err` následovanou textovým řetězcem popisující vzniklý problém, vzájemně oddělené znakem `\0`.

V případě, že server již nemá další data, která by klientovi mohl odeslat, zasílá odpověď `!bye` a následně s klientem ukončuje spojení a začíná řešit následujícího klienta ve frontě.
