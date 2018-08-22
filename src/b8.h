/***************************************************************************
 *            b8.h
 *
 *  Wed Aug 23 18:57:43 2006
 *  Copyright  2006  Florian Demski
 *  Copyright  2008  Joerg Bredendiek (sprut)
 *
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __B8_H__
#define __B8_H__


//********************************************************************************************************************
// includes
//********************************************************************************************************************
#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <errno.h>
#include <sys/types.h>
#include <math.h>
#include <string>

#include <stdlib.h>
#include <getopt.h>

using namespace std;

typedef union _BYTE
{
    uint8_t _byte;
    struct
    {
        unsigned b0:1;
        unsigned b1:1;
        unsigned b2:1;
        unsigned b3:1;
        unsigned b4:1;
        unsigned b5:1;
        unsigned b6:1;
        unsigned b7:1;
    };
} BYTE;


#define SPRUT_VID    0x04D8
#define SPRUT_PID    0xFF0B

#define tietel      "usburn V 0.4 (16.05.2010) by sprut (www.sprut.de)"

#define CMD_READ_VERSION        0x00  /* read programmer id and firmware version */
#define CMD_LED_ONOFF           0x31  /* set LEDs */
#define CMD_RD_ADC              0x37  /* get current ADC value */
#define CMD_SET_AN              0x38  /* select ADC input */
#define CMD_SET_PWM             0x39  /* set pwm value */
#define CMD_SET_SOC             0x3A  /* select socket */
#define CMD_SET_SIGNAL    0x3B  /* set single signals */
#define CMD_SET_DIR    0x3C  /* set direction of PGD */
#define CMD_READ_DATA           0x3D  /* read ICSP data line */
#define CMD_SET_CORE            0x40  /* set PIC family */
#define CMD_SET_PICTYPE         0x41  /* supply information regarding the PIC */
#define CMD_SET_ADDRESS         0x42  /* set address range */
#define CMD_SET_VPP             0x43  /* set Vpp voltage */
#define CMD_READ_EEDATA         0x44  /* read EEPROM of controller PIC */
#define CMD_WRITE_EEDATA        0x45  /* write EEPROM of controller PIC */
#define CMD_READ_CHIPID         0x50  /* read chip id of target */
#define CMD_READ_FLASH          0x51  /* read flash of target */
#define CMD_READ_EEPROM         0x52  /* read EEPROM of target */
#define CMD_READ_IDDATA         0x53  /* read id data of target */
#define CMD_READ_CONFIG         0x54  /* read config data of target */
#define CMD_WRITE_FLASH         0x60  /* write flash of target */
#define CMD_WRITE_EEPROM        0x61  /* write EEPROM of target */
#define CMD_WRITE_IDDATA        0x62  /* write id data of target */
#define CMD_WRITE_CONFIG        0x63  /* write config data of target */
#define CMD_ERASE               0x70  /* erase target */
#define CMD_REMOVECP            0x71  /* remove code protection of target (bulk erase) */
#define CMD_SUPPORTED           0x72    /* ask for supported PIC-families*/
#define CMD_PSUMM            0x73  /* calculate checksum of mirmware */
#define CMD_RESET               0xFF  /* reset controller PIC */

#define BOOT_READ_VERSION       0x00  /* read programmer id and firmware version */
#define BOOT_READ_FLASH         0x01  /* read flash of controller PIC */
#define BOOT_WRITE_FLASH        0x02  /* write flash of controller PIC */
#define BOOT_ERASE_FLASH        0x03  /* erase flash of controller PIC */
#define BOOT_READ_EEDATA        0x04  /* read EEPROM of controller PIC */
#define BOOT_WRITE_EEDATA       0x05  /* write EEPROM of controller PIC */
#define BOOT_READ_CONFIG        0x06  /* read config data of controller PIC */
#define BOOT_WRITE_CONFIG       0x07  /* write config data of controller PIC */
#define BOOT_RESET              0xFF  /* reset controller PIC */

