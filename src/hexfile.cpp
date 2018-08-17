/***************************************************************************
 *            hexfile.c
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
 * this file contains everything to read and write hex-files with usburn
 * the following functions can be called from outside:
 *
 * int32_t savehexout(void)    writes prog.HexOut into a HEX-file
 * int32_t openhexfile(void)  reads a HEX-Fie into prog.HexIn
 */


//********************************************************************************************************************
// includes
//********************************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <usb.h>
#include <string>
#include "b8.h"


//********************************************************************************************************************
// Definitionen u.s.w.
//********************************************************************************************************************

using namespace std;

FILE *outdatei;
FILE *indatei;

//********************************************************************************************************************
// Hilsroutinen
//********************************************************************************************************************

//wandelt eine ziffer in char
char int2char(int32_t zeichen)
{
  if (zeichen > 15) return(0);
  if (zeichen <  0) return(0);
  switch (zeichen)
  {
  case  0: return('0'); break;
  case  1: return('1'); break;
  case  2: return('2'); break;
  case  3: return('3'); break;
  case  4: return('4'); break;
  case  5: return('5'); break;
  case  6: return('6'); break;
  case  7: return('7'); break;
  case  8: return('8'); break;
  case  9: return('9'); break;
  case 10: return('A'); break;
  case 11: return('B'); break;
  case 12: return('C'); break;
  case 13: return('D'); break;
  case 14: return('E'); break;
  case 15: return('F'); break;
  default: return('X');
  }
}



//wandelt eine int8-zahl in einen 2-stelligen hex-string um
string hex2str(uint8_t zahl)
{
  string s;
  s =  int2char(zahl >> 4);
  s += int2char(zahl & 0x0F);
  return(s);
}

/**
 * @brief wandelt 16-Bit in eines string lsb first
 * @param zeiger: Der umzuwandelnde integer
 * @param sum:    checksum to update
 * @return string with 2 hex signes
 */
string zweiByte(int32_t zeiger, int32_t &sum)
{
  int32_t wert = 0;
  int32_t Adrr = zeiger;
  int8_t teil;
  string s = "";
  if ((zeiger >= 0x000000) && (zeiger <= 0x02ABFF)) wert = prog.HexOut.Flash[Adrr];
  if ((zeiger >= 0xF80000) && (zeiger <= 0xF8000D)) wert = prog.HexOut.Config[Adrr-0xF80000];
  if ((zeiger >= 0x7FF000) && (zeiger <= 0x7FFFFF)) wert = prog.HexOut.ROM[Adrr-0x7FF000];
  //LSB first
  teil = static_cast<uint8_t>(wert);
  sum = sum + teil;
  s = s + hex2str(teil);
  teil = static_cast<uint8_t>(wert  >> 8);
  sum = sum + teil;
  s = s + hex2str(teil);
  return(s);
}

/**
 * @brief liefert die Adressen fuer ID-Zellen
 * @param adr: TBD
 * @return TBD
 */
int32_t idNr(int32_t adr)
{
  if ((adr >=   0x2000) && (adr <=   0x2004)) return(adr -   0x2000);  // 14
  if ((adr >= 0x200000) && (adr <= 0x200007)) return(adr - 0x200000);  // 16
  if ((adr >= 0xFF0000) && (adr <= 0xFF0003)) return(adr - 0xFF0000);  // 33
  return(adr - prog.pic.userid.min);
}


// liefert die Adressen fuer Config-Zellen
int32_t confNr(int32_t adr)
{
  if ((adr >= 0x002007) && (adr <= 0x00200E)) return(adr - 0x002007);  // 14-Bit-Kern
  if ((adr >= 0x00FFF8) && (adr <= 0x00FFFF)) return(adr - 0x00FFF8);  // PIC18FxxJxx mit  64 kByte Flash
  if ((adr >= 0x017FF8) && (adr <= 0x017FFF)) return(adr - 0x017FF8);  // PIC18FxxJxx mit  96 kByte Flash
  if ((adr >= 0x01FFF8) && (adr <= 0x01FFFF)) return(adr - 0x01FFF8);  // PIC18FxxJxx mit 128 kByte Flash
  if ((adr >= 0x300000) && (adr <= 0x30000F)) return(adr - 0x300000);  // pic18F , PIC18FxxJ
  if ((adr >= 0xF80000) && (adr <= 0xF80017)) return(adr - 0xF80000);  // dsPIC30 ,dsPIC33 , PIC24
  return(adr - prog.pic.cfgmem.min);
}


