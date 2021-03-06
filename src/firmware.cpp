/***************************************************************************
 *            firmware.c
 *
 *  Sund Aug 31 18:57:43 2008
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


/*
 * this file contains everything to upload new firmware into the programmer
 * the following functions can be called from outside:
 *
 * int32_t firm_boot_on(void)  activate bootloader
 * int32_t firm_upload(void)  upload firmware
 * int32_t firm_boot_off(void)  de-activate bootloader
 */


//********************************************************************************************************************
// includes
//********************************************************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <usb.h>
#include "b8.h"


//********************************************************************************************************************
// Routinen zum einshreiben neuer Firmware
//********************************************************************************************************************

// activate bootloader
int32_t firm_boot_on(void)
{
    int32_t start = 0xFE;
    int32_t length = 1;
  unsigned char src[] = { 0xFF };
  prog_write_eedata(start, src, length);

  prog_reset();
  return 0;
}



//upload firmware
int32_t firm_upload(void)
{
  prog.core = CORE_16;
  prog.max_flash = 0x7FFF;
  EraseTPIC(prog.HexIn);
  if (openhexfile() != 0) return(-1);
  fprintf(stdout, ">>programm control PIC-firmware ");
  //unsigned char cmd[USB_BLOCKSIZE];
  unsigned char buf[USB_BLOCKSIZE];

  //brennen
    uint32_t Adresse    = 0x0800;
    uint32_t Endadresse = 0x7FFF;
  while (Adresse < Endadresse)
  {
        //64 uint8_t loeschen
    unsigned char cmd[USB_BLOCKSIZE] = { BOOT_ERASE_FLASH, 
        (unsigned char) 0x01,        // 1 x 64 uint8_t
                (unsigned char) (Adresse      ),  // low
                (unsigned char) (Adresse >>  8),    // high
                (unsigned char) (Adresse >> 16) };  // upper
        int32_t rc;
    rc = programmer_command(cmd, 5, buf);
    //fprintf(stdout, "E: %d ", rc);
    if (rc < 0) return rc;
    if (rc < 1) return -EFAULT;  // 1 Zeichen zurueck
    if (memcmp(buf, cmd, 1)) return -EFAULT;  

    //4 x 16 uint8_t schreiben
        for (int32_t k=0; k<=3; k++)
    {
      // The write holding register for the 18F4550 family is
      // actually 32-uint8_t. The code below only tries to write
      // 16-uint8_t because the GUI program only sends out 16-uint8_t
      // at a time.
      // This limitation will be fixed in the future version.
      // LEN = # of uint8_t to write  
      unsigned char cmd[USB_BLOCKSIZE] = { BOOT_WRITE_FLASH, 
          (unsigned char) 0x10,        // Laenge 16 uint8_t
                    (unsigned char) (Adresse      ),  // low
                    (unsigned char) (Adresse >>  8),  // high
                    (unsigned char) (Adresse >> 16) };  // upper
            for (int32_t L=0; L<=15; L++) cmd[5+L]= (unsigned char) prog.HexIn.Flash[Adresse+L] & 0xFF;
      rc = programmer_command(cmd, 21, buf);
      //fprintf(stdout, "W: %d ", rc);
      if (rc < 0) return rc;
      if (rc < 1) return -EFAULT;  // 1 Zeichen zurueck
      if (memcmp(buf, cmd, 1)) return -EFAULT;  

      Adresse += 16;
    };
    printf("."); fflush(stdout);
  }; //while
  printf("\n"); fflush(stdout);

    //proefen
  fprintf(stdout, ">>check control PIC-firmware ");
    int32_t Fehler = 0;
  Adresse    = 0x0800;
  Endadresse = 0x7FFF;
  while (Adresse < Endadresse)
  {
    unsigned char cmd[USB_BLOCKSIZE] = { BOOT_READ_FLASH, 
        (unsigned char) 0x10,        // Laenge 16 uint8_t
                (unsigned char) (Adresse      ),  // low
                (unsigned char) (Adresse >>  8),    // high
                (unsigned char) (Adresse >> 16)};  // upper
        int32_t rc;
    rc = programmer_command(cmd, 5, buf);
    if (rc < 0) return rc;
    if (rc < 21) return -EFAULT;  // 21 Zeichen zurueck
    if (memcmp(buf, cmd, 1)) return -EFAULT;  

        for (int32_t k=0; k<=buf[1]-1; k++)
    {
      if ((buf[k+5] & 0xFF) != (prog.HexIn.Flash[Adresse+k] & 0xFF))
      {
        Fehler++;
        if (f_i) fprintf(stdout,"\n Adr: %4x Pic %4x HexIn %4x ", Adresse+k, buf[k+5] & 0xFF, prog.HexIn.Flash[Adresse+k] & 0xFF);
      };
    };
    Adresse += 16;
    printf("."); fflush(stdout);
  };
  printf("\n"); fflush(stdout);
  return(Fehler);
}



// de-activate bootloader
int32_t firm_boot_off(void)
{
    // Firmwaresignatur loeschen
    // EEPROM-Zelle 0xFC=$FF   $FD=$FF
    // Bootloader deaktivieren
    // EEPROM-Zelle 0xFE (und 0xFF) mit 0 beschreiben
    int32_t start = 0xFC;
    int32_t length = 4;
  unsigned char src[4] = {0xFF, 0xFF, 0x00, 0x00 };
  prog_write_eedata(start, src, length);

  puts(">> now remove the jumper (if there is one) and press ENTER to continue");
  getc (stdin);

  prog_reset();
  return 0;
}

