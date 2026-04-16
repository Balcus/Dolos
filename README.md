# Detectare Plagiat (MOSS-like)

## Tema proiect

Se trimit un număr mare de fișiere (nu neapărat de cod sursă) și se vor compara din punct de vedere al conținutului, în scopul detectării plagiatului; aplicația va fi customized pentru a funcționa în diverse scenarii de utilizare. Outputul primit va fi un report conținând toate comparațiile efectuate (vezi spre exemplu MOSS). 

## Rulare proiect

Pentru a rula proiectul cat mai usor este nevoie de utilitara `just`:

[Github Link](https://github.com/casey/just)

Pentru instalare Windows (n-am dar sper sa mearga):

```
winget install Casey.Just
```

Build proiect:

```
just configure
just build
```

Rulare server:

```
just server
```

Rulare client:

```
just client
```

In cazul in care se fac schimbari se ruleaza din nou pasul de build cu:

```
just cbuild
```