// should only be included once
#define IOHANDLER(off, x) new(&iologic.at(off-0xff00)) iowrite_t{iowrite_##x, ioread_##x};

namespace gbc
{
  struct iowrite_t {
    using write_handler_t = void(*) (IO&, uint16_t, uint8_t);
    using read_handler_t  = uint8_t(*) (IO&, uint16_t);
    const write_handler_t on_write = nullptr;
    const read_handler_t  on_read  = nullptr;
  };
  static std::array<iowrite_t, 128> iologic = {};

  void iowrite_JOYP(IO& io, uint16_t addr, uint8_t value)
  {
    // TODO: joypad functionality
    printf("P1/JOYP register 0x%04x write 0x%02x\n", addr, value);
  }
  uint8_t ioread_JOYP(IO& io, uint16_t)
  {
    // logic
    return 0x0f;
  }

  void iowrite_DIV(IO& io, uint16_t, uint8_t)
  {
    // writing to DIV resets it to 0
    io.reg(IO::REG_DIV) = 0;
    io.machine().break_now();
  }
  uint8_t ioread_DIV(IO& io, uint16_t)
  {
    return io.reg(IO::REG_DIV);
  }

  void iowrite_DMA(IO& io, uint16_t addr, uint8_t value)
  {
    auto& memory = io.machine().memory;
    uint16_t src = value << 8;
    uint16_t dst = 0xfe00;
    //printf("DMA copy from 0x%04x to 0x%04x\n", src, dst);
    for (int i = 0; i < 160; i++) {
      memory.write8(dst++, memory.read8(src++));
    }
    // it does take time to do all this work
    io.machine().cpu.incr_cycles(671);
    //assert(0 && "DMA operation completed");
  }
  uint8_t ioread_DMA(IO& io, uint16_t addr)
  {
    return io.reg(addr);
  }

  void iowrite_HDMA(IO& io, uint16_t addr, uint8_t value)
  {
    printf("HDMA 0x%04x write 0x%02x\n", addr, value);
    assert(0 && "HDMA registers written to");
  }
  uint8_t ioread_HDMA(IO& io, uint16_t addr)
  {
    return io.reg(addr);
  }

  void iowrite_SND_ONOFF(IO& io, uint16_t addr, uint8_t value)
  {
    // TODO: writing bit7 should clear all sound registers
    printf("NR52 Sound ON/OFF 0x%04x write 0x%02x\n", addr, value);
    io.reg(IO::REG_NR52) |= value & 0x80;
    //assert(0 && "NR52 Sound ON/OFF register write");
  }
  uint8_t ioread_SND_ONOFF(IO& io, uint16_t addr)
  {
    return io.reg(addr);
  }


  __attribute__((constructor))
  static void set_io_handlers() {
    IOHANDLER(IO::REG_P1,    JOYP);
    IOHANDLER(IO::REG_DIV,   DIV);
    IOHANDLER(IO::REG_DMA,   DMA);
    IOHANDLER(IO::REG_HDMA1, HDMA);
    IOHANDLER(IO::REG_HDMA2, HDMA);
    IOHANDLER(IO::REG_HDMA3, HDMA);
    IOHANDLER(IO::REG_HDMA4, HDMA);
    IOHANDLER(IO::REG_HDMA5, HDMA);
    IOHANDLER(IO::REG_NR52,  SND_ONOFF);
  }
}