# sunxi-image

Tool to generate allwinner sprite download compatible dlinfos, mbrs and images.
All generate commands take json formated configuration files.

## MBR

```shell
> sunxi-image mbr --help

Usage: mbr [--help] [--version] {dump,gen}

handle mbr

Optional arguments:
  -h, --help    shows help message and exits
  -v, --version prints version information and exits

Subcommands:
  dump          dump mbr
  gen           generate mbr
```

```json
[
    ...
    {
        "classname": "DISK",
        "name": "boot",
        "user_type": 32768
    }
    ...
]
```

## MBR

```shell
> sunxi-image dlinfo --help

Usage: dlinfo [--help] [--version] {dump,gen}

handle dlinfo

Optional arguments:
  -h, --help    shows help message and exits
  -v, --version prints version information and exits

Subcommands:
  dump          dump dlinfo
  gen           generate dlinfo
```

```json
[
    ...
    {
        "name": "boot",
        "dl_name": "BOOT_FEX00000000",
        "vf_name": "VBOOT_FEX0000000",
        "encrypt": false,
        "verify": false
    }
    ...
]
```

## Image

```shell
> sunxi-image image --help

Usage: image [--help] [--version] {dump,gen}

handle image

Optional arguments:
  -h, --help    shows help message and exits
  -v, --version prints version information and exits

Subcommands:
  dump          dump image
  gen           generate image
```

```json
[
    ...
    {
        "filename": "mbr.fex",
        "maintype": "12345678",
        "subtype": "1234567890___MBR"
    }
    ...
]
```
