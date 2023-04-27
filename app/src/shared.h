#define UART_960X_NODE       DT_ALIAS( 960x )
#define UART_960X_NODE_DEF   DT_NODELABEL( uart1 )

#if DT_NODE_HAS_STATUS( UART_960X_NODE, okay )

	#define UART_960X_DEVICE \
		(struct device*)DEVICE_DT_GET( UART_960X_NODE );

#elif DT_NODE_HAS_STATUS( UART_960X_NODE_DEF, okay )

	#define UART_960X_DEVICE \
    (struct device*)DEVICE_DT_GET( UART_960X_NODE_DEF );

#else
  #error  "Additional UART is mandatory to communicate with the modem," \
          "specify 960x alias in your board specific overlay"
#endif