// liefert den Zur Adresse passenden Zellenwert
uint16_t PICzelle(int32_t adr)  //, unsigned char core)
{
  unsigned char core = prog.core;
  int32_t buffer;
  buffer = 0;
  switch (core)
  {
  case CORE_12:
    buffer = 0;
    if ((adr>=prog.pic.pgmmem.min) &&  (adr<=prog.pic.pgmmem.max)) buffer = prog.HexOut.Flash[adr];
    if ((adr>=prog.pic.userid.min) &&  (adr<=prog.pic.userid.max)) buffer = prog.HexOut.ID[idNr(adr)];
    if ((adr>=prog.pic.cfgmem.min) &&  (adr<=prog.pic.cfgmem.max)) buffer = prog.HexOut.Config[confNr(adr)];
    if ((adr>=prog.pic.eedata.min) &&  (adr<=prog.pic.eedata.max)) buffer = prog.HexOut.ROM[adr-0x2100];
    buffer = buffer & 0xFFFF;
  break;
  case CORE_14:
    if ((adr>=     0) && (adr<=0x1FFF)) buffer = prog.HexOut.Flash[adr];
    if ((adr>=0x2000) && (adr<=0x2004)) buffer = prog.HexOut.ID[idNr(adr)];
    if ((adr>=0x2007) && (adr<=0x200A)) buffer = prog.HexOut.Config[confNr(adr)];
    if ((adr>=0x2100) && (adr<=0x21FF)) buffer = prog.HexOut.ROM[adr - 0x2100];
    buffer = buffer & 0xFFFF;
  break;
  case CORE_16:
  case CORE_17:
  case CORE_18:
    if ((adr>=       0) && (adr<=0x01FFFF)) buffer = prog.HexOut.Flash[adr] & 0xFF;
    if ((adr>=0x200000) && (adr<=0x200007)) buffer = prog.HexOut.ID[adr - 0x200000];
    if ((adr>=0x300000) && (adr<=0x30000D)) buffer = prog.HexOut.Config[adr - 0x300000];
    if ((adr>=0xF00000) && (adr<=0xF003FF)) buffer = prog.HexOut.ROM[adr - 0xF00000];
  break;
  }
    return(buffer);
}


// Wert einer leeren Zelle
uint16_t getleerwert(int32_t adr)
{
  uint16_t result = 0x3FFF;
  switch (prog.core)
  {
    case CORE_12:
      result = 0x0FFF;
      if ((adr>=prog.pic.pgmmem.min) &&  (adr<=prog.pic.pgmmem.max)) result = 0x0FFF; //Flash
      if ((adr>=prog.pic.userid.min) &&  (adr<=prog.pic.userid.max)) result = 0x000F;  //ID
      if ((adr>=prog.pic.cfgmem.min) &&  (adr<=prog.pic.cfgmem.max)) result = 0x0FFF;  //Config
      if ((adr>=prog.pic.eedata.min) &&  (adr<=prog.pic.eedata.max)) result = 0x00FF;  //EEPROM
      break;
    case CORE_14:
      result = 0x3FFF;
      if ((adr >= 0x0000) && (adr <=0x1FFF)) result = 0x3FFF;  // flash
      if ((adr >= 0x2000) && (adr <=0x2003)) result = 0x000F;  // id
      if ((adr >= 0x2004) && (adr <=0x200A)) result = 0x3FFF;  // debug, config
      if ((adr >= 0x2100) && (adr <=0x21FF)) result = 0x00FF;  // eeprom
      break;
  }
  return(result);
}



//********************************************************************************************************************
// Routinen zum Schreiben eines HEX-Files
//********************************************************************************************************************


