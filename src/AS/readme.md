# 生成目标代码

<!-- ## 技术路线选择
- 使用nasm汇编作为目标代码，将四元式形式的中间代码经过优化后生成.asm代码。
- 在Linux操作系统中：
  - 使用nasm汇编器将.asm汇编生成elf文件
`
nasm -f elf hello.asm
`
  - 使用gcc中的ld链接器，将elf文件链接生成可执行文件
`
ld -m elf_i386 hello.o -o hello
`

## 中间代码转nasm汇编说明 -->

## DAG局部优化
- 代码块的划分
- 构建DAG图进行局部优化
- 生成优化后的四元式写入文件

## 生成Mips代码
- 根据四元式生成Mips代码
- 在Mars中运行Mips代码
