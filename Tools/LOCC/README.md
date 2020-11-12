LOCC - LOC Compiler & Decompiler
--------------------------------

 Localization compiler & decompiler for Hitman Blood Money
 
Usage
-----

```
LOCC.exe --from=[Path to localization in JSON] --to=[Path to final LOC file] --test=1
```

Options:

 * `--from` - path to source JSON file
 * `--to` - path to final LOC file
 * `--test` - iterate over all keys and try to find it via `ResourceCollection::Lookup`. **It takes a lot of time!**
 * `--decompile-to-json` - decompile file from `--from` as LOC and save to specified path in JSON format
 * `--help` - show help info (all other options will be ignored)