/* led codes used by CMD_LED_ONOFF */
#define LED1_ON          0x00  /* green */
#define LED1_OFF        0x04  /* green */
#define LED2_ON          0x01  /* yellow */
#define LED2_OFF        0x05  /* yellow */

/* inputs used by CMD_SET_AN */
#define ADC_VPP          0x00
#define ADC_VZ          0x01

/* sockets used by CMD_SET_SOC */
#define SOC_8_14        0x00
#define SOC_18_ICSP        0x01
#define SOC_ICSP        0x01
#define SOC_28_40        0x02
#define SOC_RUN          0xFE
#define SOC_OFF          0xFF

/* signals used by CMD_SET_SIGNAL */
#define SIG_VDD_ON        0x01
#define SIG_VDD_OFF        0x10
#define SIG_VPP_ON        0x02
#define SIG_VPP_OFF        0x20
#define SIG_PGD_ON        0x04
#define SIG_PGD_OFF        0x40
#define SIG_PGC_ON        0x08
#define SIG_PGC_OFF        0x80

/* modes used by CMD_SET_DIR */
#define DIR_PGD_INPUT        0x00
#define DIR_PGD_OUTPUT        0x01

/* cores used by CMD_SET_CORE */
#define CORE_12          12
#define CORE_14          14
#define CORE_16          16
#define CORE_17          17
#define CORE_18          18
#define CORE_30          30
#define  CORE_33          33

#define MODE_NORMAL             0x01  /* programmer in normal mode */
#define MODE_BOOT               0x02  /* programmer in boot loader mode */

#define MODE_INT                0x04  /* usb interface : interrup mode */
#define MODE_BULK               0x08  /* usb interface : bulk mode */

#define  DEVICE_B8    0x00  // Brenner8
#define  DEVICE_BOOT    0x01  // Bootloader
#define  DEVICE_B9    0x03  // Brenner9

#define TIMEOUT_WRITE      100
#define TIMEOUT_READ      1000

#define USB_BLOCKSIZE      64
#define CTRL_EEPROM_SIZE    0x100
#define RES_CALIB_NEEDED    1
#define CALIB_FIRST_TRY      18    /* value used as first estimate */

// Flags
#define f_r  prog.flags1.b0
#define f_w  prog.flags1.b1
#define f_c  prog.flags1.b2
#define f_e  prog.flags1.b3
#define f_p  prog.flags1.b4
#define f_i  prog.flags1.b5
#define f_d  prog.flags1.b6
#define f_l  prog.flags1.b7

#define f_a  prog.flags2.b0
#define f_b  prog.flags2.b1
#define f_o  prog.flags2.b2
#define f_u  prog.flags2.b3
#define f_s  prog.flags2.b4
#define f_h  prog.flags2.b5
#define f_S  prog.flags2.b6
#define f_F  prog.flags2.b7

#define f_n  prog.flags3.b0
#define f_f  prog.flags3.b1
#define f_k  prog.flags3.b2
#define f_t  prog.flags3.b3

// Databasefilenames
#define  P_filename  "picdef03.dat"
#define  C_filename  "cfgdef03.dat"
#define  F_filename  "fildef03.dat"
#define  S_filename  "setdef03.dat"
#define  B_filename  "cekdef03.dat"  //checksum
#define  T_filename  "texdef03.dat"
// max number of datasets in databasefiles
#define  P_filesize  800
#define  C_filesize  6000
#define  F_filesize  20000
#define  S_filesize  60000

// opcodes to put OSCCAL into Flash
#define  MOVLW_12  0x0C00
#define  RETLW_14  0x3400
#define no_OSCCAL  0x1000    // value for no OSCCAL on comandline

/**
 * @todo comment
 * @brief TBD
 */
class TPIC
{
public:
  uint16_t  Flash[0x40000];
  uint16_t  ROM[0x1000];
  uint16_t  ID[8];
  uint16_t   Config[24];
};

/**
 * @todo comment
 * @brief TBD
 *        Size: 22 uint8_t
 */