// Schreiben von prog.HexOut in ein Hex-File fuer 12-Bit-Kern-PICs -Unterroutine
int32_t savehex12(int32_t anfang, int32_t ende)
{
  string s;
  int32_t bytezahl;
  int32_t adressex2;
  int32_t sum;
  int32_t zeiger = anfang;
  uint16_t leerwert;

      leerwert = getleerwert(zeiger);
      if (anfang > ende) return(0);

  //while loop wird fuer jede Zeile im HEX-File durchlaufen
  while (zeiger <= ende)
  {
    bytezahl = 0;
    sum = 0;
    s = "";
    // suche naechste Zelle mit Daten
    while(((PICzelle(zeiger) & leerwert) == leerwert) && (zeiger <= ende))
      zeiger++;
    if(zeiger > ende)
      return(0); //keine Daten mehr
    adressex2 = zeiger * 2;

    while(((PICzelle(zeiger) & leerwert) != leerwert) &&
          (zeiger <= ende) &&
          (((zeiger % 0x08) !=0) || (bytezahl == 0)) &&
          (bytezahl < 0x10) )
    {
      s = s + hex2str(PICzelle(zeiger) % 256) + hex2str(PICzelle(zeiger) / 256);
      sum = sum + (PICzelle(zeiger) % 256) + (PICzelle(zeiger) / 256);
      bytezahl++; bytezahl++;
      zeiger++;
      //if (zeiger > ende) then break;
    }
    sum = sum + bytezahl + (adressex2 / 256) + (adressex2 % 256);
    sum = sum % 256;
    sum = 256 - sum;
    s = ":" + hex2str(bytezahl) + hex2str(adressex2 / 256) + hex2str(adressex2 % 256) + "00" + s + hex2str(sum % 256);
    if (bytezahl > 0) fprintf(outdatei, "%s\n", s.c_str());
  } //while  loop
  return(0);
}



// Schreiben von prog.HexOut in ein Hex-File fuer 14-Bit-Kern-PICs -Unterroutine
int32_t savehex14(int32_t anfang, int32_t ende)
{
    string s;
    int32_t bytezahl;
    int32_t adressex2;
    int32_t sum;
    int32_t zeiger;
  uint16_t leerwert;
  zeiger = anfang;  

  //Suchen der letzten belegten Zelle
  leerwert = getleerwert(zeiger);
  while (((PICzelle(ende) && leerwert) == leerwert) && (anfang < ende)) ende--;
  if (anfang == ende) return(0);

  //while loop wird fuer jede Zeile im HEX-File durchlaufen
  while (zeiger <= ende)
  {
    bytezahl = 0;
    sum = 0;
    s = "";  
    // suche naechste Zelle mit Daten
    while (((PICzelle(zeiger) && leerwert) == leerwert) && (zeiger <= ende)) zeiger++;
    if (zeiger <= ende)
    {
      adressex2 = zeiger * 2;
      while (((PICzelle(zeiger) && leerwert) != leerwert) 
        && (zeiger <= ende) 
        && (((zeiger % 0x08) != 0) || (bytezahl == 0)) 
        && (bytezahl < 0x10) )
      {
        s += hex2str(PICzelle(zeiger) % 256);
        s += hex2str(PICzelle(zeiger) / 256);
        sum = sum + (PICzelle(zeiger) % 256) + (PICzelle(zeiger) / 256);
        bytezahl++; bytezahl++;
        zeiger++;
      }
      sum = sum + bytezahl + (adressex2 / 256) + (adressex2 % 256);
      sum = sum % 256;
      sum = 256 - sum;
      s = ":" + hex2str(bytezahl) + hex2str(adressex2 / 256) + hex2str(adressex2 % 256) + "00" + s + hex2str(sum % 256);
      if (bytezahl > 0) fprintf(outdatei, "%s\n", s.c_str());
    }
  }
  return(0);
}


