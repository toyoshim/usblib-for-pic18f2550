#include <pic18fregs.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

code unsigned char at __CONFIG1L __config1l =
  _USBPLL_CLOCK_SRC_FROM_96MHZ_PLL_2_1L             &
  _CPUDIV__OSC1_OSC2_SRC___1__96MHZ_PLL_SRC___2__1L &
  _PLLDIV_DIVIDE_BY_5__20MHZ_INPUT__1L              ;

code unsigned char at __CONFIG1H __config1h =
  _OSC_HS__HS_PLL__USB_HS_1H &
  _FCMEN_OFF_1H              &
  _IESO_OFF_1H               ;

code unsigned char at __CONFIG2L __config2l =
  _VREGEN_ON_2L   &
  _PUT_ON_2L      &
  _BODEN_OFF_2L   &
  _BODENV_2_0V_2L ;

code unsigned char at __CONFIG2H __config2h =
  _WDT_DISABLED_CONTROLLED_2H &
  _WDTPS_1_32768_2H           ;

code unsigned char at __CONFIG3H __config3h =
  _CCP2MUX_RC1_3H                                          &
  _PBADEN_PORTB_4_0__CONFIGURED_AS_DIGITAL_I_O_ON_RESET_3H &
  _LPT1OSC_ON_3H                                           &
  _MCLRE_MCLR_OFF_RE3_ON_3H                                ;

code unsigned char at __CONFIG4L __config4l =
  _STVR_ON_4L      &
  _LVP_OFF_4L      &
  _ENICPORT_OFF_4L &
  _ENHCPU_OFF_4L   &
  _BACKBUG_OFF_4L  ;

code unsigned char at __CONFIG5L __config5l =
  _CP_0_OFF_5L &
  _CP_1_OFF_5L &
  _CP_2_OFF_5L &
  _CP_3_OFF_5L ;

code unsigned char at __CONFIG5H __config5h =
  _CPD_OFF_5H &
  _CPB_OFF_5H ;

code unsigned char at __CONFIG6L __config6l =
  _WRT_0_OFF_6L &
  _WRT_1_OFF_6L &
  _WRT_2_OFF_6L &
  _WRT_3_OFF_6L ;

code unsigned char at __CONFIG6H __config6h =
  _WRTD_OFF_6H &
  _WRTB_OFF_6H &
  _WRTC_OFF_6H;

code unsigned char at __CONFIG7L __config7l =
  _EBTR_0_OFF_7L &
  _EBTR_1_OFF_7L &
  _EBTR_2_OFF_7L &
  _EBTR_3_OFF_7L ;

code unsigned char at __CONFIG7H __config7h =
  _EBTRB_OFF_7H;

static unsigned char usart_trisc6;
static unsigned char usart_trisc7;

void
psg_init
(void)
{
  int i;
  PORTAbits.RA3 = 0;	// ~IC = 0
  for (i = 0; i < 60; i++) Nop();	// Wait more than 60 Cycle
  PORTAbits.RA3 = 1;	// ~IC = 1 (Reset)
  printf("PSG Initialize\r\n");
}

void
psg_write
(unsigned char addr, unsigned char param)
{
  PORTAbits.RA2 = 0;	// A0 = 0 (Address Write)
  PORTAbits.RA1 = 0;	// ~CS = 0
  PORTAbits.RA0 = 0;	// ~WR = 0
  PORTB = addr;		// D[7:0] = addr
  PORTAbits.RA1 = 1;	// ~CS = 1
  PORTAbits.RA0 = 1;	// ~WR = 1

  PORTAbits.RA2 = 1;	// A0 = 1 (Data Write)
  PORTAbits.RA1 = 0;	// ~CS = 0
  PORTAbits.RA0 = 0;	// ~WR = 0
  PORTB = param;	// D[7:0] = param
  PORTAbits.RA1 = 1;	// ~CS = 1
  PORTAbits.RA0 = 1;	// ~WR = 1

  //printf("PSG Write: 0x%02x <= 0x%02x\r\n", addr, param);
}