typedef struct
{
  int32_t  Nr;
  int32_t  Key;
  uint32_t addr;  //!< 32 bit unsigned
  uint16_t  unused;
  int32_t  cfgbitsnr;
  int32_t  fieldNr;
} TCfgbits;

/**
 * @todo comment
 * @brief TBD
 *        Size: 25 uint8_t
 */
typedef struct
{
  int32_t  Nr;
  int32_t  Key;
  uint16_t  mask;
  int32_t  desc;
  uint8_t  flags;    //!< 0:-  1:h  2:xh
  uint16_t  init;
  int32_t  fieldNr;
  int32_t  settingNr;
} TField;

/**
 * @todo comment
 * @brief TBD
 *        Size: 20 uint8_t
 */
typedef struct
{
  int32_t  Nr;
  uint16_t  req;
  uint16_t  value;
  int32_t  desc;
  int32_t  settingNr;
  int32_t  checksum;
} TSetting;

/**
 * @todo comment
 * @brief TBD
 *        Size: 12 uint8_t
 */
typedef struct
{
  int32_t  Nr;
  uint16_t  typ;
  uint16_t  protstart;
  int32_t  protende;
} TChecksum;

#pragma pack(push, 8)          /* set alignment to 8 -- really important */

/**
 * @brief datastructure for the programmer
 */
typedef struct T_taktik  // 7 uint8_t
{
  uint8_t  flash;
  uint8_t  eeprom;
  uint8_t  id;
  uint8_t  config;
  uint8_t  erase;
  uint8_t  cp;
  uint8_t  read_eeprom;
} T_taktik;

//Struktur of the database
typedef struct
{
  char name[21];
  uint8_t cpu;
  uint8_t power;
  uint8_t fill1;  //unpacked
  __int32_t config;
  uint8_t software;
  uint8_t fill2;  //unpacked
  uint8_t fill3;  //unpacked
  uint8_t fill4;  //unpacked
  __int32_t blocksize;
  uint8_t pins;
  char ExtraStr[17];
  uint8_t fill5;  //unpacked
  uint8_t fill6;  //unpacked
  __int32_t ExtraInt;
  uint8_t ExtraBool;

  struct
  {
    uint8_t io;
    uint8_t adc;
    uint8_t adctyp;
    uint8_t uart;
    uint8_t spi;
    uint8_t i2c;
    uint8_t can;
    uint8_t usb;
    uint8_t timer;
    uint8_t compare;
    uint8_t capture;
    uint8_t pwm;
    uint8_t ccp;
    uint8_t eccp;
    uint8_t ssp;
    uint8_t ext;
    uint8_t fill7;  //unpacked
    uint8_t fill8;  //unpacked
    uint8_t fill9;  //unpacked
  } interfaces;

  struct
  {
    double min;
    double max;
    double deflt;
  } vpp;

  struct
  {
    double min;
    double max;
    double dfltmin;
    double dfltmax;
    double nominal;
  } vdd;

  struct
  {
    uint8_t memtech, ovrpgm, tries;
    uint8_t fill10;  //unpacked
    uint8_t fill11;  //unpacked
    uint8_t fill12;  //unpacked
    uint8_t fill13;  //unpacked
    uint8_t fill14;  //unpacked
    double lvpthresh;
    uint32_t panelsize;
    uint8_t fill15;  //unpacked
    uint8_t fill16;  //unpacked
    uint8_t fill17;  //unpacked
    uint8_t fill18;  //unpacked
    uint8_t fill19;  //unpacked
    uint8_t fill20;  //unpacked
  } pgming;

  struct
  {
    uint16_t pgm;
    uint16_t lvpgm;
    uint16_t eedata;
    uint16_t cfg;
    uint16_t userid;
    uint16_t erase;
    uint16_t lverase;
  } wait;

  struct
  {
    uint8_t pgm;
    uint8_t eedata;
    uint8_t userid;
    uint8_t cfg;
    uint8_t rowerase;
    uint8_t fill21;  //unpacked
  } latches;

  struct
  {
    int32_t min;
    int32_t max;    // 32 Bit mit vorzeichen, klappt so nicht unter 64-bit-Linux
  } pgmmem;

  struct
  {
    int32_t min;
    int32_t max;
  } eedata;

  struct
  {
    int32_t min;
    int32_t max;
    int32_t modeaddr;
  } extpgm;

  struct
  {
    int32_t min;
    int32_t max;
  } cfgmem;

  struct
  {
    int32_t min;
    int32_t max;
  } calmem;

  struct
  {
    int32_t min;
    int32_t max;
  } userid;

  struct
  {
    uint32_t min;
    uint32_t max;
    uint32_t idmask;
    uint32_t id;
  } devid;

  T_taktik taktik;

  uint8_t fill21;  //unpacked
  uint8_t fill22;  //unpacked
  uint8_t fill23;  //unpacked
  uint8_t fill24;  //unpacked
  uint8_t fill25;  //unpacked
} TPicDef;

