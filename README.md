# Danish compiler
Why not code in your native language?

## Example
```
Offentlig Funktion Hovedsagelig tilbagegiver heltal.
Goddag.
    Lad A være et heltal.
    Gem toogfyrre i A.
    Sæt A i anden, og print det.
Farvel.

Offentlig Funktion Sæt (A som heltal) i anden tilbagegiver heltal.
Goddag.
    Lad C være et heltal.
    Gang A med A, og gem det i C.
    Tilbagegiv C.
Farvel.
```

## Build
Clang:
```
clang main.c -o dansk.exe
```

MSVC:
```
cl main.c /Fedansk.exe /std:c17
```
