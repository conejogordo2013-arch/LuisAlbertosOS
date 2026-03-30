# LuisAlbertoOS ASM -> C Migration (full subsystem pass)

## Scope delivered in this pass
- Converted **all kernel/drivers/apps ASM modules** into C equivalents under `c_port/`.
- Left ASM only at strict CPU/boot boundaries:
  - `c_port/boot/LABootL.asm` (BIOS real mode boot sector)
  - `c_port/boot/kernel_entry.asm` (GDT + CR0 protected-mode transition)
  - inline asm wrappers in `c_port/include/io.h` for port I/O and control registers.

## Per-module behavior mapping (ASM -> C)

### Boot & entry
- `boot/LABootL.asm` -> kept ASM (BIOS `int 10h`, `int 13h`, boot signature)
- `kernel/LAKernel.asm` transition logic -> `boot/kernel_entry.asm` + C kernel orchestration in `kernel/kernel.c`

### Kernel core
- `kernel/LAoskrnl.asm` -> `kernel/kernel.c`
- `kernel/LAApi.asm` + `kernel/LAApiApplications.asm` -> `kernel/api.c`
- `kernel/interrupts.lasys` -> `kernel/interrupts.c`
- `kernel/pmm.lasys` -> `kernel/pmm.c`
- `kernel/paging.lasys` -> `kernel/paging.c`
- `kernel/heap.lasys` -> `kernel/heap.c`
- `kernel/acpi.lasys` -> `kernel/acpi.c`
- `kernel/fs.lasys` -> `kernel/fs.c`
- `kernel/LACommand.asm` -> `kernel/shell.c`
- `kernel/netstack.lasys` -> `kernel/netstack.c`

### Drivers
- `drivers/keyboard.lasys` -> `drivers/keyboard.c`
- `drivers/ata.lasys` -> `drivers/ata.c`
- `drivers/ahci.lasys` -> `drivers/ahci.c`
- `drivers/rtl8139.lasys` -> `drivers/rtl8139.c`
- `drivers/ac97.lasys` -> `drivers/ac97.c`
- `drivers/vga_image.lasys` -> `drivers/vga_image.c`

### Apps
- `apps/sample1.laa.asm` -> `apps/sample1.c`
- `apps/textedit.laa.asm` -> `apps/textedit.c`
- `apps/taskmgr.laa.asm` -> `apps/taskmgr.c`
- `apps/LATextedit.asm` -> `apps/latextedit.c`

## Commands kept in C shell
`echo`, `help`, `cat`, `time`, `ls`, `stat`, `rm`, `touch`, `mkdir`, `mem`, `cpu`, `regs`, `clear`, `version/versión`, `edit`, `net`, `audio`, `img`, `dir`, `cd`.

## Build notes
Use freestanding cross toolchain:
```bash
cd c_port
make CROSS=i686-elf
```

Expected flags and link are already in `Makefile`:
- `-ffreestanding -nostdlib -m32`
- `linker/kernel.ld` with `ENTRY(kernel_entry)` and base `0x1000`

Run with QEMU:
```bash
qemu-system-i386 -drive format=raw,file=LuisAlbertoOS.img -serial stdio
```

## Validation stubs
Host-side harness placeholders remain in `c_port/tests/` for keyboard/ATA/AC97/VGA bring-up checks.