void
usart_init
(void)
{
  usart_trisc7 = TRISCbits.TRISC7;
  usart_trisc6 = TRISCbits.TRISC6;
  TRISCbits.TRISC7 = 1;
  TRISCbits.TRISC6 = 0;

  // 57600bps
  TXSTAbits.BRGH = 1;
  SPBRG = 51;

  TXSTAbits.SYNC = 0;
  RCSTAbits.SPEN = 1;
  TXSTAbits.TXEN = 1;

  stdout = STREAM_USART;
}

void
usart_trash
(void)
{
  TXSTAbits.TXEN = 0;
  RCSTAbits.SPEN = 0;
  TRISCbits.TRISC7 = usart_trisc7;
  TRISCbits.TRISC6 = usart_trisc6;
}

typedef struct _usb_bdt {
  unsigned char stat;
  unsigned char cnt;
  unsigned char adrl;
  unsigned char adrh;
} usb_bdt;

typedef struct _descriptor {
  unsigned char  bLength;
  unsigned char  bDescriptor;
  unsigned short bcdUSB;
  unsigned char  bDeviceClass;
  unsigned char  bDeviceSubClass;
  unsigned char  bDeviceProtocol;
  unsigned char  bMaxPacketSize0;
  unsigned short idVendor;
  unsigned short idProduct;
  unsigned short bcdDevice;
  unsigned char  iManufacturer;
  unsigned char  iProduct;
  unsigned char  iSerialNumber;
  unsigned char  bNumConfigurations;
} descriptor;

typedef struct _config {
  unsigned char  bLength;
  unsigned char  bDescriptorType;
  unsigned short wTotalLength;
  unsigned char  bNumInterfaces;
  unsigned char  bConfigurationValue;
  unsigned char  iConfiguration;
  unsigned char  bmAttributes;
  unsigned char  bMaxPower;
} config;

typedef struct _interface {
  unsigned char  bLength;
  unsigned char  bDescriptorType;
  unsigned char  bInterfaceNumber;
  unsigned char  bAlternateSetting;
  unsigned char  bNumEndpoints;
  unsigned char  bInterfaceClass;
  unsigned char  bInterfaceSubClass;
  unsigned char  bInterfaceProtocol;
  unsigned char  iInterface;
} interface;

typedef struct _endpoint {
  unsigned char  bLength;
  unsigned char  bDescriptorType;
  unsigned char  bEndpointAddress;
  unsigned char  bmAttributes;
  unsigned short wMaxPacketSize;
  unsigned char  bInterval;
} endpoint;

typedef struct _allconfig {
  config config;
  interface interface;
  endpoint ep01_out;
} allconfig;

typedef struct _qualifier {
  unsigned char  bLength;
  unsigned char  bDescriptorType;
  unsigned short bcdUSB;
  unsigned char  bDeviceClass;
  unsigned char  bDeviceSubClass;
  unsigned char  bDeviceProtocol;
  unsigned char  bMaxPacketSize;
  unsigned char  bNumConfigulations;
  unsigned char  bReserved;
} qualifier;

descriptor ud_d = {
  sizeof(descriptor),	// Size of Descriptor
  1,			// Device Descriptor Type
  0x200,		// USB2.0
  0,			// Class
  0,			// Sub Class
  0,			// Protocol
  8,			// Buffer Size
  0x6666,		// idVendor
  0x5100,		// idProduct
  0x0080,		// bcdDevice
  1,			// iManufacturer
  2,			// iProduct
  0,			// iSerialNumber
  1,			// bNumConfigulations
};

allconfig ud_c = {
  {
    sizeof(config),	// Size of Descriptor
    2,			// Configuration Descriptor Type
    sizeof(allconfig),	// Total Length of Data
    1,			// Number of Interfaces
    1,			// Index Value
    0,			// Configuration String Index
    0x80,		// Attributes
    50,			// Max Power Consumption (100mA)
  },
  {
    sizeof(interface),	// Size of Descriptor
    4,			// Configuration Descriptor Type
    0,			// Interface Number
    0,			// Alternate Setting Number
    1,			// Number of Endpoints
    0,			// Class
    0,			// Subclass
    0,			// Protocol
    0,			// Interface String Index
  },
  {
    sizeof(endpoint),	// Size of Descriptor
    5,			// Endpint Descriptor Type
    0x01,		// EP01 OUT
    0x02,		// Bulk Transfer
    8,			// Buffer Size
    32,			// Interval
  },
};