// Schreiben von prog.HexOut in ein Hex-File fuer 16-Bit-Kern-PICs -Unterroutine
int32_t savehex16(int32_t anfang, int32_t ende)
{
  string s;
  int32_t bytezahl;
  int32_t adressex2;
  int32_t sum;
  int32_t zeiger = anfang;

  while (zeiger < ende)
  {
    bytezahl = 0;
    sum = 0;
    s = "";
    while ((PICzelle(ende) == 0xFF) && (anfang < ende)) ende--;
    if (anfang == ende) return(0);
    // ULBA : Extended Linear Address Record
    if (zeiger == anfang)
    {
      if (anfang == 0x000000) fprintf(outdatei, ":020000040000FA\n");  //Flash
      if (anfang == 0x200000) fprintf(outdatei, ":020000040020DA\n");  //ID
      if (anfang == 0x300000) fprintf(outdatei, ":020000040030CA\n");  //Config
      if (anfang == 0xF00000) fprintf(outdatei, ":0200000400F00A\n");  //EEPROM
    }
    adressex2 = zeiger % 0x10000;
    while ((zeiger <= ende) &&
      (((zeiger % 0x10) != 0) || (bytezahl == 0)) &&
      (bytezahl < 0x10) )
    {
      s = s + hex2str(PICzelle(zeiger));
      sum = sum + PICzelle(zeiger);
      bytezahl++;
      zeiger++;
      //if zeiger>ende then break;
    }
    sum = sum + bytezahl + (adressex2 / 256) + (adressex2 % 256);
    sum = sum % 256;
    sum = 256 - sum;
    s = ":" + hex2str(bytezahl) + hex2str(adressex2 / 256) + hex2str(adressex2 % 256) + "00" + s + hex2str(sum % 256);
    if (bytezahl > 0) fprintf(outdatei, "%s\n", s.c_str());
  }
  return(0);
}


// The 32-bit Extended Linear Address Record is used to specify
// bits 16-31 of the Linear Base Address
// (LBA), where bits 0-15 of the LBA are zero. Bits 16-31 of the LBA
// are referred to as the Upper Linear Base Address (ULBA).
void elar_zeile(int32_t adresse)
{
    int32_t k;
    int32_t sum;
  string s;
  // die Adresse im HEX-File ist 2xPIC-Adresse
  s = ":02000004";
  sum = 0x02 + 0x00 + 0x00 + 0x04;
  //high zuerst
  k = (adresse*2 & 0xFF000000) >> 24;
  s = s + hex2str(k);
  sum = sum + k;
  //dann low
  k = (adresse*2 & 0x00FF0000) >> 16;
  s = s + hex2str(k);
  sum = sum + k;
  //und pruefsumme
  //This field contains the check sum on the RECLEN, LOAD OFFSET, RECTYP, and ULBA fields.
  sum = sum % 256;
  sum = 256 - sum;
  s = s + hex2str(sum % 256);
  fprintf(outdatei, "%s\n", s.c_str());
}

// Schreiben von prog.HexOut in ein Hex-File fuer 24-Bit-Kern-PICs -Unterroutine
int32_t savehex30(int32_t anfang, int32_t ende)
{
    string s;
    int32_t bytezahl;
    int32_t adressex2;
    int32_t sum;
    int32_t zeiger = anfang;

  while (zeiger < ende)
  {
    bytezahl = 0;
    sum = 0;
    s = "";
    if (anfang >= ende) return(0);
    if ((zeiger == anfang) || ((zeiger & 0x007FFF) == 0)) elar_zeile(zeiger);
    adressex2 = (zeiger * 2) % 0x10000;
    while ((zeiger <= ende) &&
      (((zeiger % 0x10) != 0) || (bytezahl == 0)) &&
      (bytezahl <  0x10))
    {
      s = s + zweiByte(zeiger, sum);
      bytezahl = bytezahl + 2;
      zeiger++;
      //if (zeiger > ende) break;
    }
    sum = sum + bytezahl + (adressex2 / 256) + (adressex2 % 256);
    sum = sum % 256;
    sum = 256 - sum;
    s = ":" + hex2str(bytezahl) + hex2str(adressex2 / 256) + hex2str(adressex2 % 256) + "00" + s + hex2str(sum % 256);
    if (bytezahl > 0) fprintf(outdatei, "%s\n", s.c_str());
  }
  return(0);
}




// Schreiben von prog.HexOut in ein Hex-File fuer 12-Bit-Kern-PICs
int32_t SaveHexout12(void)
{
  //Sichern Programmbereich
  savehex12(0, prog.max_flash);
  //sichern ID
  savehex12(prog.pic.userid.min, prog.pic.userid.max);
  //sichern OSCCALBACKUP
  savehex12(prog.pic.userid.max+1, prog.pic.userid.max+1);
  //Sichern Config
  savehex12(prog.pic.cfgmem.min, prog.pic.cfgmem.max);
  //Sichern EEprom
  if (prog.pic.eedata.min != -1) savehex12(prog.pic.eedata.min, prog.pic.eedata.min+prog.max_ee);
  return(0);
}


