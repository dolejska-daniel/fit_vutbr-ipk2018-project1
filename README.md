# IPK project 2018
> Daniel Dolejška (xdolej08@stud.fit.vutbr.cz)

# Obsah
1. [Možnosti překladu](#moznosti-prekladu)
	1. [make](#make-makefile-)
	2. [gcc](#gcc)
2. [Chybové kódy](#chybove-kody)
3. [Požadavky](#pozadavky)
4. [Omezení](#omezeni)
5. [Implementace](#implementace)
	1. [Klient](#klient)
	2. [Server](#server)
		1. [Speciální odpovědi](#specialni-odpovedi)

# Možnosti překladu
Níže jsou popsány možnosti překladu ať už za použití `make` či přímo `gcc`.

## make (Makefile)
Překlad probíhá vždy s parametry `-Wall -Wextra -pedantic`. Překlad pomocí `make` dále podporuje následující možnosti modifikující podrobnosti překladu:
  - `make` a `make all` - přeloží zdrojové soubory klientské i serverové části aplikace na soubory `ipk-client` a `ipk-server`.
  - `make debug` - přeloží zdrojové soubory klientské i serverové části aplikace na soubory `ipk-client` a `ipk-server` s definicí všech vývojových (debug) symbolů, které aktivují výpisy činnosti programu.
  - `make client` - přeloží zdrojové soubory pouze klientské části aplikace na `ipk-client`
  - `make debug-client` - přeloží zdrojové soubory pouze klientské části aplikace na `ipk-client` + debug symboly
  - `make server` - přeloží zdrojové soubory pouze serverové části aplikace na `ipk-server`
  - `make debug-server` - přeloží zdrojové soubory pouze serverové části aplikace na `ipk-server` + debug symboly

## gcc
Překlad pomocí `gcc` podporuje následující možnosti:

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
**Serverová část** programu vyžaduje pro správnou funkcionalitu možnost provádět
systémová volání pomocí funkce `popen`, práva čtení pro soubor `/etc/passwd` a možnost spouštět následující příkazy:
  - `/bin/cat`
  - `/bin/grep`
  - `/bin/awk`

# Omezení
**Server** dokáže obsloužit pouze jednoho klienta najednou. V případě více současných spojení, server nejdříve zpracuje požadavek prvního klienta v řadě a až pak se věnuje ostatním.

# Implementace
## Klient
Po zpracování přepínačů z příkazové řádky klient odesílá jeden řídící packet obsahující _typ vyhledávání_ (user info, home dir, list) a _obsah vyhledávání_ oddělené dvojtečkou - `{info|home|list}:[vyraz]` (př.: `info:xdolej08`, `home:xdolej08` či `list:x`).

Následně čeká na odpověď serveru. Přímo vypisuje všechny (úspěšné) přicházející data dokud není spojení ukončeno. V případě odpovědi indikující chybu na straně serveru vypisuje přednastavená chybová hlášení v závislosti na typu chyby.

## Server
Po připojení klienta server čeká na kontrolní data ve výše uvedeném formátu. Identifikuje typ vyžádaných informací a provede volání následujícího příkazu pomocí funkce `popen`:
```bash
cat /etc/passwd | grep %s | awk -F : `{ print $%d }`
```
V příkazu je `%s` nahrazeno _výrazem_ dle _typu_ vyhledávání a `%d` identifikátorem _sloupce_ následovně:
| typ    | výraz            | sloupec |
|--------|------------------|---------|
| `info` | `{input}:x:`     | 5       |
| `home` | `{input}:x:`     | 6       |
| `list` | `-E ^{input}.+$` | 1       |

Díky tomuto volání získá server stream obsahující vyžádané informace, následně cyklicky ze vzniklého streamu čte a přeposílá přečtená data klientovi. Při vyčerpání streamu ukončuje spojení s klientem.

### Speciální odpovědi
V případě, že je stream prázdný, odesílá server speciální odpověď `!empty` - označující, že vyhledávání bylo úspěšné ovšem nebyly nalezeny žádné výsledky.

V případě, že došlo k chybě při filtrování výsledků či jiné chybě v průběhu zpracování požadavku, odesílá server speciální odpověď `!err`.