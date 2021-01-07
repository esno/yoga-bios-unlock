# yoga-bios-unlock

Based on FlyGoat's work to unlock the BIOS advanced menu documented [here](https://zhuanlan.zhihu.com/p/184982689),
I wrote that tool to unlock my yoga laptop without using a proprietary software which is only available on Windows.

I'd like to thank FlyGoat a lot for giving me the right direction on how to translate his guide into low-level functions
provided by glibc.

This tool will unlock the advanced menu in your Lenovo Yoga Slim 7 14ARE05.

## Disclaimer

This tool may eat your cat, burn your house or do anything else beside the expected task.
So use it at your own risk and be aware that you're playing around with your BIOS which may end in a bricked device.

## Usage

    git clone https://github.com/esno/yoga-bios-unlock.git
    cd ./yoga-bios-unlock
    make

    # ./yoga-bios-unlock --dry-run
    Enabled dry-run
    WARNING: use at your own risk!
    Agree? (y/Y) y
    Port 0x72 is 0xf4 and would be set to 0xf7
    Port 0x73 is 0x00 and would be set to 0x77

    # ./yoga-bios-unlock          
    WARNING: use at your own risk!
    Agree? (y/Y) y