// Schreiben von prog.HexOut in ein Hex-File fuer 14-Bit-Kern-PICs
int32_t SaveHexout14(void)
{
  //Sichern Programmbereich
  savehex14(0, prog.max_flash);
  //sichern ID
  savehex14(0x2000,0x2003);
  //sichern Debug
  savehex14(0x2004,0x2004);
  //Sichern Config
  savehex14(0x2007,0x200A);
  //Sichern EEprom
  savehex14(0x2100,0x2100+prog.max_ee);
  return(0);
}


// Schreiben von prog.HexOut in ein Hex-File fuer 16-Bit-Kern-PICs
// pic18F  PIC18FxxJxx
int32_t SaveHexout16(void)
{
  //Sichern Programmbereich
  //fprintf(stdout, "save Flash from %6x  to %6x Kern %d \n", 0, prog.max_flash, prog.core);
  savehex16(0, prog.max_flash);
  if ((prog.core == CORE_16) || (prog.core == CORE_17))    //nicht fuer 18FxxJxx
  {
    //sichern ID
    savehex16(0x200000, 0x200007);
    //Sichern Config
    savehex16(0x300000, 0x30000D);
    //Sichern EEprom
    savehex16(0xF00000, 0xF00000+prog.max_ee);
  }
  return(0);
}


// Schreiben von prog.HexOut in ein Hex-File fuer 24-Bit-Kern-PICs
int32_t SaveHexout30(void)
{
  //Sichern Programmbereich
  savehex30(0, prog.max_flash);
  //Sichern Config
  switch (prog.core)
  {
    case CORE_30: savehex30(0xF80000, 0xF8000D); break;
          case CORE_33: savehex30(0xF80000, 0xF8000D); break;  //dahinter steht die ID von F80010 bis F80016
  }
  //Sichern EEprom
  //savehex($7FF000, $7FF000+MaxEE);
  if (prog.pic.eedata.max > -1) savehex30(prog.pic.eedata.min, prog.pic.eedata.max);
  return(0);
}


// Schreiben von prog.HexOut in ein Hex-File
int32_t savehexout(void)
{
  //file oeffnen
  outdatei = fopen(prog.OutHexfilename, "w");
  fprintf(stdout, "save HEX-file: %s\n", prog.OutHexfilename);
  if (outdatei != NULL)
  {
    switch (prog.core)
    {
      case CORE_12 : SaveHexout12(); break;
      case CORE_14 : SaveHexout14(); break;
      case CORE_16 : 
      case CORE_17 : 
      case CORE_18 : SaveHexout16(); break;
      case CORE_30 :
      case CORE_33 : SaveHexout30(); break;
    }
    //Endekennzeichen
    char s[12];
    strcpy(s, ":00000001FF");
    fprintf(outdatei, "%s\n", s);
    //file schliessen
    fclose(outdatei);
  } else {
    fprintf(stderr, "## can not create HEX-file ");
    return(-1);
  }
  return(0);
}


//********************************************************************************************************************
// Routinen zum einlesen eines HEX-Files
//********************************************************************************************************************


// Wandeln eines Char-Zeichens in seinen Hex-Zahlenwert
unsigned char st2hex(unsigned char zeichen)
{
  switch (zeichen)
  {
  case  '0': return(0); break;
  case  '1': return(1); break;
  case  '2': return(2); break;
  case  '3': return(3); break;
  case  '4': return(4); break;
  case  '5': return(5); break;
  case  '6': return(6); break;
  case  '7': return(7); break;
  case  '8': return(8); break;
  case  '9': return(9); break;
  case 'A': return(10); break;
  case 'B': return(11); break;
  case 'C': return(12); break;
  case 'D': return(13); break;
  case 'E': return(14); break;
  case 'F': return(15); break;
  case 'a': return(10); break;
  case 'b': return(11); break;
  case 'c': return(12); break;
  case 'd': return(13); break;
  case 'e': return(14); break;
  case 'f': return(15); break;
  default: return(0);
  }
}

// Lesen eines 8-Bit Wertes aus dem HEX-File
uint16_t ReadByte(void)
{
  uint16_t a = st2hex( fgetc(indatei) );
  uint16_t b = st2hex( fgetc(indatei) );
  return((a<<4) + b);
}


