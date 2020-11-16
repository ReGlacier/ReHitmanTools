LOCC - LOC Compiler & Decompiler
================================

Localization compiler & decompiler for Hitman Blood Money
 
Usage
=====

Decompile:
----------

```
LOCC.exe --from=M13_main.LOC --to=rel/M13_main.JSON --mode=decompile --pretty-json=on
```

Compile:
--------

```
LOCC.exe --from=rel/M13_main.JSON --to=rel/M13_main.LOC
```

Options:
========

 * `--f`, `--from` - Path to source file
 * `--t`, `--to` - Path to destination file
 * `--m`, `--mode` - Specify tool mode. Allowed values:
    * `compile` - Use tool as compiler. `--from` must be path to **LOC** file. `--to` must be path to JSON
    * `decompile` - Use tool as decompiler. ``--from` must be path to **JSON** file. `--to` must be path to LOC
 * `--g`, `--game` - Specify game name. Allowed values: 
    * `bloodmoney` - Hitman Blood Money LOC format - **supported**
    * `contracts` - Hitman Contracts LOC format - **in progress**
    * `2sa` - Hitman 2 Silent Assassin - **queued**
    * `a47` - Hitman Agent 47 - **queued**
 * `--p`, `--pretty`, `--pretty-json` - Specify JSON pretty printing. Allowed values:
    * `on` - Enable JSON pretty printing
    * `off` - Disable JSON pretty printing (default)