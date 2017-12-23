char serial_rxchar();
void serial_txchar(char ch);
void serial_init(void);

volatile char buffer[5];
volatile char start_flag;
volatile char buf_char;
volatile char stop_flag;
