// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "includes/sunplus_unsp20soc.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"
#include "screen.h"


class mobigo_state : public generalplus_gpac800_game_state
{
public:
	mobigo_state(const machine_config &mconfig, device_type type, const char *tag) :
		generalplus_gpac800_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{ }

	void mobigo(machine_config &config);

protected:

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	required_device<generic_slot_device> m_cart;
};


static INPUT_PORTS_START( mobigo )
	PORT_START("IN0")
	PORT_START("IN1")
	PORT_START("IN2")
INPUT_PORTS_END



DEVICE_IMAGE_LOAD_MEMBER(mobigo_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");
	return image_init_result::PASS;
}

void mobigo_state::mobigo(machine_config &config)
{
	GPAC800(config, m_maincpu, 96000000/2, m_screen);  // Doesn't have GPnandnand header in NAND tho, so non-standard bootloader
	m_maincpu->porta_in().set(FUNC(mobigo_state::porta_r));
	m_maincpu->portb_in().set(FUNC(mobigo_state::portb_r));
	m_maincpu->portc_in().set(FUNC(mobigo_state::portc_r));
	m_maincpu->porta_out().set(FUNC(mobigo_state::porta_w));
	m_maincpu->space_read_callback().set(FUNC(mobigo_state::read_external_space));
	m_maincpu->space_write_callback().set(FUNC(mobigo_state::write_external_space));
	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
	m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
	m_maincpu->set_bootmode(0); // boot from internal ROM (NAND bootstrap)
	m_maincpu->set_cs_config_callback(FUNC(mobigo_state::cs_callback));

	m_maincpu->nand_read_callback().set(FUNC(mobigo_state::read_nand));

	FULL_MEMORY(config, m_memory).set_map(&mobigo_state::cs_map_base);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "mobigo_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(mobigo_state::cart_load));
	//m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("mobigo_cart");
}

ROM_START( mobigo2 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // doesn't have GPnandnand header in NAND, so bootstrap is likely custom

	ROM_REGION( 0x8400000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "mobigo2_bios_ger.bin", 0x00000, 0x8400000, CRC(d5ab613d) SHA1(6fb104057dc3484fa958e2cb20c5dd0c19589f75) ) // SPANSION S34ML01G100TF100
ROM_END


CONS( 200?, mobigo2, 0, 0, mobigo,  mobigo, mobigo_state, nand_init840, "VTech", "MobiGo 2 (Germany)", MACHINE_IS_SKELETON )
