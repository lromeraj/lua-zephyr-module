#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <drivers/eeprom.h>
#include <drivers/flash.h>
#include <drivers/uart.h>
#include <drivers/pinmux.h>

#include <zsl/matrices.h>
#include <zsl/interp.h>

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "zsl/interp.h"
#include "zsl/vectors.h"
#include "zsl/zsl.h"

#include <string.h>

//////////////
/// BUFFER ///
//////////////

#define T_USERDATA_FAT 1
#define UD_GET_BUFFER(ix) ud_fat_t * buf = lua_touserdata(L, ix); \
    luaL_argcheck(L, buf->type == T_USERDATA_FAT, ix, "`buffer' expected"); \
    luaL_argcheck(L, buf->ptr != NULL, ix, "`buffer' does not exist");
typedef struct {
  int type;
  struct {
    size_t size;
    void * ptr;
  };
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

////////////////////////////
/// ZSCILIB Bindings MTX ///
////////////////////////////

#define T_USERDATA_MTX 3
#define UD_GET_MTX_NAMED(ix, named) ud_mtx_t * named = lua_touserdata(L, ix); \
    luaL_argcheck(L, named->type == T_USERDATA_MTX, ix, "`matrix' expected");
#define UD_GET_MTX(ix) UD_GET_MTX_NAMED(ix, mtx)
typedef struct {
    int type;
    struct zsl_mtx mtx;
} ud_mtx_t;

static int lua_zsl_mtx_new(lua_State * L) {
  int rows = luaL_checkinteger(L, 1);
  int cols = luaL_checkinteger(L, 2);
  zsl_real_t defVal = luaL_optnumber(L, 3, 0.0f);
  ud_mtx_t* ptr = lua_newuserdata(L,
                                  sizeof(int) + sizeof(struct zsl_mtx)
                                  + sizeof(zsl_real_t) * rows * cols);
  ptr->type = T_USERDATA_MTX;
  struct zsl_mtx* mtx = &(ptr->mtx);
  mtx->sz_rows = rows;
  mtx->sz_cols = cols;
  mtx->data = (zsl_real_t *)(mtx+1);
  for(int i = 0; i < rows; i++) {
    for(int j = 0; j < cols; j++) {
      zsl_mtx_set(mtx, i, j, defVal);
    }
  }
  return 1;
}

static int lua_zsl_mtx_from_buffer(lua_State * L) {
  UD_GET_MTX(1);
  UD_GET_BUFFER(2);
  zsl_mtx_from_arr(&mtx->mtx, buf->ptr);
  lua_pushlightuserdata(L, mtx);
  return 1;
}

static int lua_zsl_mtx_copy(lua_State * L) {
  UD_GET_MTX(1);
  UD_GET_MTX_NAMED(1, src);
  zsl_mtx_copy(&mtx->mtx, &src->mtx);
  lua_pushlightuserdata(L, mtx);
  return 1;
}

static int lua_zsl_mtx_get(lua_State * L) {
  UD_GET_MTX(1);
  int i = luaL_checkinteger(L, 2);
  int j = luaL_checkinteger(L, 3);
  zsl_real_t x;
  zsl_mtx_get(&mtx->mtx, i, j, &x);
  lua_pushnumber(L, x);
  return 1;
}

static int lua_zsl_mtx_set(lua_State * L) {
  UD_GET_MTX(1);
  int i = luaL_checkinteger(L, 2);
  int j = luaL_checkinteger(L, 3);
  zsl_real_t x = luaL_checknumber(L, 4);
  zsl_mtx_set(&mtx->mtx, i, j, x);
  return 0;
}

static int lua_zsl_mtx_get_row(lua_State * L) {
  UD_GET_MTX(1);
  int i = luaL_checkinteger(L, 2);
  UD_GET_BUFFER(3);
  zsl_mtx_get_row(&mtx->mtx, i, buf->ptr);
  lua_pushlightuserdata(L, buf);
  return 1;
}

static int lua_zsl_mtx_set_row(lua_State * L) {
  UD_GET_MTX(1);
  int i = luaL_checkinteger(L, 2);
  UD_GET_BUFFER(3);
  zsl_mtx_set_row(&mtx->mtx, i, buf->ptr);
  return 0;
}

static int lua_zsl_mtx_get_col(lua_State * L) {
  UD_GET_MTX(1);
  int j = luaL_checkinteger(L, 2);
  UD_GET_BUFFER(3);
  zsl_mtx_get_col(&mtx->mtx, j, buf->ptr);
  lua_pushlightuserdata(L, buf);
  return 1;
}

static int lua_zsl_mtx_set_col(lua_State * L) {
  UD_GET_MTX(1);
  int j = luaL_checkinteger(L, 2);
  UD_GET_BUFFER(3);
  zsl_mtx_set_col(&mtx->mtx, j, buf->ptr);
  return 0;
}

static int lua_zsl_mtx_unary_op(lua_State * L) {
  UD_GET_MTX(1);
  int op = luaL_checkinteger(L, 2);
  zsl_mtx_unary_op(&mtx->mtx, op);
  return 0;
}

// TODO Mapping functions unary and binary

static int lua_zsl_mtx_binary_op(lua_State * L) {
  UD_GET_MTX_NAMED(1, ma);
  UD_GET_MTX_NAMED(2, mb);
  UD_GET_MTX_NAMED(3, mc);
  int op = luaL_checkinteger(L, 4);
  zsl_mtx_binary_op(&ma->mtx, &mb->mtx, &mc->mtx, op);
  lua_pushlightuserdata(L, mc);
  return 1;
}

static int lua_zsl_mtx_add(lua_State * L) {
  UD_GET_MTX_NAMED(1, ma);
  UD_GET_MTX_NAMED(2, mb);
  UD_GET_MTX_NAMED(3, mc);
  zsl_mtx_add(&ma->mtx, &mb->mtx, &mc->mtx);
  lua_pushlightuserdata(L, mc);
  return 1;
}

static int lua_zsl_mtx_add_d(lua_State * L) {
  UD_GET_MTX_NAMED(1, ma);
  UD_GET_MTX_NAMED(2, mb);
  zsl_mtx_add_d(&ma->mtx, &mb->mtx);
  lua_pushlightuserdata(L, ma);
  return 1;
}

static int lua_zsl_mtx_sum_rows_d(lua_State * L) {
  UD_GET_MTX(1);
  int i = luaL_checkinteger(L, 2);
  int j = luaL_checkinteger(L, 3);
  zsl_mtx_sum_rows_d(&mtx->mtx, i, j);
  return 0;
}

static int lua_zsl_mtx_sum_rows_scaled_d(lua_State * L) {
  UD_GET_MTX(1);
  int i = luaL_checkinteger(L, 2);
  int j = luaL_checkinteger(L, 3);
  zsl_real_t scale = luaL_checknumber(L, 4);
  zsl_mtx_sum_rows_scaled_d(&mtx->mtx, i, j, scale);
  return 0;
}

static int lua_zsl_mtx_sub(lua_State * L) {
  UD_GET_MTX_NAMED(1, ma);
  UD_GET_MTX_NAMED(2, mb);
  UD_GET_MTX_NAMED(3, mc);
  zsl_mtx_sub(&ma->mtx, &mb->mtx, &mc->mtx);
  lua_pushlightuserdata(L, mc);
  return 1;
}

static int lua_zsl_mtx_sub_d(lua_State * L) {
  UD_GET_MTX_NAMED(1, ma);
  UD_GET_MTX_NAMED(2, mb);
  zsl_mtx_sub_d(&ma->mtx, &mb->mtx);
  lua_pushlightuserdata(L, ma);
  return 1;
}

static int lua_zsl_mtx_mult(lua_State * L) {
  UD_GET_MTX_NAMED(1, ma);
  UD_GET_MTX_NAMED(2, mb);
  UD_GET_MTX_NAMED(3, mc);
  zsl_mtx_mult(&ma->mtx, &mb->mtx, &mc->mtx);
  lua_pushlightuserdata(L, mc);
  return 1;
}

static int lua_zsl_mtx_mult_d(lua_State * L) {
  UD_GET_MTX_NAMED(1, ma);
  UD_GET_MTX_NAMED(2, mb);
  zsl_mtx_mult_d(&ma->mtx, &mb->mtx);
  lua_pushlightuserdata(L, ma);
  return 1;
}

static int lua_zsl_mtx_scalar_mult_d(lua_State * L) {
  UD_GET_MTX(1);
  zsl_real_t s = luaL_checknumber(L, 2);
  zsl_mtx_scalar_mult_d(&mtx->mtx, s);
  return 0;
}

static int lua_zsl_mtx_scalar_mult_row_d(lua_State * L) {
  UD_GET_MTX(1);
  int i = luaL_checkinteger(L, 2);
  zsl_real_t s = luaL_checknumber(L, 3);
  zsl_mtx_scalar_mult_row_d(&mtx->mtx, i, s);
  return 0;
}

static int lua_zsl_mtx_trans(lua_State * L) {
  UD_GET_MTX_NAMED(1, ma);
  UD_GET_MTX_NAMED(2, mb);
  zsl_mtx_trans(&ma->mtx, &mb->mtx);
  return 0;
}

static int lua_zsl_mtx_adjoin(lua_State * L) {
  UD_GET_MTX_NAMED(1, m);
  UD_GET_MTX_NAMED(2, ma);
  if(m->mtx.sz_cols == 3 && m->mtx.sz_rows == 3) {
    zsl_mtx_adjoint_3x3(&m->mtx, &ma->mtx);
  } else {
    zsl_mtx_adjoint(&m->mtx, &ma->mtx);
  }
  return 0;
}

static int lua_zsl_mtx_reduce(lua_State * L) {
  UD_GET_MTX_NAMED(1, m);
  UD_GET_MTX_NAMED(2, mr);
  int i = luaL_checkinteger(L, 3);
  int j = luaL_checkinteger(L, 4);
  zsl_mtx_reduce(&m->mtx, &mr->mtx, i, j);
  lua_pushlightuserdata(L, mr);
  return 1;
}

static int lua_zsl_mtx_deter(lua_State * L) {
  UD_GET_MTX(1);
  zsl_real_t r;
  if(mtx->mtx.sz_cols == 3 && mtx->mtx.sz_rows == 3) {
    zsl_mtx_deter_3x3(&mtx->mtx, &r);
  } else {
    zsl_mtx_deter(&mtx->mtx, &r);
  }
  lua_pushnumber(L, r);
  return 1;
}

static int lua_zsl_mtx_cols_norm(lua_State * L) {
  UD_GET_MTX_NAMED(1, m);
  UD_GET_MTX_NAMED(2, mnorm);
  zsl_mtx_cols_norm(&m->mtx, &mnorm->mtx);
  return 0;
}

static int lua_zsl_mtx_norm_elem(lua_State * L) {
  UD_GET_MTX_NAMED(1, m);
  UD_GET_MTX_NAMED(2, mnorm);
  UD_GET_MTX_NAMED(3, mi);
  int i = luaL_checkinteger(L, 4);
  int j = luaL_checkinteger(L, 5);
  zsl_mtx_norm_elem(&m->mtx, &mnorm->mtx, &mi->mtx, i, j);
  return 0;
}

static int lua_zsl_mtx_inv(lua_State * L) {
  UD_GET_MTX(1);
  UD_GET_MTX_NAMED(2, mi);
  if(mtx->mtx.sz_cols == 3 && mtx->mtx.sz_rows == 3) {
    zsl_mtx_inv_3x3(&mtx->mtx, &mi->mtx);
  } else {
    zsl_mtx_inv(&mtx->mtx, &mi->mtx);
  }
  lua_pushlightuserdata(L, mi);
  return 1;
}

static int lua_zsl_mtx_qrd(lua_State * L) {
  UD_GET_MTX_NAMED(1, m);
  UD_GET_MTX_NAMED(2, q);
  UD_GET_MTX_NAMED(3, r);
  int hessenberg = luaL_optinteger(L, 4, 0);
  zsl_mtx_qrd(&m->mtx, &q->mtx, &r->mtx, hessenberg);
  return 0;
}

static int lua_zsl_mtx_min(lua_State * L) {
  UD_GET_MTX(1);
  int i;
  int j;
  zsl_real_t x;
  zsl_mtx_min_idx(&mtx->mtx, &i, &j);
  zsl_mtx_get(&mtx->mtx, i, j, &x);
  lua_pushnumber(L, x);
  lua_pushinteger(L, i);
  lua_pushinteger(L, j);
  return 3;
}

static int lua_zsl_mtx_max(lua_State * L) {
  UD_GET_MTX(1);
  int i;
  int j;
  zsl_real_t x;
  zsl_mtx_max_idx(&mtx->mtx, &i, &j);
  zsl_mtx_get(&mtx->mtx, i, j, &x);
  lua_pushnumber(L, x);
  lua_pushinteger(L, i);
  lua_pushinteger(L, j);
  return 3;
}

static int lua_zsl_mtx_is_equal(lua_State * L) {
  UD_GET_MTX_NAMED(1, ma);
  UD_GET_MTX_NAMED(2, mb);
  lua_pushboolean(L, zsl_mtx_is_equal(&ma->mtx, &mb->mtx));
  return 1;
}

static int lua_zsl_mtx_is_notneg(lua_State * L) {
  UD_GET_MTX_NAMED(1, m);
  lua_pushboolean(L, zsl_mtx_is_notneg(&m->mtx));
  return 1;
}

static int lua_zsl_mtx_is_sym(lua_State * L) {
  UD_GET_MTX_NAMED(1, m);
  lua_pushboolean(L, zsl_mtx_is_sym(&m->mtx));
  return 1;
}

static int lua_zsl_mtx_print(lua_State * L) {
  UD_GET_MTX_NAMED(1, m);
  zsl_mtx_is_sym(&m->mtx);
  return 0;
}

///////////////////////////////
/// ZSCILIB Bindings interp ///
///////////////////////////////

static int lua_zsl_interp_lerp(lua_State * L) {
  zsl_real_t v0 = luaL_checknumber(L, 1);
  zsl_real_t v1 = luaL_checknumber(L, 2);
  zsl_real_t t = luaL_checknumber(L, 3);
  zsl_real_t v;
  zsl_interp_lerp(v0, v1, t, &v);
  lua_pushnumber(L, v);
  return 1;
}

static int lua_zsl_interp_find_x(lua_State * L) {
  UD_GET_BUFFER(1);
  zsl_real_t x = luaL_checknumber(L, 2);
  int idx;
  size_t n = buf->size / sizeof(struct zsl_interp_xy);
  zsl_interp_find_x(buf->ptr, n, x, &idx);
  lua_pushinteger(L, idx);
  return 1;
}

static int lua_zsl_interp_arr(lua_State * L) {
  UD_GET_BUFFER(1);
  zsl_real_t x = luaL_checknumber(L, 2);
  int degree = luaL_checkinteger(L, 3);
  size_t n = buf->size / sizeof(struct zsl_interp_xy);
  zsl_real_t y;
  switch (degree) {
    case 0: zsl_interp_nn_arr(buf->ptr, n, x, &y); break;
    case 1: zsl_interp_lin_y_arr(buf->ptr, n, x, &y); break;
    default: break;
  }
  lua_pushnumber(L, y);
  return 1;
}

////////////////////////////
/// ZSCILIB Bindings VEC ///
////////////////////////////

#define T_USERDATA_VEC 4
#define UD_GET_VEC_NAMED(ix, named) ud_vec_t * named = lua_touserdata(L, ix); \
    luaL_argcheck(L, named->type == T_USERDATA_VEC, ix, "`matrix' expected");
#define UD_GET_VEC(ix) UD_GET_VEC_NAMED(ix, vec)
typedef struct {
  int type;
  struct zsl_vec vec;
} ud_vec_t;

static int lua_zsl_vec_new(lua_State * L) {
  int size = luaL_checkinteger(L, 1);
  ud_vec_t* ptr = lua_newuserdata(L, sizeof(ud_vec_t) + size * sizeof(zsl_real_t));
  ptr->type = T_USERDATA_VEC;
  struct zsl_vec* vec = &(ptr->vec);
  vec->sz = size;
  vec->data = (zsl_real_t *)(vec+1);
  zsl_vec_init(vec);
  return 1;
}

static int lua_zsl_vec_from_arr(lua_State * L) {
  UD_GET_BUFFER(1);
  buf->type = T_USERDATA_VEC;
  buf->size = buf->size / sizeof(zsl_real_t);
  lua_pushlightuserdata(L, buf);
  return 1;
}

static int lua_zsl_arr_from_vec(lua_State * L) {
  UD_GET_VEC(1);
  vec->type = T_USERDATA_FAT;
  vec->vec.sz = vec->vec.sz * sizeof(zsl_real_t);
  lua_pushlightuserdata(L, vec);
  return 1;
}

static int lua_zsl_vec_copy(lua_State * L) {
  UD_GET_VEC_NAMED(1, vdest);
  UD_GET_VEC_NAMED(2, vsrc);
  zsl_vec_copy(&vdest->vec, &vsrc->vec);
  lua_pushlightuserdata(L, vdest);
  return 1;
}

static int lua_zsl_vec_get_subset(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  int offset = luaL_checkinteger(L, 2);
  int len = luaL_checkinteger(L, 3);
  UD_GET_VEC_NAMED(4, vsub);
  zsl_vec_get_subset(&v->vec, offset, len, &vsub->vec);
  lua_pushlightuserdata(L, vsub);
  return 1;
}

static int lua_zsl_vec_add(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  UD_GET_VEC_NAMED(2, w);
  UD_GET_VEC_NAMED(3, x);
  zsl_vec_add(&v->vec, &w->vec, &x->vec);
  lua_pushlightuserdata(L, x);
  return 1;
}

static int lua_zsl_vec_sub(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  UD_GET_VEC_NAMED(2, w);
  UD_GET_VEC_NAMED(3, x);
  zsl_vec_sub(&v->vec, &w->vec, &x->vec);
  lua_pushlightuserdata(L, x);
  return 1;
}

static int lua_zsl_vec_neg(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  zsl_vec_neg(&v->vec);
  lua_pushlightuserdata(L, v);
  return 1;
}

// TODO zsl_vec_sum

static int lua_zsl_vec_scalar_add(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  zsl_real_t s = luaL_checknumber(L, 2);
  zsl_vec_scalar_add(&v->vec, s);
  lua_pushlightuserdata(L, v);
  return 1;
}

static int lua_zsl_vec_scalar_mult(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  zsl_real_t s = luaL_checknumber(L, 2);
  zsl_vec_scalar_mult(&v->vec, s);
  lua_pushlightuserdata(L, v);
  return 1;
}

static int lua_zsl_vec_scalar_div(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  zsl_real_t s = luaL_checknumber(L, 2);
  zsl_vec_scalar_div(&v->vec, s);
  lua_pushlightuserdata(L, v);
  return 1;
}

static int lua_zsl_vec_dist(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  UD_GET_VEC_NAMED(2, w);
  lua_pushnumber(L, zsl_vec_dist(&v->vec, &w->vec));
  return 1;
}

static int lua_zsl_vec_dot(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  UD_GET_VEC_NAMED(2, w);
  zsl_real_t d;
  zsl_vec_dot(&v->vec, &w->vec, &d);
  lua_pushnumber(L, d);
  return 1;
}

static int lua_zsl_vec_norm(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  lua_pushnumber(L, zsl_vec_norm(&v->vec));
  return 1;
}

static int lua_zsl_vec_project(lua_State * L) {
  UD_GET_VEC_NAMED(1, u);
  UD_GET_VEC_NAMED(2, v);
  UD_GET_VEC_NAMED(3, w);
  zsl_vec_project(&u->vec, &v->vec, &w->vec);
  lua_pushlightuserdata(L, w);
  return 1;
}

static int lua_zsl_vec_to_unit(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  zsl_vec_to_unit(&v->vec);
  lua_pushlightuserdata(L, v);
  return 1;
}

static int lua_zsl_vec_cross(lua_State * L) {
  UD_GET_VEC_NAMED(1, u);
  UD_GET_VEC_NAMED(2, v);
  UD_GET_VEC_NAMED(3, w);
  zsl_vec_cross(&u->vec, &v->vec, &w->vec);
  lua_pushlightuserdata(L, w);
  return 1;
}

static int lua_zsl_vec_sum_of_sqrs(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  lua_pushnumber(L, zsl_vec_sum_of_sqrs(&v->vec));
  return 1;
}

// TODO MEAN

static int lua_zsl_vec_ar_mean(lua_State * L) {
  UD_GET_VEC(1);
  zsl_real_t m;
  zsl_vec_ar_mean(&vec->vec, &m);
  lua_pushnumber(L, m);
  return 1;
}

static int lua_zsl_vec_rev(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  lua_pushnumber(L, zsl_vec_rev(&v->vec));
  return 1;
}

static int lua_zsl_vec_zte(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  lua_pushnumber(L, zsl_vec_zte(&v->vec));
  return 1;
}

static int lua_zsl_vec_is_equal(lua_State * L) {
  UD_GET_VEC_NAMED(1, u);
  UD_GET_VEC_NAMED(2, v);
  zsl_real_t eps = luaL_checknumber(L, 3);
  lua_pushboolean(L, zsl_vec_is_equal(&u->vec, &v->vec, eps));
  return 1;
}

static int lua_zsl_vec_is_nonneg(lua_State * L) {
  UD_GET_VEC_NAMED(1, u);
  lua_pushboolean(L, zsl_vec_is_nonneg(&u->vec));
  return 1;
}

static int lua_zsl_vec_contains(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  zsl_real_t val = luaL_checknumber(L, 2);
  zsl_real_t eps = luaL_checknumber(L, 3);
  lua_pushinteger(L, zsl_vec_contains(&v->vec, val, eps));
  return 1;
}

static int lua_zsl_vec_sort(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  UD_GET_VEC_NAMED(2, w);
  zsl_vec_sort(&v->vec, &w->vec);
  lua_pushlightuserdata(L, w);
  return 1;
}

static int lua_zsl_vec_print(lua_State * L) {
  UD_GET_VEC_NAMED(1, v);
  zsl_vec_print(&v->vec);
  return 0;
}

//////////////////////////
/// Library definition ///
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
  {"flash_read", lua_flash_read},
  {"flash_write", lua_flash_write},
  {"flash_erase", lua_flash_erase},
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
  // ZSL
  {"mtx_new", lua_zsl_mtx_new},
  {"mtx_from_buffer", lua_zsl_mtx_from_buffer},
  {"mtx_copy", lua_zsl_mtx_copy},
  {"mtx_get", lua_zsl_mtx_get},
  {"mtx_set", lua_zsl_mtx_set},
  {"mtx_get_row", lua_zsl_mtx_get_row},
  {"mtx_set_col", lua_zsl_mtx_set_col},
  {"mtx_get_col", lua_zsl_mtx_get_col},
  {"mtx_set_row", lua_zsl_mtx_set_row},
  {"mtx_unary_op", lua_zsl_mtx_unary_op},
  {"mtx_binary_op", lua_zsl_mtx_binary_op},
  {"mtx_add", lua_zsl_mtx_add},
  {"mtx_add_d", lua_zsl_mtx_add_d},
  {"mtx_sum_rows_d", lua_zsl_mtx_sum_rows_d},
  {"mtx_sum_rows_scaled_d", lua_zsl_mtx_sum_rows_scaled_d},
  {"mtx_sub", lua_zsl_mtx_sub},
  {"mtx_sub_d", lua_zsl_mtx_sub_d},
  {"mtx_mult", lua_zsl_mtx_mult},
  {"mtx_mult_d", lua_zsl_mtx_mult_d},
  {"mtx_scalar_mult_d", lua_zsl_mtx_scalar_mult_d},
  {"mtx_scalar_mult_row_d", lua_zsl_mtx_scalar_mult_row_d},
  {"mtx_trans", lua_zsl_mtx_trans},
  {"mtx_adjoin", lua_zsl_mtx_adjoin},
  {"mtx_deter", lua_zsl_mtx_deter},
  {"mtx_reduce", lua_zsl_mtx_reduce},
  {"mtx_cols_norm", lua_zsl_mtx_cols_norm},
  {"mtx_norm_elem", lua_zsl_mtx_norm_elem},
  {"mtx_inv", lua_zsl_mtx_inv},
  {"mtx_qrd", lua_zsl_mtx_qrd},
  {"mtx_min", lua_zsl_mtx_min},
  {"mtx_max", lua_zsl_mtx_max},
  {"mtx_is_equal", lua_zsl_mtx_is_equal},
  {"mtx_is_notneg", lua_zsl_mtx_is_notneg},
  {"mtx_is_sym", lua_zsl_mtx_is_sym},
  {"mtx_print", lua_zsl_mtx_print},
  {"interp_lerp", lua_zsl_interp_lerp},
  {"interp_find_x", lua_zsl_interp_find_x},
  {"interp_arr", lua_zsl_interp_arr},
  {"vec_new", lua_zsl_vec_new},
  {"vec_from_arr", lua_zsl_vec_from_arr},
  {"arr_from_vec", lua_zsl_arr_from_vec},
  {"vec_copy", lua_zsl_vec_copy},
  {"vec_get_subset", lua_zsl_vec_get_subset},
  {"vec_add", lua_zsl_vec_add},
  {"vec_sub", lua_zsl_vec_sub},
  {"vec_neg", lua_zsl_vec_neg},
  {"vec_scalar_add", lua_zsl_vec_scalar_add},
  {"vec_scalar_mult", lua_zsl_vec_scalar_mult},
  {"vec_scalar_div", lua_zsl_vec_scalar_div},
  {"vec_dist", lua_zsl_vec_dist},
  {"vec_dot", lua_zsl_vec_dot},
  {"vec_norm", lua_zsl_vec_norm},
  {"vec_project", lua_zsl_vec_project},
  {"vec_to_unit", lua_zsl_vec_to_unit},
  {"vec_cross", lua_zsl_vec_cross},
  {"vec_sum_of_sqrs", lua_zsl_vec_sum_of_sqrs},
  {"vec_ar_mean", lua_zsl_vec_ar_mean},
  {"vec_rev", lua_zsl_vec_rev},
  {"vec_zte", lua_zsl_vec_zte},
  {"vec_is_equal", lua_zsl_vec_is_equal},
  {"vec_is_nonneg", lua_zsl_vec_is_nonneg},
  {"vec_contains", lua_zsl_vec_contains},
  {"vec_sort", lua_zsl_vec_sort},
  {"vec_print", lua_zsl_vec_print},
  // TODO statistics
  // TODO colorimetry
  // TODO probability
  {NULL, NULL},
};


LUAMOD_API int luaopen_zephyr (lua_State *L) {
  luaL_newlib(L, zephyr_funcs);
  return 1;
}