typedef struct { unsigned char bLength; unsigned char bDscType; unsigned short string[1]; } s0;
s0 ud_s0 = {
  sizeof(s0),
  3,
  { 0x0409 },
};

typedef struct { unsigned char bLength; unsigned char bDscType; unsigned short string[15]; } s1;
s1 ud_s1 = {
  sizeof(s1),
  3,
  { 'T','O','Y','O','S','H','I','M','A','-','H','O','U','S','E' },
};

typedef struct { unsigned char bLength; unsigned char bDscType; unsigned short string[10]; } s2;
s2 ud_s2 = {
  sizeof(s2),
  3,
  { 'U','S','B','-','P','S','G',' ','v','1' },
};

qualifier ud_q = {
  sizeof(qualifier),	// Size of descriptor
  6, 			// Device Qualifier Type
  0x200,		// USB2.0
  0,			// Class
  0,			// Sub Class
  0,			// Protocol
  8,			// Buffer Size
  0, 			// Num of Other Configulation
  0,			// Reserved
};

usb_bdt *
usb_get_bdt
(int n)
{
  return (usb_bdt *)(0x400 + n * 4);
}

void
usb_init
(void)
{
  int i;
  for (i = 0; i < 2; i++) {
    usb_bdt *bdt = usb_get_bdt(i);
    unsigned short adr = 0x500 + 64 * i;
    bdt->stat = (i != 0)? 0x00: 0x80;
    bdt->cnt  = (i != 0)? 0x00: 0x40;
    bdt->adrl = adr & 0xff;
    bdt->adrh = adr >> 8;
  }

  UEP0 = 0;		// Initialize EP0
  UEP0bits.EPHSHK = 1;	// Handshake Enable on EP0
  UEP0bits.EPOUTEN = 1;	// Output Enable on EP0
  UEP0bits.EPINEN = 1;	// Input Enable on EP0

  UEP1  = 0; UEP2  = 0; UEP3  = 0; UEP4  = 0; UEP5  = 0;
  UEP6  = 0; UEP7  = 0; UEP8  = 0; UEP9  = 0; UEP10 = 0;
  UEP11 = 0; UEP12 = 0; UEP13 = 0; UEP14 = 0; UEP15 = 0;

  UCFG = 0;
  UCFGbits.UPUEN = 1;	// Pull-up Enable
  UCFGbits.FSEN = 1;	// Full-Speed Enable

  UCON = 0;
  UIE = 0xff;		// Enable All USB Interrupt
  UIR = 0;		// Reset All USB Interrupt Flag
  UEIE = 0xff;
  UEIR = 0;
  PIE2bits.USBIE = 1;	// Enable USB Interrupt
  PIR2bits.USBIF = 0;	// Reset USB Interrupt Flag
  INTCONbits.PEIE = 1;	// Enable Peripheral Interrupt
  UCONbits.USBEN = 1;	// USB Enable
}