// Lesen eines 16-Bit Wertes aus dem HEX-File - big endian
uint16_t ReadWord(void)
{
  return (ReadByte() << 8) + ReadByte();
}


// Lesen eines 16-Bit Wertes aus dem HEX-File - little endian
uint16_t ReadWordLH(void)
{
  return ReadByte() + (ReadByte() << 8);
}



// Lesen eines 32-Bit-Longword
uint32_t Wortlesen24(void)
{
//  if ((PICAdresse % 1) != 0) return(0);
  uint8_t a = ReadByte();
  uint8_t b = ReadByte();
  uint8_t c = ReadByte();
  uint8_t d = ReadByte();
  return(a + (b << 8) + (c << 16) + (d << 24));
}


// Lesen eines HEX-Files fuer 12-Bit-Kern-PICs
void OpenHexFile12(void)
{
  prog.EndFlash = 0;
  prog.EndEE    = -1;
  int32_t ULBA      = 0;
  int32_t SBA       = 0;
  uint16_t ByteAnzahl;
  uint16_t HexTyp;
  int32_t Adresse;
  int32_t Pointer;

  do
  {
    fgetc(indatei);  //erst mal ':' einlesen
    ByteAnzahl  = ReadByte();
    Adresse     = ReadWord();
    HexTyp      = ReadByte();

    switch (HexTyp)
    {
      case 0:
        for (int32_t k=0; k<=(ByteAnzahl / 2)-1; k++)
        {
          Pointer = (ULBA * 0x10000) + (SBA * 0x10) + Adresse;
          Pointer = Pointer / 2;
          Pointer = Pointer + k;
          // Daten nach Hexin schreiben
          // Flash
          if      ((Pointer >= prog.pic.pgmmem.min) && (Pointer <= prog.pic.pgmmem.max))
          {
            prog.HexIn.Flash[Pointer] = ReadWordLH();
            if (Pointer > prog.EndFlash) prog.EndFlash = Pointer;
          }
          else if ((Pointer >= prog.pic.userid.min) && (Pointer <= prog.pic.userid.max)) prog.HexIn.ID[idNr(Pointer)] = ReadWordLH();   // USERID
          else if ((Pointer >= prog.pic.cfgmem.min) && (Pointer <= prog.pic.cfgmem.max)) prog.HexIn.Config[confNr(Pointer)] = ReadWordLH();// Config
          else if ((Pointer >= prog.pic.eedata.min) && (Pointer <= prog.pic.eedata.max))                // EEPROM
          {
             prog.HexIn.ROM[Pointer-prog.pic.eedata.min] = ReadWordLH() & 0x00FF;
            if ((Pointer-prog.pic.eedata.min) > prog.EndEE) prog.EndEE = Pointer- prog.pic.eedata.min;
          }
        }
        break;
      case 2:
        SBA = ReadWord();  //Extended Segment Address Record
        break;
      case 4:
        ULBA = ReadWord();  //Extended Linear Address Record
        break;
    }

    //zur naechsten Zeile gehen
    while ((fgetc(indatei) != 0x0A) && !feof(indatei))
      {}
  } while ((HexTyp != 1) && !feof(indatei));
}