typedef struct T_latches // Schreibpuffergroeßen in uint8_t
                         // 5 uint8_t
{
  uint8_t  pgm ;
  uint8_t  eedata;
  uint8_t  userid;
  uint8_t  cfg;
  uint8_t  rowerase;    // Loeschbereich
                                                    // in uint8_t
} T_latches;
typedef struct T_wait    // alle Brenn-Zeiten in Mikrosekunden
                         // 14 uint8_t
{
  uint16_t  pgm;
  uint16_t  lvpgm;
  uint16_t  eedata;
  uint16_t  cfg;
  uint16_t  userid;
  uint16_t  erase;
  uint16_t  lverase;
} T_wait;
typedef struct T_PICtype // 9 + 7 + 5 + 14 = 35 uint8_t
{
  uint8_t  cpu;
  uint8_t  power;
//     uint32_t blocksize;
  uint16_t  blocksize;
  uint8_t  pins;
  uint8_t  vpp;
  uint16_t  panelsize;
  T_taktik  taktik;
  T_latches  latches;
  T_wait    wait;
} T_PICtype;



#pragma pack(pop)

struct programmer {
  usb_dev_handle *dev;
  int32_t interface;
  int32_t ep_in, ep_out;
  int32_t usbmode;
  int32_t mode;

  int32_t fw;      // version (e.g. firmwareversion)
  int32_t device;    // device: 0-Brenner8; 1=Bootloader 2=USB4A 3=Brenner9
  char usb_name[64];
  unsigned char supp;

  double refZ, divU;
  double gainOff, pwm0VOff, gainOn, pwm0VOn;
  double vdd;

  int32_t in_calibration, block_timer;
  char VppLoopMode;  //0-nichts / 1-immer / 2-einmalig / 3- nur herunterregeln
  unsigned char socket, core;

  double d_Uz;    //Referenzspannung Z-Diode
  double d_DIV;    //VppSpannungsteiler
  double d_gain_off;
  double d_pwm0v_off;
  double d_gain_on;
  double d_pwm0v_on;
  double d_Vusb_cal;  // Betriebsspannung bei kalibrierung
  double d_korr;    //Ukor
  float U00_off;
  float U00_on;
  BYTE flags1;
  BYTE flags2;
  BYTE flags3;
  uint8_t ADCL;
  uint8_t ADCH;

  int32_t max_flash;
  int32_t max_ee;
  int32_t EndFlash;
  int32_t EndEE;
  uint16_t chipid;
  uint16_t revision;

  TPicDef pic;              //!< alles ueber den PIC fuer den Brenner
  TPIC HexIn, HexOut;
  uint16_t Calmem[40000];
  char calmemsaved;
  char InHexfilename[255];
  char OutHexfilename[255];
  char picname[21];
  char OsccalString[10];
  uint16_t OsccalRom;
  int32_t OsccalPar;        //!< OSCCAL-valu from komandline option, cvan be negative vor PIC10F
  uint16_t BGmask;
  int32_t BGadr;
  uint16_t BGvalue;
  char BGString[10];
  uint16_t BGnewvalue;
  uint8_t WRITE_EDATA_KEY;
};

extern struct programmer prog;



