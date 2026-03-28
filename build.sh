#!/bin/bash
echo "Building LuisAlbertoOS..."

mkdir -p boot kernel apps drivers libs bin

# 1. Bootloader
nasm -f bin boot/LABootL.asm -o bin/boot.bin

# 2. Kernel
nasm -f bin kernel/LAKernel.asm -o bin/kernel.bin

# 3. Apps
nasm -f bin apps/sample1.laa.asm -o bin/sample1.laa
nasm -f bin apps/textedit.laa.asm -o bin/textedit.laa
nasm -f bin apps/taskmgr.laa.asm -o bin/taskmgr.laa
nasm -f bin apps/LATextedit.asm -o bin/LATextedit.laa

# 4. Crear imagen SOLO UNA VEZ
dd if=/dev/zero of=LuisAlbertoOS.img bs=512 count=2880 status=none

# 5. Bootloader
dd if=bin/boot.bin of=LuisAlbertoOS.img conv=notrunc status=none

# 6. Kernel
dd if=bin/kernel.bin of=LuisAlbertoOS.img seek=1 conv=notrunc status=none

# 7. Apps (sectores manuales)
dd if=bin/sample1.laa   of=LuisAlbertoOS.img seek=19 conv=notrunc status=none
dd if=bin/textedit.laa  of=LuisAlbertoOS.img seek=21 conv=notrunc status=none
dd if=bin/taskmgr.laa   of=LuisAlbertoOS.img seek=23 conv=notrunc status=none
dd if=bin/LATextedit.laa of=LuisAlbertoOS.img seek=25 conv=notrunc status=none

# 8. Mock FS (opcional)
printf "test.txt\0\0\0\0\0\0\0\0\x1F\x00\x00\x00\x0F\x00\x00\x00" > bin/mock_dir.bin
dd if=bin/mock_dir.bin of=LuisAlbertoOS.img seek=30 conv=notrunc status=none

echo "Build complete."