// Lesen eines HEX-Files fuer 14-Bit-Kern-PICs
void OpenHexFile14(void)
{
  prog.EndFlash = 0;
  prog.EndEE    = -1;
  int32_t ULBA      = 0;
  int32_t SBA       = 0;
  uint16_t ByteAnzahl;
  uint16_t HexTyp;
  int32_t Adresse;
  int32_t Pointer;

  do
  {
    fgetc(indatei);  //erst mal ':' einlesen
    ByteAnzahl  = ReadByte();
    Adresse     = ReadWord();
    HexTyp      = ReadByte();

    switch (HexTyp)
    {
    case 0:
            for (int32_t k=0; k<=(ByteAnzahl / 2)-1; k++)
      {
        Pointer = (ULBA * 0x10000) + (SBA * 0x10) + Adresse;  
        Pointer = Pointer / 2;  
        Pointer = Pointer + k;
        if      ((Pointer >= 0x0000) && (Pointer <= 0x1FFF)) 
        {
          prog.HexIn.Flash[Pointer] = ReadWordLH();
          if (Pointer > prog.EndFlash) prog.EndFlash = Pointer;
        }
        else if ((Pointer >= 0x2000) && (Pointer <= 0x2004)) prog.HexIn.ID[idNr(Pointer)] = ReadWordLH();
        else if ((Pointer >= 0x2007) && (Pointer <= 0x200A)) prog.HexIn.Config[confNr(Pointer)] = ReadWordLH();
        else if ((Pointer >= 0x2100) && (Pointer <= 0x21FF))
        {
           prog.HexIn.ROM[Pointer-0x2100] = ReadWordLH() & 0x00FF;
          if ((Pointer-0x2100) > prog.EndEE) prog.EndEE = Pointer- 0x2100;
        }
      }
    break;

    case 2:
      SBA = ReadWord();  //Extended Segment Address Record
    break;

    case 4:
      ULBA = ReadWord();  //Extended Linear Address Record
    break;
    }

    //zur naechsten Zeile gehen
    while ((fgetc(indatei) != 0x0A) && !feof(indatei));
  } while ((HexTyp != 1) && !feof(indatei));
}


// Lesen eines HEX-Files fuer 16-Bit-Kern-PICs
void OpenHexFile16(void)
{
  prog.EndFlash = 0;
  prog.EndEE    = -1;
    int32_t ULBA      = 0;
    int32_t SBA       = 0;
  uint16_t ByteAnzahl;
  uint16_t HexTyp;
    int32_t Adresse;
    int32_t Pointer;

  do
  {
    fgetc(indatei);  //erst mal ':' einlesen
    ByteAnzahl  = ReadByte();
    Adresse     = ReadWord();
    HexTyp      = ReadByte();
    //fprintf(stdout, " ByteAnzahl %6x Adresse %6x HexTyp %6x \n", ByteAnzahl, Adresse, HexTyp);
    switch (HexTyp)
    {
    case 0:
            for (int32_t k=0; k<=ByteAnzahl-1; k++)
      {
        Pointer = (ULBA * 0x10000) + (SBA * 0x10) + Adresse + k;
  
        if      ((Pointer >= 0x000000) && (Pointer <= 0x01FFFF)) 
        {
          prog.HexIn.Flash[Pointer] = ReadByte();
          //f�r PIC18FxxJxx die Config einblenden
          if ((Pointer >= prog.pic.cfgmem.min) && (Pointer <= prog.pic.cfgmem.max))
            prog.HexIn.Config[Pointer-prog.pic.cfgmem.min] = prog.HexIn.Flash[Pointer];
          if (Pointer > prog.EndFlash) prog.EndFlash = Pointer;
        }
        else if ((Pointer >= 0x200000) && (Pointer <= 0x200007)) prog.HexIn.ID[idNr(Pointer)] = ReadByte();
        else if ((Pointer >= 0x300000) && (Pointer <= 0x30000D)) prog.HexIn.Config[Pointer-0x300000] = ReadByte();

        else if ((Pointer >= 0xF00000) && (Pointer <= 0xF003FF)) 
        {
          prog.HexIn.ROM[Pointer-0xF00000] = ReadByte();
          if ((Pointer & 0x0003FF) > prog.EndEE) prog.EndEE = Pointer & 0x0003FF;
        }
      }
    break;

    case 2:
      SBA = ReadWord();  //Extended Segment Address Record
    break;

    case 4:
      ULBA = ReadWord();  //Extended Linear Address Record
    break;
    }

    //zur naechsten Zeile gehen
    while ((fgetc(indatei) != 0x0A) && !feof(indatei));
  } while ((HexTyp != 1) && !feof(indatei));
}


