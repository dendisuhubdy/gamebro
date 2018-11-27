#include "memory.hpp"
#include "machine.hpp"
#include "bios.hpp"

namespace gbc
{
  Memory::Memory(Machine& mach, std::vector<uint8_t> rom)
    : m_machine(mach), m_mbc{*this, std::move(rom)}
  {
    assert(m_mbc.rom_valid());
    this->m_bootrom_enabled = false;
    // set CGB mode when ROM supports it
    const uint8_t cgb = this->read8(0x143);
    machine().m_cgb_mode = cgb & 0x80;
  }
  void Memory::reset()
  {
    //this->disable_bootrom();
    //m_mbc.reset();
  }
  void Memory::install_rom(std::vector<uint8_t> rom) {
    this->m_mbc.install_rom(std::move(rom));
  }
  void Memory::disable_bootrom() {
    this->m_bootrom_enabled = false;
  }
  void Memory::set_wram_bank(uint8_t bank)
  {
    this->m_mbc.set_wrambank(bank);
  }

  uint8_t Memory::read8(uint16_t address)
  {
    for (auto& func : m_read_breakpoints) {
      func(*this, address, 0x0);
    }
    if (this->is_within(address, ProgramArea)) {
      if (address >= 0x100 || this->m_bootrom_enabled == false)
        return m_mbc.read(address);
      else
        return dmg0_rom.at(address);
    }
    else if (this->is_within(address, VideoRAM)) {
      // cant read from Video RAM when working on scanline
      //if (UNLIKELY(machine().gpu.get_mode() == 3))
      //    return 0xff;
      const uint16_t offset = machine().gpu.video_offset();
      return m_video_ram.at(offset + address - VideoRAM.first);
    }
    else if (this->is_within(address, BankRAM)) {
      return m_mbc.read(address);
    }
    else if (this->is_within(address, WorkRAM)) {
      return m_mbc.read(address);
    }
    else if (this->is_within(address, EchoRAM)) {
      return m_mbc.read(address);
    }
    else if (this->is_within(address, OAM_RAM)) {
      // TODO: return 0xff when rendering?
      return m_oam_ram.at(address - OAM_RAM.first);
    }
    else if (this->is_within(address, IO_Ports)) {
      return machine().io.read_io(address);
    }
    else if (this->is_within(address, ZRAM)) {
      return m_zram.at(address - ZRAM.first);
    }
    else if (address == InterruptEn) {
      return machine().io.read_io(address);
    }
    printf(">>> Invalid memory read at 0x%04x\n", address);
    return 0xff;
  }

  void Memory::write8(uint16_t address, uint8_t value)
  {
    for (auto& func : m_write_breakpoints) {
      func(*this, address, value);
    }
    if (this->is_within(address, ProgramArea)) {
      m_mbc.write(address, value);
      return;
    }
    else if (this->is_within(address, VideoRAM)) {
      if (machine().gpu.video_writable())
      {
        const uint16_t offset = machine().gpu.video_offset();
        m_video_ram.at(offset + address - VideoRAM.first) = value;
      }
      return;
    }
    else if (this->is_within(address, BankRAM)) {
      m_mbc.write(address, value);
      return;
    }
    else if (this->is_within(address, WorkRAM)) {
      m_mbc.write(address, value);
      return;
    }
    else if (this->is_within(address, EchoRAM)) {
      m_mbc.write(address, value);
      return;
    }
    else if (this->is_within(address, OAM_RAM)) {
      m_oam_ram.at(address - OAM_RAM.first) = value;
      return;
    }
    else if (this->is_within(address, IO_Ports)) {
      machine().io.write_io(address, value);
      return;
    }
    else if (this->is_within(address, ZRAM)) {
      m_zram.at(address - ZRAM.first) = value;
      return;
    }
    else if (address == InterruptEn) {
      machine().io.write_io(address, value);
      return;
    }
    printf(">>> Invalid memory write at 0x%04x, value 0x%x\n",
           address, value);
  }

  uint16_t Memory::read16(uint16_t address) {
    return read8(address) | read8(address+1) << 8;
  }
  void Memory::write16(uint16_t address, uint16_t value) {
    write8(address+0, value & 0xff);
    write8(address+1, value >> 8);
  }

  bool Memory::double_speed() const noexcept
  {
    return m_speed_factor != 1;
  }
  int  Memory::speed_factor() const noexcept
  {
    return m_speed_factor;
  }
  void Memory::do_switch_speed()
  {
    auto& speed = machine().io.reg(IO::REG_KEY1);
    if (speed) {
      speed = 0x0;
      this->m_speed_factor = 1;
    }
    else {
      speed = 0x80;
      this->m_speed_factor = 2;
    }
  }
}