void
usb_handler
(void)
{
  static unsigned char uaddr = 0;
  static unsigned char *txbuf = NULL;
  static unsigned char txidx = 0;
  static unsigned char txlen = 0;
  static unsigned short txreq = 0;
  //int i, x, y;
  //far unsigned char *p;
  //  if (UIRbits.URSTIF && (0 == txlen)) {
  if (UIRbits.URSTIF) {
    usb_bdt *bdt = usb_get_bdt(0);
    printf("    Handle Reset\r\n");
    //printf("    UEP%d   : %02x\r\n\r\n", 0, UEP0);
    UADDR = 0;
    UEP0 = 0;			// Initialize EP0
    UEP0bits.EPHSHK = 1;	// Handshake Enable on EP0
    UEP0bits.EPOUTEN = 1;	// Output Enable on EP0
    UEP0bits.EPINEN = 1;	// Input Enable on EP0
    while (UIRbits.TRNIF == 1) {
      UIRbits.TRNIF = 0;
      Nop(); Nop(); Nop();
      Nop(); Nop(); Nop();
    }
    bdt->stat = 0x80;
    bdt->cnt = 0x40;
    UCONbits.PKTDIS = 0;
    UIRbits.URSTIF = 0;
    UIE = 0xff;		// Enable All USB Interrupt
  }
  if (UIRbits.URSTIF) {
    UIRbits.URSTIF = 0;
  }
  if (UIRbits.IDLEIF) {
    UIRbits.IDLEIF = 0;
    //printf("    Handle Suspend\r\n\r\n");
    //UCONbits.SUSPND = 1;
  }
  if (UIRbits.ACTVIF) {
    UIRbits.ACTVIF = 0;
    //printf("    Handle Resume\r\n\r\n");
    //UCONbits.SUSPND = 0;
  }
  if (UIRbits.SOFIF) {
    UIRbits.SOFIF = 0;
    //printf("    Handle Start of Frame\r\n");
  }
  if (UIRbits.TRNIF && USTATbits.DIR) {
    int i;
    usb_bdt *bdt = usb_get_bdt(1);
    unsigned char *buf = (unsigned char *)((bdt->adrh << 8) | bdt->adrl);
    //printf("    Handle Transaction: %02x\r\n", USTAT);
    //printf("    BD%dSTAT: %02x\r\n", 0, bdt->stat);
    //printf("    BD%dCNT : %02x\r\n", 0, bdt->cnt );
    //printf("    BD%dADRL: %02x\r\n", 0, bdt->adrl);
    //printf("    BD%dARRH: %02x\r\n", 0, bdt->adrh);
    //printf("      TXLEN: %d\r\n", txlen);
    //printf("      TXIDX: %d\r\n", txidx);
    //printf("      TX:");
    //for (i = 0; i < bdt->cnt; i++) {
    //printf("%02x,", buf[i]);
    //}
    //printf("\r\n");
    //printf("\r\n");
    txidx += bdt->cnt;
    //printf("%d/%d/%d\r\n", txidx, txlen, txreq);
    if (bdt->cnt != 0) {
      unsigned char len = txlen - txidx;
      if (len > 8) bdt->cnt = 8;
      else bdt->cnt = len;
      memcpy(buf, &txbuf[txidx], bdt->cnt);
      if (((txidx >> 3) & 1) ^ (0 != (txidx & 7))) bdt->stat = 0x88;
      else bdt->stat = 0xc8;
      //printf("      stat: %02x\r\n", bdt->stat);
      //printf("      NX:");
      //for (i = 0; i < bdt->cnt; i++) {
      //printf("%02x,", buf[i]);
      //}
      //printf("\r\n");
    } else {
      txidx = 0;
      bdt->cnt = 0;
    }
    if (0 != uaddr) {
      UADDR = uaddr;
      uaddr = 0;
    }
    UIRbits.TRNIF = 0;
  }
  if (UIRbits.TRNIF && !USTATbits.DIR) {
    if (1 == (USTAT >> 3)) {
      usb_bdt *bdt = usb_get_bdt(2);
      unsigned char * buf = (unsigned char *)((bdt->adrh << 8) | bdt->adrl);
      psg_write(buf[0], buf[1]);
      bdt->stat = 0x80;
      bdt->cnt = 0x40;
    } else if (0 == (USTAT >> 3)) {
      int i;
      unsigned char req;
      unsigned char type;
      unsigned char stat;
      unsigned char didx;
      usb_bdt *bdt0 = usb_get_bdt(0);
      usb_bdt *bdt1 = usb_get_bdt(1);
      unsigned char * buf = (unsigned char *)((bdt0->adrh << 8) | bdt0->adrl);
      //printf("    Handle Transaction: %02x\r\n", USTAT);
      //printf("    BD%dSTAT: %02x\r\n", 0, bdt0->stat);
      //printf("    BD%dCNT : %02x\r\n", 0, bdt0->cnt );
      //printf("    BD%dADRL: %02x\r\n", 0, bdt0->adrl);
      //printf("    BD%dARRH: %02x\r\n", 0, bdt0->adrh);
      //printf("      RX:");
      //for (i = 0; i < bdt0->cnt; i++) {
      //printf("%02x,", buf[i]);
      //}
      //printf("\r\n");
      if (0 == bdt0->cnt) req = 255;
      else req = buf[1];
      type = buf[3];
      txreq = (buf[7] << 8) | buf[6];
      didx = buf[2];
      //if ((6 == req) && (3 == type)) {
      //printf("      RX:");
      //for (i = 0; i < bdt0->cnt; i++) {
      //printf("%02x,", buf[i]);
      //}
      //printf("\r\n");
      //}
      stat = bdt0->stat;
    
      buf = (unsigned char *)((bdt1->adrh << 8) | bdt1->adrl);
      //printf("    Response Buffer: %04x\r\n", ((bdt1->adrh << 8) | bdt1->adrl));
      if (req == 6) {
	if (1 == type) {
	  printf("[GET_DESCRIPTOR(DEVICE)]\r\n");
	  memcpy(buf, &ud_d, 8);
	  txbuf = (unsigned char *)&ud_d;
	  txidx = 0;
	  if (UADDR == 0) txlen = 8;
	  else txlen = sizeof(descriptor);
	  bdt1->cnt = 8;
	  bdt1->stat = 0xc8;
	} else if (2 == type) {
	  printf("[GET_DESCRIPTOR(CONFIGURATION:%d/%d)]\r\n", didx, txreq);
	  memcpy(buf, &ud_c, 8);
	  txbuf = (unsigned char *)&ud_c;
	  txidx = 0;
	  txlen = sizeof(allconfig);
	  if (txreq < txlen) txlen = txreq;
	  bdt1->cnt = 8;
	  bdt1->stat = 0xc8;
	} else if (3 == type) {
	  txbuf = (didx == 0)? (unsigned char *)&ud_s0: (didx == 1)? (unsigned char *)&ud_s1: (unsigned char *)&ud_s2;
	  printf("[GET_DESCRIPTOR(STRING:%d/%d)]\r\n", didx, txreq);
	  memcpy(buf, txbuf, 8);
	  txidx = 0;
	  txlen = txbuf[0];
	  if (txlen > 8) bdt1->cnt = 8;
	  else bdt1->cnt = txlen;
	  bdt1->stat = 0xc8;
	} else if (6 == type) {
	  printf("[GET_DESCRIPTOR(QUALIFIER)]\r\n");
	  memcpy(buf, &ud_q, 8);
	  txbuf = (unsigned char *)&ud_q;
	  txidx = 0;
	  txlen = sizeof(ud_q);
	  bdt1->cnt = 8;
	  bdt1->stat = 0xc8;
	} else {
	  printf("[GET_DESCRIPTOR(%d)]\r\n", type);
	  bdt1->cnt = 0;
	}
      } else if (req == 5) {
	uaddr = didx;
	printf("[SET_ADDRESS] %d\r\n", uaddr);
	txbuf = NULL;
	txidx = 0;
	txlen = 0;
	bdt1->cnt = 0;
	bdt1->stat = 0xc8;
      } else if (req == 9) {
	usb_bdt *bdt = usb_get_bdt(2);
	printf("[SET_CONFIGURATION] %02x%02x\r\n", type, didx);
	txbuf = NULL;
	txidx = 0;
	txlen = 0;
	bdt1->cnt = 0;
	bdt1->stat = 0xc8;

	bdt->stat = 0x80;
	bdt->cnt = 0x40;
	bdt->adrl = (0x500 + 64 * 2) & 0xff;
	bdt->adrh = (0x500 + 64 * 2) >> 8;
	UEP1 = 0;
	UEP1bits.EPHSHK = 1;
	UEP1bits.EPCONDIS = 1;
	UEP1bits.EPOUTEN = 1;
	psg_init();
      } else if (req == 11) {
	printf("[SET_INTERFACE] %02x%02x\r\n", type, didx);
	txbuf = NULL;
	txidx = 0;
	txlen = 0;
	bdt1->cnt = 0;
	bdt1->stat = 0xc8;
      } else if (req == 255) {
	bdt1->cnt = 0;
	//printf("[0B]\r\n");
      } else {
	bdt1->cnt = 0;
	printf("!!! unknown request: %d !!!\r\n", req);
      }
      //printf("      ");
      //for (i = 0; i < bdt1->cnt; i++) {
      //printf("%02x,", buf[i]);
      //}
      //printf("\r\n\r\n");
      //printf("\r\n");
      bdt0->cnt = 0x40;
      bdt0->stat = 0x80;
    } else {
      printf("    ENDP:%d\r\n", USTAT >> 3);
    }
    if (UCONbits.PKTDIS) {
      UCONbits.PKTDIS = 0;
    }
    UIRbits.TRNIF = 0;
  }

  if (UIRbits.UERRIF) {
    printf("USB Error\r\n");
    printf("    UEIR: %02x\r\n\r\n", UEIR);
    UEIR = 0;
    UIRbits.UERRIF = 0;
  }
  if (UIRbits.STALLIF) {
    printf("EP Stalled\r\n\r\n");
    UIRbits.STALLIF = 0;
  }
  //if (0 != UIR) {
  //printf("UIR: %02x\r\n", UIR);
  //}
  //if (UCONbits.PKTDIS) {
  //printf("PKTDIS\r\n");
  //}
  //  for (i = 0; i < 2; i++) {
  //    usb_bdt *bdt = usb_get_bdt(i);
  //    printf("    BD%dSTAT: %02x\r\n", i, bdt->stat);
  //    printf("    BD%dCNT : %02x\r\n", i, bdt->cnt );
  //    printf("    BD%dADRL: %02x\r\n", i, bdt->adrl);
  //    printf("    BD%dARRH: %02x\r\n", i, bdt->adrh);
  //  }
  //  p = (far unsigned char *)0x500;
  //  for (y = 0; y < 8; y++) {
  //    for (x = 0; x < 16; x++) {
  //      printf("%02x,", *p++);
  //    }
  //    printf("\r\n");
  //  }
}

