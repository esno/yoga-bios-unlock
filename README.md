# yoga-bios-unlock

Based on FlyGoat's work to unlock the BIOS advanced menu documented [here](https://zhuanlan.zhihu.com/p/184982689),
I wrote that tool to unlock my yoga laptop without using a proprietary software which is only available on Windows.

I'd like to thank FlyGoat a lot for giving me the right direction on how to translate his guide into low-level functions
provided by glibc (see also [FlyGoats gist](https://gist.github.com/FlyGoat/5f0dba5b5ccc1b6ab73023489e1e989a)).

This tool will unlock the advanced menu in your Lenovo Yoga Slim 7 14ARE05.

To unlock the advanced menu it's necessary to change the data field on port `0x73` at index `0xf7`.
Port `0x72` is the index port that defines which value at `0x73` will be accessed.

                                \/
         | 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
      ---+------------------------------------------------
      00 | 21 bc 21 63 00 00 5a a5 18 70 f6 7c 00 f5 19 00
      10 | 80 00 a6 20 00 44 00 02 00 00 94 80 89 00 e0 49
      20 | 00 0d 04 30 21 00 24 00 00 10 00 04 00 00 04 ec
      30 | 55 55 55 55 ff 07 00 20 29 00 8c 00 0a a8 00 08
      40 | f4 00 00 00 01 20 01 01 08 00 00 00 10 11 00 00
      50 | 00 00 80 00 00 00 41 00 00 00 00 00 ff 07 00 00
      60 | 00 00 00 00 00 10 00 20 02 00 00 00 00 00 00 00
      70 | 00 00 00 00 08 01 08 00 00 00 00 00 86 22 00 00
      80 | c0 0c 00 80 09 52 00 00 00 00 00 00 00 00 00 00
      90 | 00 00 00 00 08 00 00 80 00 00 04 40 00 80 00 00
      a0 | 00 00 00 00 00 00 00 00 20 40 00 00 20 00 01 00
      b0 | 00 00 08 20 00 00 40 00 00 00 00 00 00 42 00 00
      c0 | 02 02 00 00 00 40 00 00 00 00 80 80 00 00 00 40
      d0 | 00 40 14 00 02 00 82 00 00 00 00 80 81 08 00 80
      e0 | 00 02 00 01 00 00 00 00 00 00 00 00 00 00 01 00
    > f0 | 29 a4 a7 a7 00 00 00 77 e0 00 24 04 00 10 10 00
                                /\

## Disclaimer

> This tool may eat your cat, burn your house or do anything else beside the expected task.
> So use it at your own risk and be aware that you're playing around with your BIOS which may end in a bricked device.

## Compatibility

    | Version  | Missing features    |
    | -------- | ------------------- |
    | DMCN27WW |                     |
    | DMCN29WW |                     |
    | DMCN32WW |                     |
    | DMCN34WW | no XFR enhancements |
    | DMCN35WW | no XFR enhancements |
    | DMCN36WW | no XFR enhancements |
    | DMCN38WW | no XFR enhancements |
    | DMCN39WW | no XFR enhancements |

If you're aware of any further differences in BIOS version please raise a [ticket](https://github.com/esno/yoga-bios-unlock/issues/new)
or open a pull request.

### BIOS downgrade

Boot into BIOS and go to `Configuration` tab.
There is an option called `BIOS Back Flash` which is disabled by default.

The product page of [lenovo's Yoga Slim 7](https://pcsupport.lenovo.com/de/de/products/laptops-and-netbooks/ideapad-s-series-netbooks/slim-7-14are05/downloads/driver-list/component?name=BIOS)
provides the latest BIOS version. Thanks to lenovo they do not obfuscate their download links therefore
it's quiet easy to estimate former versions and download them.

Former BIOS versions are available at lenovo servers (Readme and installation instructions are also available):

* [DMCN27WW](https://download.lenovo.com/consumer/mobiles/dmcn27ww.exe) ([ReadMe](https://download.lenovo.com/consumer/mobiles/dmcn27ww.txt))
* [DMCN29WW](https://download.lenovo.com/consumer/mobiles/dmcn29ww.exe) ([ReadMe](https://download.lenovo.com/consumer/mobiles/dmcn29ww.txt))
* [DMCN32WW](https://download.lenovo.com/consumer/mobiles/dmcn32ww.exe) ([ReadMe](https://download.lenovo.com/consumer/mobiles/dmcn32ww.txt))
* [DMCN34WW](https://download.lenovo.com/consumer/mobiles/dmcn34ww.exe) ([ReadMe](https://download.lenovo.com/consumer/mobiles/dmcn34ww.txt))
* [DMCN35WW](https://download.lenovo.com/consumer/mobiles/dmcn35ww.exe) ([ReadMe](https://download.lenovo.com/consumer/mobiles/dmcn35ww.txt))
* [DMCN36WW](https://download.lenovo.com/consumer/mobiles/dmcn36ww.exe) ([ReadMe](https://download.lenovo.com/consumer/mobiles/dmcn36ww.txt))
* [DMCN38WW](https://download.lenovo.com/consumer/mobiles/dmcn38ww.exe) ([ReadMe](https://download.lenovo.com/consumer/mobiles/dmcn38ww.txt))
* [DMCN39WW](https://download.lenovo.com/consumer/mobiles/dmcn39ww.exe) ([ReadMe](https://download.lenovo.com/consumer/mobiles/dmcn39ww.txt))

## Known Issues

If you hit the following issue please disable secure boot first and try again:

    Can't set I/O privilege level (Operation not permitted)

## Usage

> BIOS versions greater or equal to `DMCN34WW` are supported but hiding some menus.
> Using `DMCN32WW` is recommended.

### Build from source

    git clone https://github.com/esno/yoga-bios-unlock.git
    cd ./yoga-bios-unlock
    make

#### Add new bios/board versions

New supported bios and board versions can be added by appending the version string either to [bios-versions.txt](https://github.com/esno/yoga-bios-unlock/blob/master/bios-versions.txt)
or to [board-versions.txt](https://github.com/esno/yoga-bios-unlock/blob/master/board-versions.txt).
Afterwards the headers needs to be regenerated with

    make headers

### Verify current value of data port

    # ./yoga-bios-unlock --read
    Run in read mode
    Be aware that readmode temporarily changes value of port 0x72 to index 0xf7
    WARNING: use at your own risk!
    Agree? (y/n) y
    Port 0x72 is 0xf4 and will be set to 0xf7
    Port 0x73 is 0x00 and would be set to 0x77

### Unlock your BIOS

    # ./yoga-bios-unlock --unlock
    Run in unlock mode
    WARNING: use at your own risk!
    Agree? (y/n) y
    Port 0x72 is 0xf4 and will be set to 0xf7

### Lock your BIOS

    # ./yoga-bios-unlock --lock
    Run in lock mode
    WARNING: use at your own risk!
    Agree? (y/n) y
    Port 0x72 is 0xf4 and will be set to 0xf7

### Ignore platform check results

If you know that you're on Lenovo Yoga Slim 7 (14ARE05) but either board version might differ
or BIOS version is currently not supported by this tool you can enforce an unlock.
If you do please open a ticket/PR to notify me that your current BIOS version/board version works well.

#### Enforce unlock your BIOS

    # ./yoga-bios-unlock --unlock --force
    Run in unlock mode
    Platform checks are disabled - hopefully you know what you do
    WARNING: use at your own risk!
    Agree? (y/n) y
    Port 0x72 is 0xf4 and will be set to 0xf7
