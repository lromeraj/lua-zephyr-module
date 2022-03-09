#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <drivers/eeprom.h>
#include <drivers/flash.h>
#include <drivers/uart.h>

#include "lua/lauxlib.h"
#include "lua/lua.h"

//////////////
/// BUFFER ///
//////////////

#define T_USERDATA_FAT 1
#define UD_GET_BUFFER(ix) ud_fat_t * buf = lua_touserdata(L, ix); \
    luaL_argcheck(L, buf->type == T_USERDATA_FAT, ix, "`buffer' expected"); \
    luaL_argcheck(L, buf->ptr != NULL, ix, "`buffer' does not exist");
typedef struct {
    int type;
    void * ptr;
    size_t size;
} ud_fat_t;

static int lua_alloc_buffer(lua_State * L) {
    int size = luaL_checkinteger(L, 1);
    ud_fat_t * ptr = lua_newuserdata(L, sizeof(ud_fat_t) + size);
    ptr->type = T_USERDATA_FAT;
    ptr->ptr  = (void*)(ptr+1);
    ptr->size = size;
    return 1;
}

static int lua_buffer_size(lua_State * L) {
    UD_GET_BUFFER(1);
    lua_pushinteger(L, buf->size);
    return 1;
}

static int lua_get_buffer_u8(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    luaL_argcheck(L, sizeof(uint8_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    lua_pushinteger(L, ((uint8_t *) buf->ptr)[ix]);
    return 1;
}

static int lua_get_buffer_s8(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    luaL_argcheck(L, sizeof(int8_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    lua_pushinteger(L, ((int8_t *) buf->ptr)[ix]);
    return 1;
}

static int lua_get_buffer_u16(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    luaL_argcheck(L, sizeof(uint16_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    lua_pushinteger(L, ((uint16_t *) buf->ptr)[ix]);
    return 1;
}

static int lua_get_buffer_s16(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    luaL_argcheck(L, sizeof(int16_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    lua_pushinteger(L, ((int16_t *) buf->ptr)[ix]);
    return 1;
}

static int lua_get_buffer_u32(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    luaL_argcheck(L, sizeof(uint32_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    lua_pushinteger(L, ((uint32_t *) buf->ptr)[ix]);
    return 1;
}

static int lua_get_buffer_s32(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    luaL_argcheck(L, sizeof(int32_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    lua_pushinteger(L, ((int32_t *) buf->ptr)[ix]);
    return 1;
}

static int lua_set_buffer_u8(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    unsigned value = luaL_checkinteger(L, 3);
    luaL_argcheck(L, sizeof(uint8_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    ((uint8_t *) buf->ptr)[ix] = value;
    return 0;
}

static int lua_set_buffer_s8(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    luaL_argcheck(L, sizeof(int8_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    ((int8_t *) buf->ptr)[ix] = value;
    return 0;
}

static int lua_set_buffer_u16(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    unsigned value = luaL_checkinteger(L, 3);
    luaL_argcheck(L, sizeof(uint16_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    ((uint16_t *) buf->ptr)[ix] = value;
    return 0;
}

static int lua_set_buffer_s16(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    luaL_argcheck(L, sizeof(int16_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    ((int16_t *) buf->ptr)[ix] = value;
    return 0;
}

static int lua_set_buffer_u32(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    unsigned value = luaL_checkinteger(L, 3);
    luaL_argcheck(L, sizeof(uint32_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    ((uint32_t *) buf->ptr)[ix] = value;
    return 0;
}

static int lua_set_buffer_s32(lua_State * L) {
    UD_GET_BUFFER(1);
    unsigned ix = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    luaL_argcheck(L, sizeof(int32_t) * ix < buf->size, ix, "`index' is out of scope for the buffer.");
    ((int32_t *) buf->ptr)[ix] = value;
    return 0;
}

//////////////
/// DEVICE ///
//////////////

#define T_USERDATA_DEVICE 2
#define UD_GET_DEVICE(ix) ud_device_t * dev = lua_touserdata(L, ix); \
    luaL_argcheck(L, dev->type == T_USERDATA_DEVICE, ix, "`device' expected"); \
    luaL_argcheck(L, dev->device != NULL, ix, "`device' does not exist");
typedef struct {
    int type;
    struct device* device;
} ud_device_t;

static int lua_device_get_binding(lua_State * L) {
    char * name = luaL_checkstring(L, 1);
    ud_device_t * dev = lua_newuserdata(L, sizeof(ud_device_t));
    dev->type = T_USERDATA_DEVICE;
    dev->device = device_get_binding(name);
    return 1;
}

static int lua_device_is_ready(lua_State * L) {
    UD_GET_DEVICE(1);
    lua_pushboolean(L, device_is_ready(dev->device));
    return 1;
}

///////////
/// ADC ///
///////////

//////////////
/// EEPROM ///
//////////////

/* static int lua_eeprom_read(lua_State * L) { */
/*     UD_GET_DEVICE(1); */
/*     UD_GET_BUFFER(2); */
/*     unsigned int offset = luaL_checkinteger(L, 3); */

/*     eeprom_read(dev->device, offset, buf->ptr, buf->size); */
/*     return 0; */
/* } */

/* static int lua_eeprom_write(lua_State * L) { */
/*     UD_GET_DEVICE(1); */
/*     UD_GET_BUFFER(2); */
/*     unsigned int offset = luaL_checkinteger(L, 3); */

/*     eeprom_write(dev->device, offset, buf->ptr, buf->size); */
/*     return 0; */
/* } */

/* static int lua_eeprom_get_size(lua_State * L) { */
/*     UD_GET_DEVICE(1); */
/*     lua_pushinteger(L, eeprom_get_size(dev->device)); */
/*     return 1; */
/* } */

/////////////
/// FLASH ///
/////////////

static int lua_flash_read(lua_State * L) {
    UD_GET_DEVICE(1);
    UD_GET_BUFFER(2);
    unsigned int offset = luaL_checkinteger(L, 3);
    flash_read(dev->device, offset, buf->ptr, buf->size);
    return 0;
}

static int lua_flash_write(lua_State * L) {
    UD_GET_DEVICE(1);
    UD_GET_BUFFER(2);
    unsigned int offset = luaL_checkinteger(L, 3);

    flash_write(dev->device, offset, buf->ptr, buf->size);
    return 0;
}

static int lua_flash_erase(lua_State * L) {
    UD_GET_DEVICE(1);
    unsigned int offset = luaL_checkinteger(L, 2);
    unsigned int size = luaL_checkinteger(L, 3);
    flash_erase(dev->device, offset, size);
    return 0;
}

/* static int lua_flash_get_page_count(lua_State * L) { */
/*     UD_GET_DEVICE(1); */
/*     lua_pushinteger(L, flash_get_page_count(dev->device)); */
/*     return 1; */
/* } */

///////////
/// I2C ///
///////////

static int lua_i2c_configure(lua_State * L) {
    UD_GET_DEVICE(1);
    int flags = luaL_checkinteger(L, 2);
    int ret = i2c_configure(dev->device, flags);
    lua_pushinteger(L, ret);
    return 1;
}

// static int lua_i2c_get_config(lua_State * L) {
//     ud_device_t * dev = lua_touserdata(L,1);
//     int flags;
//     i2c_get_config(dev->device, &flags);
//     lua_pushinteger(L, flags);
//     return 1;
// }

// TODO i2c_write_read
// TODO i2c_read
// TODO i2c_write

////////////
/// GPIO ///
////////////

static int lua_gpio_pin_interrupt_configure(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    int flags = luaL_checkinteger(L, 3);
    int ret = gpio_pin_interrupt_configure(dev->device, pin, flags);
    lua_pushinteger(L, ret);
    return 1;
}

static int lua_gpio_pin_configure(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    int flags = luaL_checkinteger(L, 3);
    int ret = gpio_pin_configure(dev->device, pin, flags);
    lua_pushinteger(L, ret);
    return 1;
}

static int lua_gpio_port_get_raw(lua_State * L) {
    UD_GET_DEVICE(1);
    int value;
    gpio_port_get_raw(dev->device, &value);
    lua_pushinteger(L, value);
    return 1;
}

static int lua_gpio_port_get(lua_State * L) {
    UD_GET_DEVICE(1);
    int value;
    gpio_port_get(dev->device, &value);
    lua_pushinteger(L, value);
    return 1;
}

static int lua_gpio_port_set_masked_raw(lua_State * L) {
    UD_GET_DEVICE(1);
    int mask = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    gpio_port_set_masked_raw(dev->device, mask, value);
    return 0;
}

static int lua_gpio_port_set_masked(lua_State * L) {
    UD_GET_DEVICE(1);
    int mask = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    gpio_port_set_masked(dev->device, mask, value);
    return 0;
}

static int lua_gpio_port_set_bits_raw(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    gpio_port_set_bits_raw(dev->device, pin);
    return 0;
}

static int lua_gpio_port_set_bits(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    gpio_port_set_bits(dev->device, pin);
    return 0;
}

static int lua_gpio_port_clear_bits_raw(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    gpio_port_clear_bits_raw(dev->device, pin);
    return 0;
}

static int lua_gpio_port_clear_bits(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    gpio_port_clear_bits(dev->device, pin);
    return 0;
}

static int lua_gpio_port_toggle_bits(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    gpio_port_toggle_bits(dev->device, pin);
    return 0;
}

static int lua_gpio_port_set_clr_bits_raw(lua_State * L) {
    UD_GET_DEVICE(1);
    int setpin = luaL_checkinteger(L, 2);
    int clrpin = luaL_checkinteger(L, 3);
    gpio_port_set_clr_bits_raw(dev->device, setpin, clrpin);
    return 0;
}

static int lua_gpio_port_set_clr_bits(lua_State * L) {
    UD_GET_DEVICE(1);
    int setpin = luaL_checkinteger(L, 2);
    int clrpin = luaL_checkinteger(L, 3);
    gpio_port_set_clr_bits(dev->device, setpin, clrpin);
    return 0;
}

static int lua_gpio_pin_get_raw(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    int val = gpio_pin_get_raw(dev->device, pin);
    lua_pushinteger(L, val);
    return 1;
}

static int lua_gpio_pin_get(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    int val = gpio_pin_get(dev->device, pin);
    lua_pushinteger(L, val);
    return 1;
}

static int lua_gpio_pin_set_raw(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    gpio_pin_set_raw(dev->device, pin, value);
    return 0;
}

static int lua_gpio_pin_set(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    gpio_pin_set(dev->device, pin, value);
    return 0;
}

static int lua_gpio_pin_toggle(lua_State * L) {
    UD_GET_DEVICE(1);
    int pin = luaL_checkinteger(L, 2);
    gpio_pin_toggle(dev->device, pin);
    return 0;
}

////////////////////////////
/// HARDWARE INFORMATION ///
////////////////////////////

//static int lua_hwinfo_get_reset_cause(lua_State * L) {
//    int cause;
//    hwinfo_get_reset_cause(&cause);
//    lua_pushinteger(L, cause);
//    return 1;
//}
//
//static int lua_hwinfo_get_supported_reset_cause(lua_State * L) {
//    int cause;
//    hwinfo_get_supported_reset_cause(&cause);
//    lua_pushinteger(L, cause);
//    return 1;
//}
//
//static int lua_hwinfo_clear_reset_cause() {
//    hwinfo_clear_reset_cause();
//    return 0;
//}

//////////////
/// PINMUX ///
//////////////

static inline int lua_pinmux_pin_set(lua_State * L) {
    UD_GET_DEVICE(1);
    unsigned int pin = luaL_checkinteger(L, 2);
    unsigned int func = luaL_checkinteger(L, 3);
    lua_pushinteger(L, pinmux_pin_set(dev->device, pin, func));
    return 1;
}

static inline int lua_pinmux_pin_get(lua_State * L) {
    UD_GET_DEVICE(1);
    unsigned int pin = luaL_checkinteger(L, 2);
    unsigned int func;
    pinmux_pin_get(dev->device, pin, &func);
    lua_pushinteger(L, func);
    return 1;
}

static inline int lua_pinmux_pin_pullup(lua_State * L) {
    UD_GET_DEVICE(1);
    unsigned int pin = luaL_checkinteger(L, 2);
    unsigned int func = luaL_checkinteger(L, 3);
    lua_pushinteger(L, pinmux_pin_pullup(dev->device, pin, func));
    return 1;
}

static inline int lua_pinmux_pin_input_enable(lua_State * L) {
    UD_GET_DEVICE(1);
    unsigned int pin = luaL_checkinteger(L, 2);
    unsigned int func = luaL_checkinteger(L, 3);
    lua_pushinteger(L, pinmux_pin_input_enable(dev->device, pin, func));
    return 1;
}


///////////
/// SPI ///
///////////


//static int lua_spi_transceive(lua_State * L) {}
//static int lua_spi_read(lua_State * L) {}
//static int lua_spi_write(lua_State * L) {}
//static int lua_spi_transceive_async(lua_State * L) {}
//static int lua_spi_read_async(lua_State * L) {}
//static int lua_spi_write_async(lua_State * L) {}

////////////
/// UART ///
////////////

static int lua_uart_err_check(lua_State * L) {
    UD_GET_DEVICE(1);
    lua_pushinteger(L, uart_err_check(dev->device));
    return 1;
}

static int lua_uart_configure(lua_State * L) {
    UD_GET_DEVICE(1);
    int baudrate = luaL_checkinteger(L, 2);
    struct uart_config cfg;
    cfg.baudrate = baudrate;
    cfg.parity = UART_CFG_PARITY_NONE;
    cfg.stop_bits = UART_CFG_STOP_BITS_1;
    cfg.data_bits = UART_CFG_DATA_BITS_8;
    cfg.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;
    lua_pushinteger(L, uart_configure(dev->device, &cfg));
    return 1;
}

static int lua_uart_config_get(lua_State * L) {
    UD_GET_DEVICE(1);
    struct uart_config cfg;
    uart_config_get(dev->device, &cfg);
    lua_pushinteger(L, cfg.baudrate);
    return 1;
}

static int lua_uart_line_ctrl_set(lua_State * L) {
    UD_GET_DEVICE(1);
    int ctrl = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    uart_line_ctrl_set(dev->device, ctrl, value);
    return 0;
}

static int lua_uart_line_ctrl_get(lua_State * L) {
    UD_GET_DEVICE(1);
    int ctrl = luaL_checkinteger(L, 2);
    int value;
    uart_line_ctrl_get(dev->device, ctrl, &value);
    lua_pushinteger(L, value);
    return 1;
}

static int lua_uart_poll_in(lua_State * L) {
    UD_GET_DEVICE(1);
    unsigned char c;
    int ret = uart_poll_in(dev->device, &c);
    lua_pushinteger(L, c);
    lua_pushinteger(L, ret);
    return 2;
}

//static int lua_uart_poll_in_u16(lua_State * L) {
    //UD_GET_DEVICE(1);
    //uint16_t c;
    //int ret = uart_poll_in_u16(dev->device, &c);
    //lua_pushinteger(L, c);
    //lua_pushinteger(L, ret);
    //return 2;
//}

static int lua_uart_poll_out(lua_State * L) {
    UD_GET_DEVICE(1);
    unsigned char c = luaL_checkinteger(L, 2);
    uart_poll_out(dev->device, c);
    return 0;
}

//static int lua_uart_poll_out_u16(lua_State * L) {
    //UD_GET_DEVICE(1);
    //uint16_t c = luaL_checkinteger(L, 2);
    //uart_poll_out_u16(dev->device, c);
    //return 0;
//}

//static int lua_uart_fifo_fill(lua_State * L) {
//    UD_GET_DEVICE(1);
//    UD_GET_BUFFER(2);
//    lua_pushinteger(L, uart_fifo_fill(dev->device, buf->ptr, buf->size));
//    return 1;
//}

static int lua_uart_tx(lua_State * L) {
    UD_GET_DEVICE(1);
    UD_GET_BUFFER(2);
    int32_t timeout = luaL_checkinteger(L, 3);
    lua_pushinteger(L, uart_tx(dev->device, buf->ptr, buf->size, timeout));
    return 1;
}

//static int lua_uart_tx_u16(lua_State * L) {
    //UD_GET_DEVICE(1);
    //UD_GET_BUFFER(2);
    //int32_t timeout = luaL_checkinteger(L, 3);
    //lua_pushinteger(L, uart_tx_u16(dev->device, buf->ptr, buf->size/2, timeout));
    //return 1;
//}

static int lua_uart_tx_abort(lua_State * L) {
    UD_GET_DEVICE(1);
    lua_pushinteger(L, uart_tx_abort(dev->device));
    return 1;
}

static int lua_uart_rx_enable(lua_State * L) {
    UD_GET_DEVICE(1);
    UD_GET_BUFFER(2);
    int32_t timeout = luaL_checkinteger(L, 3);
    lua_pushinteger(L, uart_rx_enable(dev->device, buf->ptr, buf->size, timeout));
    return 1;
}

//static int lua_uart_rx_enable_u16(lua_State * L) {
    //UD_GET_DEVICE(1);
    //UD_GET_BUFFER(2);
    //int32_t timeout = luaL_checkinteger(L, 3);
    //lua_pushinteger(L, uart_rx_enable_u16(dev->device, buf->ptr, buf->size/2, timeout));
    //return 1;
//}

static int lua_uart_rx_disable(lua_State * L) {
    UD_GET_DEVICE(1);
    lua_pushinteger(L, uart_rx_disable(dev->device));
    return 1;
}

//////////////////////////
/// library definition ///
//////////////////////////

static const luaL_Reg zephyr_funcs[] = {
  // buffer
  {"alloc_buffer", lua_alloc_buffer},
  {"buffer_size", lua_buffer_size},
  {"get_buffer_u8", lua_get_buffer_u8},
  {"get_buffer_s8", lua_get_buffer_s8},
  {"get_buffer_u16", lua_get_buffer_u16},
  {"get_buffer_s16", lua_get_buffer_s16},
  {"get_buffer_u32", lua_get_buffer_u32},
  {"get_buffer_s32", lua_get_buffer_s32},
  {"set_buffer_u8", lua_set_buffer_u8},
  {"set_buffer_s8", lua_set_buffer_s8},
  {"set_buffer_u16", lua_set_buffer_u16},
  {"set_buffer_s16", lua_set_buffer_s16},
  {"set_buffer_u32", lua_set_buffer_u32},
  {"set_buffer_s32", lua_set_buffer_s32},
  // device model
  {"device_get_binding", lua_device_get_binding},
  {"device_is_ready", lua_device_is_ready},
  // EEPROM
  /* {"eeprom_read", lua_eeprom_read}, */
  /* {"eeprom_write", lua_eeprom_write}, */
  /* {"eeprom_get_size", lua_eeprom_get_size}, */
  // FLASH
  {"flash_read", flash_read},
  {"flash_write", flash_write},
  {"flash_erase", flash_erase},
  /* {"flash_get_page_count", flash_get_page_count}, */
  // i2c
  /* {"i2c_get_config", lua_i2c_get_config}, */
  {"i2c_configure", lua_i2c_configure},
  // gpio
  {"gpio_pin_interrupt_configure", lua_gpio_pin_interrupt_configure},
  {"gpio_pin_configure", lua_gpio_pin_configure},
  {"gpio_port_get", lua_gpio_port_get},
  {"gpio_port_get_raw", lua_gpio_port_get_raw},
  {"gpio_port_set_masked", lua_gpio_port_set_masked},
  {"gpio_port_set_masked_raw", lua_gpio_port_set_masked_raw},
  {"gpio_port_set_bits", lua_gpio_port_set_bits},
  {"gpio_port_set_bits_raw", lua_gpio_port_set_bits_raw},
  {"gpio_port_clear_bits", lua_gpio_port_clear_bits},
  {"gpio_port_clear_bits_raw", lua_gpio_port_clear_bits_raw},
  {"gpio_port_toggle_bits", lua_gpio_port_toggle_bits},
  {"gpio_port_set_clr_bits_raw", lua_gpio_port_set_clr_bits_raw},
  {"gpio_port_set_clr_bits", lua_gpio_port_set_clr_bits},
  {"gpio_pin_get", lua_gpio_pin_get},
  {"gpio_pin_get_raw", lua_gpio_pin_get_raw},
  {"gpio_pin_set", lua_gpio_pin_set},
  {"gpio_pin_set_raw", lua_gpio_pin_set_raw},
  {"gpio_pin_toggle", lua_gpio_pin_toggle},
  // hwinfo
  /* {"hwinfo_get_reset_cause", lua_hwinfo_get_reset_cause}, */
  /* {"hwinfo_get_supported_reset_cause", lua_hwinfo_get_supported_reset_cause}, */
  /* {"hwinfo_clear_reset_cause", lua_hwinfo_clear_reset_cause}, */
  // pinmux
  /* {"pinmux_pin_set", lua_pinmux_pin_set}, */
  /* {"pinmux_pin_get", lua_pinmux_pin_set}, */
  /* {"pinmux_pin_pullup", lua_pinmux_pin_pullup}, */
  /* {"pinmux_pin_input_enable", lua_pinmux_pin_input_enable}, */
  // uart
  {"uart_err_check", lua_uart_err_check},
  {"uart_configure", lua_uart_configure},
  {"uart_config_get", lua_uart_config_get},
  {"uart_line_ctrl_set", lua_uart_line_ctrl_set},
  {"uart_line_ctrl_get", lua_uart_line_ctrl_get},
  {"uart_poll_in", lua_uart_poll_in},
  /* {"uart_poll_in_u16", lua_uart_poll_in_u16}, */
  {"uart_poll_out", lua_uart_poll_out},
  /* {"uart_poll_out_u16", lua_uart_poll_out_u16}, */
  {"uart_tx", lua_uart_tx},
  /* {"uart_tx_u16", lua_uart_tx_u16}, */
  {"uart_tx_abort", lua_uart_tx_abort},
  {"uart_rx_enable", lua_uart_rx_enable},
  /* {"uart_rx_enable_u16", lua_uart_rx_enable_u16}, */
  {"uart_rx_disable", lua_uart_rx_disable},

  {NULL, NULL},
};


LUAMOD_API int luaopen_zephyr (lua_State *L) {
  luaL_newlib(L, zephyr_funcs);
  return 1;
}