// Lesen eines HEX-Files fuer 24-Bit-Kern-PICs
void OpenHexFile30(void)
{
  prog.EndFlash = 0;
  prog.EndEE    = -1;
    int32_t ULBA      = 0;
    int32_t SBA       = 0;
  uint16_t ByteAnzahl;
  uint16_t HexTyp;
    int32_t Adresse;
    int32_t PICAdresse;
    int32_t HexinAdresse;
    int32_t Pointer;
  uint16_t puffer16;

  do
  {
    fgetc(indatei);  //erst mal ':' einlesen
    ByteAnzahl  = ReadByte();
    Adresse     = ReadWord();
    HexTyp      = ReadByte();

    switch (HexTyp)
    {
    case 0:
      if ((ByteAnzahl % 4) != 0)
      {
        fprintf(stderr,"## hex-file-data corrupt");
        ByteAnzahl= (ByteAnzahl % 4) * 4;
      }
      if (ByteAnzahl > 0)
            for (int32_t k=0; k<=(ByteAnzahl / 2) -1; k++)
      {
        // der Pointer zeigt auf ein uint8_t
        Pointer = (ULBA * 0x10000) + (SBA * 0x10) + Adresse + k*2;
        PICAdresse = Pointer / 2;
        HexinAdresse = PICAdresse;
        // Daten nach Hexin schreiben
        // im hex-file sind flash, eeprom und config als 32-Bit worte abgelegt
        // die hoeherwertigen Bits sind 0
        // ich lese 16-bit-weise ein, damit habe ich auch ungerade Flash-Adressen
        // die ungerade Adresse enthaelt den high-Teil
        puffer16 = ReadWordLH();
        if      ((Pointer >= 0x000000) && (Pointer <= 0x02ABFF)) 
        {
          prog.HexIn.Flash[HexinAdresse] = puffer16;
          if (((Pointer >= 0x000000) && (Pointer <= 0x017FFF)) && (HexinAdresse > prog.EndFlash)) prog.EndFlash = HexinAdresse;
        }
//        else if ((Pointer >= 0x200000) && (Pointer <= 0x200007)) prog.HexIn.ID[idNr(Pointer)] = ReadByte();
        else if ((Pointer >= 0xF80000) && (Pointer <= 0xF8000D)) prog.HexIn.Config[HexinAdresse-0xF80000] = puffer16;
        //  die .ROM-Zellen sind nur 16 Bit gross
        //  EEPROM endet immer bei 0x7FFFFF
        else if ((Pointer >= 0x7FF000) && (Pointer <= 0x7FFFFF)) 
        {
          prog.HexIn.ROM[HexinAdresse-0x7FF000] = puffer16;
          if ((HexinAdresse-0x7FF000) > prog.EndEE) prog.EndEE = HexinAdresse-0x7FF000;
        }
//        else if ((Pointer >= 0x800000) && (Pointer <= 0x8005BF))  PEC[HexinAdresse-0x800000] = puffer16;
      }
    break;

    case 2:
      SBA = ReadWord();  //Extended Segment Address Record
    break;

    case 4:
      ULBA = ReadWord();  //Extended Linear Address Record
    break;
    }

    //zur naechsten Zeile gehen
    while ((fgetc(indatei) != 0x0A) && !feof(indatei));
  } while ((HexTyp != 1) && !feof(indatei));
}



//Einlesen eines HEX-Files nach prog.HexIn
int32_t openhexfile(void)
{
  //file oeffnen
  indatei = fopen(prog.InHexfilename, "r");
  fprintf(stdout, "load HEX-file: %s\n", prog.InHexfilename);
  if (indatei != NULL)
  {
    switch (prog.core)
    {
      case CORE_12 : OpenHexFile12(); break;
      case CORE_14 : OpenHexFile14(); break;
      case CORE_16 : 
      case CORE_17 : 
      case CORE_18 : OpenHexFile16(); break;
      case CORE_30 :
      case CORE_33 : OpenHexFile30(); break;
    }

    //file schliessen
    fclose(indatei);
    if ((prog.max_flash > 0) && (prog.EndFlash > prog.max_flash)) fprintf(stderr, "## code dont fits into FLASH-memory");
    if ((prog.max_ee    > 0) && (prog.EndEE    > prog.max_ee))    fprintf(stderr, "## data dont fits into EEPROM");

    // flash zum Test auflisten
    if (f_i)
    {
            for (int32_t k=0; k<=prog.max_flash; k++)
      {
        if ((k % 0x10) ==0) fprintf(stdout,"\n%6x : ", k);
        fprintf(stdout, "%4x ", prog.HexIn.Flash[k]);
      }  
      fprintf(stdout,"\n");
    }
  }
  else
  {
    fprintf(stderr, "## can not open HEX-file");
    return(-1);
  }
  return(0);
}
