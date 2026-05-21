#ifndef PIN_OPERATIONS_H
#define PIN_OPERATIONS_H

// DISCRETE VALUES
#define LOW false
#define HIGH true

// DATA DIRECTION
#define SET_INPUT(x)    DDRB &=  ~(1 << x)
#define SET_OUTPUT(x)   DDRB |=   (1 << x)

// WRITE OPERATIONS
#define WRITE_HIGH(x)   PORTB |=  (1 << x)
#define WRITE_LOW(x)    PORTB &= ~(1 << x)
#define WRITE_TOGGLE(x) PORTB ^=  (1 << x)

// READ OPERATION
#define READ_PIN(x)     (PINB & (1 << x))

#endif