static int volatile int_load_count;
static unsigned int volatile int_count;

void
tmr0_init
(void)
{
  int_load_count = 0;
  int_count = 0;

  T0CONbits.TMR0ON = 1;
  T0CONbits.T08BIT = 1;	// 8bit Counter
  T0CONbits.T0CS = 0;	// Internal 8MHz
  T0CONbits.PSA = 0;	// Prescaler ON
  T0CONbits.T0PS2 = 1;
  T0CONbits.T0PS1 = 0;
  T0CONbits.T0PS0 = 1;	// 1:64(125kHz)
  TMR0L = (256 - 125);	// 1msec Interrupt
  INTCONbits.T0IF = 0;
  INTCONbits.T0IE = 1;
}

void
tmr0_handler
(void)
{
  // load count value
  if (0 != int_load_count) {
    int_count = int_load_count;
    int_load_count = 0;
  }

  // count down
  if (0 != int_count) {
    int_count--;
  }
}

void
init
(void)
{
  // A[0]: ~WR
  // A[1]: ~CS
  // A[2]:  A0
  // A[3]: ~IC
  PORTA = 0x1f;		// All Ports Output High
  TRISA = 0x00;		// All Ports are Output
  ADCON1 = 0x0f;	// All Ports are Digital
  PORTA = 0x1f;		// All Ports Output High
  PORTAbits.RA3 = 0;	// ~IC = 0

  // B[7:0]: D[7:0]
  PORTB = 0x00;		// All Ports Output Low
  TRISB = 0x00;		// All Ports are Output as D[7:0]
  PORTB = 0x00;		// All Ports Output Low

  PORTC = 0xf7;		// All Ports Output High
  TRISC = 0x80;		// [7] is Input, Other ports are Output
  PORTC = 0xf7;		// All Ports Output High
}

static void
isr_high
(void)
  interrupt 1
{
  if (INTCONbits.T0IF) {
    INTCONbits.T0IF = 0;
    tmr0_handler();
  }
  if (PIR2bits.USBIF) {
    PIR2bits.USBIF = 0;
    usb_handler();
  }
}

void
wait_msec
(unsigned int msec)
{
  if (0 == msec) return;
  int_load_count = msec;
  while (0 != int_load_count);
  while (0 != int_count);
}

int
main
(void)
{
  init();
  usart_init();
  INTCONbits.GIE = 1;	// Enable Interrupt

  printf("booting pic18f2550...\r\n");
  printf("\tinitialize timer...\r\n");
  tmr0_init();
  wait_msec(250);
  printf("\tinitialize usb...\r\n");
  usb_init();
  printf("ready\r\n");
  while (1) {
    //PORTC = 0x80;
    wait_msec(250);
    //PORTC = 0x40;
    wait_msec(250);
    //PORTC = 0x00;
    wait_msec(250);
    //PORTC = 0xc0;
    wait_msec(250);
  }
  return 0;
}
