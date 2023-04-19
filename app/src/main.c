#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/fs/fs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "lauxlib.h"
#include "lualib.h"

#include "isbd.h"

#include "luac.h"

/* change this to any other UART peripheral if desired */
// #define UART_MASTER_DEVICE_NODE DT_NODELABEL(uart0)
#define UART_DTE_DEVICE_NODE DT_NODELABEL( uart3 )

static struct device *dte_device =
  (struct device*)DEVICE_DT_GET( UART_DTE_DEVICE_NODE );

// For reference
// https://www.lua.org/pil/24.1.html
// https://docs.zephyrproject.org/latest/contribute/bin_blobs.html

static isu_dte_t g_isu_dte;

/*
static FATFS fat_fs;
static struct fs_mount_t mp = {
    .type = FS_FATFS,
    .fs_data = &fat_fs,
    .mnt_point = "/SD:",
};
*/

// typedef int (*lua_CFunction) (lua_State *L);

static int l_isbd_setup( lua_State *L );
static int l_isbd_wait_event( lua_State *L );

static const struct luaL_Reg l_iridium[] = {
  {"isbd_setup", l_isbd_setup },
  {"isbd_wait_event", l_isbd_wait_event },
  {NULL, NULL}  /* sentinel */
};

static uint8_t rx_buf[ 512 ];
static uint8_t tx_buf[ 512 ];

static int l_isbd_setup( lua_State *L ) {

  // double d = luaL_checknumber(L, 1);
  
  struct uart_config uart_config;

	uart_config_get( dte_device, &uart_config );
	uart_config.baudrate = 19200;
	uart_configure( dte_device, &uart_config );

  isu_dte_config_t isu_dte_config = {
    .at_uart = {
      .echo = false,
      .verbose = true,
      // .zuart = ZUART_CONF_POLL( uart_slave_device ),
      .zuart = ZUART_CONF_IRQ( dte_device, rx_buf, sizeof( rx_buf ), tx_buf, sizeof( tx_buf ) ),
      // .zuart = ZUART_CONF_MIX_RX_IRQ_TX_POLL( uart_slave_device, rx_buf, sizeof( rx_buf ) ),
      // .zuart = ZUART_CONF_MIX_RX_POLL_TX_IRQ( uart_slave_device, tx_buf, sizeof( tx_buf ) ),
    }
  };

  lua_pushnumber( L, isu_dte_setup( &g_isu_dte, &isu_dte_config ) );

  // TODO: allow configuration from Lua
  isbd_config_t isbd_config = {
    .dte            = &g_isu_dte,
    .priority       = 0,
    .mo_queue_len   = 4,
    .evt_queue_len  = 8,
  };

  isbd_setup( &isbd_config );

  return 1;
}

static int l_isbd_wait_event( lua_State *L ) {

  isbd_evt_t evt;
  lua_Number timeout_ms = luaL_checknumber( L, 1 );

  if ( isbd_wait_evt( &evt, (uint32_t)timeout_ms ) ) {
    
    lua_pushnumber( L, (lua_Number)evt.id );
    
    if ( evt.id == ISBD_EVT_MT ) {
      lua_createtable( L, 0, 1 );
      lua_pushstring( L, "data" );
      lua_pushlstring( L, (const char*)evt.mt.data, evt.mt.len );
      lua_settable( L, -3 );
    } else {
      lua_newtable( L );
    }

    isbd_destroy_evt( &evt );

    return 2;

  }

  return 0;

}

int luaopen_iridium( lua_State *L ) {

  lua_newtable( L );
  luaL_setfuncs( L, l_iridium, 0 );

  // lua_pushvalue( L, -1 ); // pluck these lines out if they offend you

  lua_setglobal( L, "iridium" );

  // luaL_openlib( L, "iridium", l_iridium, 0 );
  return 1;
}

void main(void) {

  static lua_State * L;

  L = luaL_newstate();
  luaL_openlibs( L );
  luaopen_iridium( L );
  
  // lua_pushcfunction( L, l_sin );
  // lua_setglobal( L, "mysin" );

  int error = 
    luaL_loadbuffer( L, g_lua_main, sizeof( g_lua_main ), "main" ) 
    || lua_pcall( L, 0, 0, 0 );

  if ( error ) {
    fprintf( stderr, "%s", lua_tostring( L, -1 ) );
    lua_pop( L, 1 );  /* pop error message from the stack */
  }

  lua_close( L );

}

