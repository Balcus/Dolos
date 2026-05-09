# Detectare Plagiat (MOSS-like)

## Tema proiect

Se trimit un număr mare de fișiere (nu neapărat de cod sursă) și se vor compara din punct de vedere al conținutului, în scopul detectării plagiatului; aplicația va fi customized pentru a funcționa în diverse scenarii de utilizare. Outputul primit va fi un report conținând toate comparațiile efectuate (vezi spre exemplu MOSS). 

## Rulare proiect

Pentru a rula proiectul cat mai usor este nevoie de utilitara `just`:

[Just Github Link](https://github.com/casey/just)

Build proiect:

```sh
just configure
just build
```

Rulare server:

```sh
just server
```

Rulare client:

```sh
./build/Release/client -f demo/stack_original.c -f demo/stack_palg.c -f demo/other_stack.c
```

In cazul in care se fac schimbari se ruleaza din nou pasul de build cu:

```
just cbuild
```

Exemplu de raport:

```
=== Raport Detectare Plagiat ===
Fisiere analizate: 3

stack_original.c <-> stack_palg.c : 59.9%
stack_original.c <-> other_stack.c : 4.0%
stack_palg.c <-> other_stack.c : 1.0%
```