#ifdef __cplusplus
extern "C" {
#endif



/* programmer_usb.c */
void sleepms(uint32_t mseconds);
unsigned char d2c(double d);

/**
 * @brief Init usburn with default values
 */
void init_system(void);

/**
 * @brief Close usb connections
 */
void cleanup_system(void);
int32_t search_brenner8(void);
int32_t programmer_command(unsigned char *data_in, int32_t datasz, unsigned char *data_out);

int32_t prog_get_version(void);
int32_t prog_set_led(unsigned char led);
void prog_get_supported(void);
int32_t prog_read_adc(void);
int32_t prog_set_an(unsigned char an);
int32_t prog_set_signal(unsigned char signal);
int32_t prog_set_pwm(unsigned char pwm_off, unsigned char pwm_on);
int32_t prog_set_vpp(unsigned char vppl, unsigned char vpph);
int32_t prog_read_eedata(uint32_t start, uint32_t length, unsigned char *dst);
int32_t prog_write_eedata(int32_t start, unsigned char *src, int32_t length);
int32_t prog_reset(void);
int32_t prog_read_chipid(void);
int32_t prog_set_address(int32_t start, int32_t stop);
void EraseTPIC(TPIC & PIC);
int32_t prog_read_flash(int32_t start, int32_t stop);
int32_t prog_read_calmem(int32_t start, int32_t stop);
int32_t prog_read_ee(int32_t start, int32_t stop);
int32_t prog_read_ID(void);
int32_t prog_read_CONFIG(void);
int32_t prog_check_firmware(void);
int32_t prog_write_flash();
int32_t prog_write_ee();
int32_t prog_write_config(void);
int32_t prog_write_id(void);
int32_t prog_erase(void);
int32_t prog_removecp(void);
int32_t prog_set_core(unsigned char core);
int32_t prog_set_pictype(TPicDef& pic);
int32_t prog_set_socket(unsigned char socket);
int32_t prog_target_run(void);
int32_t prog_regulate_vpp(double lower, double mid, double upper);
int32_t prog_identify(unsigned char core, unsigned char socket);


/* firmware.c */
int32_t firm_boot_on(void);
int32_t firm_upload(void);
int32_t firm_boot_off(void);


/* calibration.c */
int32_t cal_step3(void);      // Messung der PWM-Regelsteilheit
int32_t cal_write_data(void);    // write calibration data to control PIC
void cal_read_data(void);    // read calibration data from control PIC

int32_t vpp_loop_on(int32_t mode);
int32_t vpp_loop_off(void);
float cal_Kalibrierespannung(void);
float vpp_getADC(int32_t kanal, int32_t zyklen);
float vpp_getVoltage(int32_t kanal, int32_t Zyklen);
float vpp_getVpp(void);
float vpp_getVpp_stable(void);
void vpp_setVpp(float VppSoll);


/* database.c */
int32_t db_findpicname(TPicDef& pic);
int32_t db_listpics();
int32_t db_findpicid(uint16_t picid, TPicDef& pic);
int32_t db_load_db(void);
uint16_t db_getdefConfMask(int32_t adr);
void db_find_BG(void);


/* hexfile.c */
string hex2str(uint8_t zahl);
int32_t idNr(int32_t adr);
int32_t confNr(int32_t adr);
uint16_t PICzelle(int32_t adr);
//uint16_t getleerwert(int32_t adr, unsigned char core);
uint16_t getleerwert(int32_t adr);
int32_t savehex12(int32_t anfang, int32_t ende);
int32_t savehex14(int32_t anfang, int32_t ende);
int32_t savehex16(int32_t anfang, int32_t ende);
int32_t savehex30(int32_t anfang, int32_t ende);
int32_t SaveHexout12(void);
int32_t SaveHexout14(void);
int32_t SaveHexout16(void);
int32_t SaveHexout30(void);
int32_t savehexout(void);
int32_t openhexfile(void);


/* testc */
int32_t test_hardware(void);


#ifdef __cplusplus
}
#endif

#endif /* __B8_H__ */

