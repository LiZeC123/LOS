%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR

  mov byte [gs:0x00], 'L'
  mov byte [gs:0x01], 0xA4

  mov byte [gs:0x02], 'O'
  mov byte [gs:0x03], 0xA4

  mov byte [gs:0x04], 'A'
  mov byte [gs:0x05], 0xA4

  mov byte [gs:0x06], 'D'
  mov byte [gs:0x07], 0xA4