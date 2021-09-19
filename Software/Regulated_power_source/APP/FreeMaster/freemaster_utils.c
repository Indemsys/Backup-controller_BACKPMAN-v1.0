/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2019 NXP
 *
 * License: NXP LA_OPT_NXP_Software_License
 *
 * NXP Confidential. This software is owned or controlled by NXP and may
 * only be used strictly in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that
 * you have read, and that you agree to comply with and are bound by,
 * such license terms.  If you do not agree to be bound by the applicable
 * license terms, then you may not retain, install, activate or otherwise
 * use the software.  This code may only be used in a microprocessor,
 * microcontroller, sensor or digital signal processor ("NXP Product")
 * supplied directly or indirectly from NXP.  See the full NXP Software
 * License Agreement in license/LA_OPT_NXP_Software_License.pdf
 *
 * FreeMASTER Communication Driver - utility code
 */

#include "App.h"
#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_utils.h"

#if !(FMSTR_DISABLE)

/********************************************************
*  optimized memory copy helper macros
********************************************************/

  #if FMSTR_MEMCPY_MAX_SIZE >= 8

/* Copy variable to destination by bytes from an aligned source address */
FMSTR_INLINE void _FMSTR_CopySrcAligned_U64(FMSTR_U8 *dest, FMSTR_U64 *src)
{
  TX_INTERRUPT_SAVE_AREA
  TX_DISABLE;
  union
  {
      FMSTR_U64 n;
      FMSTR_U8 raw[8]; } value;
  FMSTR_U8 *raw = value.raw;
  value.n =*src;      /* read aligned source value with a single operation */
  *dest++=*raw++;    /* copy all bytes to generally unaligned destination */
  *dest++=*raw++;
  *dest++=*raw++;
  *dest++=*raw++;
  *dest++=*raw++;
  *dest++=*raw++;
  *dest++=*raw++;
  *dest++=*raw++;
  TX_RESTORE;
}

/* Copy variable from source by bytes to an aligned destination address */
FMSTR_INLINE void _FMSTR_CopyDstAligned_U64(FMSTR_U64 *dest, FMSTR_U8 *src)
{
  TX_INTERRUPT_SAVE_AREA
  TX_DISABLE;
  union
  {
      FMSTR_U64 n;
      FMSTR_U8 raw[8]; } value;
  FMSTR_U8 *raw = value.raw;
  *raw++=*src++;    /* copy all bytes from generally unaligned source */
  *raw++=*src++;
  *raw++=*src++;
  *raw++=*src++;
  *raw++=*src++;
  *raw++=*src++;
  *raw++=*src++;
  *raw++=*src++;
  *dest = value.n;    /* write aligned destination with a single operation */
  TX_RESTORE;
}

/* Masked copy variable from source by bytes to an aligned destination address */
FMSTR_INLINE void _FMSTR_CopyMaskedDstAligned_U64(FMSTR_U64 *dest, FMSTR_U8 *src, FMSTR_U8 *mask)
{
  FMSTR_U64 v, m, x;
  _FMSTR_CopyDstAligned_U64(&v, src);
  _FMSTR_CopyDstAligned_U64(&m, mask);
  x =*dest;
  x =(x & ~m) | (v & m);
  *dest = x;
}
  #endif

  #if FMSTR_MEMCPY_MAX_SIZE >= 4

/* Copy variable to destination by bytes from an aligned source address */
FMSTR_INLINE void _FMSTR_CopySrcAligned_U32(FMSTR_U8 *dest, FMSTR_U32 *src)
{
  TX_INTERRUPT_SAVE_AREA
  TX_DISABLE;
  union
  {
      FMSTR_U32 n;
      FMSTR_U8 raw[4]; } value;
  FMSTR_U8 *raw = value.raw;
  value.n =*src;      /* read aligned source value with a single operation */
  *dest++=*raw++;    /* copy all bytes to generally unaligned destination */
  *dest++=*raw++;
  *dest++=*raw++;
  *dest++=*raw++;
  TX_RESTORE;
}

/* Copy variable from source by bytes to an aligned destination address */
FMSTR_INLINE void _FMSTR_CopyDstAligned_U32(FMSTR_U32 *dest, FMSTR_U8 *src)
{
  TX_INTERRUPT_SAVE_AREA
  TX_DISABLE;
  union
  {
      FMSTR_U32 n;
      FMSTR_U8 raw[4]; } value;
  FMSTR_U8 *raw = value.raw;
  *raw++=*src++;    /* copy all bytes from generally unaligned source */
  *raw++=*src++;
  *raw++=*src++;
  *raw++=*src++;
  *dest = value.n;    /* write aligned destination with a single operation */
  TX_RESTORE;
}

/* Masked copy variable from source by bytes to an aligned destination address */
FMSTR_INLINE void _FMSTR_CopyMaskedDstAligned_U32(FMSTR_U32 *dest, FMSTR_U8 *src, FMSTR_U8 *mask)
{
  FMSTR_U32 v, m, x;
  _FMSTR_CopyDstAligned_U32(&v, src);
  _FMSTR_CopyDstAligned_U32(&m, mask);
  x =*dest;
  x =(x & ~m) | (v & m);
  *dest = x;
}

  #endif

  #if FMSTR_MEMCPY_MAX_SIZE >= 2

/* Copy variable to destination by bytes from an aligned source address */
FMSTR_INLINE void _FMSTR_CopySrcAligned_U16(FMSTR_U8 *dest, FMSTR_U16 *src)
{
  TX_INTERRUPT_SAVE_AREA
  TX_DISABLE;
  union
  {
      FMSTR_U16 n;
      FMSTR_U8 raw[2]; } value;
  FMSTR_U8 *raw = value.raw;
  value.n =*src;      /* read aligned source value with a single operation */
  *dest++=*raw++;    /* copy all bytes to generally unaligned destination */
  *dest++=*raw++;
  TX_RESTORE;
}

/* Copy variable from source by bytes to an aligned destination address */
FMSTR_INLINE void _FMSTR_CopyDstAligned_U16(FMSTR_U16 *dest, FMSTR_U8 *src)
{
  TX_INTERRUPT_SAVE_AREA
  TX_DISABLE;
  union
  {
      FMSTR_U16 n;
      FMSTR_U8 raw[2]; } value;
  FMSTR_U8 *raw = value.raw;
  *raw++=*src++;    /* copy all bytes from generally unaligned source */
  *raw++=*src++;
  *dest = value.n;    /* write aligned destination with a single operation */
  TX_RESTORE;
}

/* Masked copy variable from source by bytes to an aligned destination address */
FMSTR_INLINE void _FMSTR_CopyMaskedDstAligned_U16(FMSTR_U16 *dest, FMSTR_U8 *src, FMSTR_U8 *mask)
{
  FMSTR_U16 v, m, x;
  _FMSTR_CopyDstAligned_U16(&v, src);
  _FMSTR_CopyDstAligned_U16(&m, mask);
  x =*dest;
  x =(x & ~m) | (v & m);
  *dest = x;
}

  #endif

/* Test if FMSTR_ADDR address is mis-aligned for given number of bits */
  #define TEST_MISALIGNED(addr, bits) ( (((FMSTR_U32)(addr)) & ((1<<(bits))-1)) != 0 )

/* in this helper call, we are already sure that the destination pointer is 64-bit aligned */
static void _FMSTR_MemCpyDstAligned(FMSTR_ADDR dest, FMSTR_ADDR src, FMSTR_SIZE size)
{
  FMSTR_U8 *src8 = (FMSTR_U8 *) src;

#if FMSTR_MEMCPY_MAX_SIZE >= 8
  {
    /* 64-bit aligned part */
    FMSTR_U64 *d64 = (FMSTR_U64 *) dest;

    while (size >= sizeof(FMSTR_U64))
    {
      _FMSTR_CopyDstAligned_U64(d64, src8);
      size -= sizeof(FMSTR_U64);
      src8 += sizeof(FMSTR_U64);
      d64++;
    }

    dest = (FMSTR_ADDR)(d64);
  }
#endif
#if FMSTR_MEMCPY_MAX_SIZE >= 4
  {
    /* remaining word(s) */
    FMSTR_U32 *d32 = (FMSTR_U32 *) dest;

    while (size >= sizeof(FMSTR_U32))
    {
      _FMSTR_CopyDstAligned_U32(d32, src8);
      size -= sizeof(FMSTR_U32);
      src8 += sizeof(FMSTR_U32);
      d32++;
    }

    dest = (FMSTR_ADDR)(d32);
  }
#endif
#if FMSTR_MEMCPY_MAX_SIZE >= 2
  {
    /* remaining halfword(s) */
    FMSTR_U16 *d16 = (FMSTR_U16 *) dest;

    while (size >= sizeof(FMSTR_U16))
    {
      _FMSTR_CopyDstAligned_U16(d16, src8);
      size -= sizeof(FMSTR_U16);
      src8 += sizeof(FMSTR_U16);
      d16++;
    }

    dest = (FMSTR_ADDR)(d16);
  }
#endif
  {
    volatile FMSTR_U8 *d8 = (FMSTR_U8 *) dest;
    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE;
    /* remaining byte(s) */
    while (size >= 1)
    {
      *d8++=*src8++;
      size--;
    }
    TX_RESTORE;
  }

  FMSTR_ASSERT(size == 0);
}

/* in this helper call, we are already sure that the source pointer is 64-bit aligned */
static void _FMSTR_MemCpySrcAligned(FMSTR_ADDR dest, FMSTR_ADDR src, FMSTR_SIZE size)
{
  FMSTR_U8 *dest8 = (FMSTR_U8 *) dest;

#if FMSTR_MEMCPY_MAX_SIZE >= 8
  {
    /* 64-bit aligned part */
    FMSTR_U64 *s64 = (FMSTR_U64 *) src;

    while (size >= sizeof(FMSTR_U64))
    {
      _FMSTR_CopySrcAligned_U64(dest8, s64);
      size -= sizeof(FMSTR_U64);
      dest8 += sizeof(FMSTR_U64);
      s64++;
    }

    src = (FMSTR_ADDR)(s64);
  }
#endif
#if FMSTR_MEMCPY_MAX_SIZE >= 4
  {
    /* remaining word(s) */
    FMSTR_U32 *s32 = (FMSTR_U32 *) src;

    while (size >= sizeof(FMSTR_U32))
    {
      _FMSTR_CopySrcAligned_U32(dest8, s32);
      size -= sizeof(FMSTR_U32);
      dest8 += sizeof(FMSTR_U32);
      s32++;
    }

    src = (FMSTR_ADDR)(s32);
  }
#endif
#if FMSTR_MEMCPY_MAX_SIZE >= 2
  {
    /* remaining halfword(s) */
    FMSTR_U16 *s16 = (FMSTR_U16 *) src;

    while (size >= sizeof(FMSTR_U16))
    {
      _FMSTR_CopySrcAligned_U16(dest8, s16);
      size -= sizeof(FMSTR_U16);
      dest8 += sizeof(FMSTR_U16);
      s16++;
    }

    src = (FMSTR_ADDR)(s16);
  }
#endif
  {
    volatile FMSTR_U8 *s8 = (FMSTR_U8 *) src;
    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE;
    /* remaining byte(s) */
    while (size >= 1)
    {
      *dest8++=*s8++;
      size--;
    }
    TX_RESTORE;
  }

  FMSTR_ASSERT(size == 0);
}

/* in this helper call, we are already sure that the required pointer is aligned */
static void _FMSTR_MemCpyMaskedDstAligned(FMSTR_ADDR dest, FMSTR_ADDR src, FMSTR_ADDR mask, FMSTR_SIZE size)
{
  FMSTR_U8 *src8 = (FMSTR_U8 *) src;
  FMSTR_U8 *mask8 = (FMSTR_U8 *) mask;

#if FMSTR_MEMCPY_MAX_SIZE >= 8
  {
    /* 64-bit aligned part */
    FMSTR_U64 *d64 = (FMSTR_U64 *) dest;

    while (size >= sizeof(FMSTR_U64))
    {
      _FMSTR_CopyMaskedDstAligned_U64(d64, src8, mask8);
      size -= sizeof(FMSTR_U64);
      src8 += sizeof(FMSTR_U64);
      mask8 += sizeof(FMSTR_U64);
      d64++;
    }

    dest = (FMSTR_ADDR)(d64);
  }
#endif
#if FMSTR_MEMCPY_MAX_SIZE >= 4
  {
    /* remaining word(s) */
    FMSTR_U32 *d32 = (FMSTR_U32 *) dest;

    while (size >= sizeof(FMSTR_U32))
    {
      _FMSTR_CopyMaskedDstAligned_U32(d32, src8, mask8);
      size -= sizeof(FMSTR_U32);
      src8 += sizeof(FMSTR_U32);
      mask8 += sizeof(FMSTR_U32);
      d32++;
    }

    dest = (FMSTR_ADDR)(d32);
  }
#endif
#if FMSTR_MEMCPY_MAX_SIZE >= 2
  {
    /* remaining halfword(s) */
    FMSTR_U16 *d16 = (FMSTR_U16 *) dest;

    while (size >= sizeof(FMSTR_U16))
    {
      _FMSTR_CopyMaskedDstAligned_U16(d16, src8, mask8);
      size -= sizeof(FMSTR_U16);
      src8 += sizeof(FMSTR_U16);
      mask8 += sizeof(FMSTR_U16);
      d16++;
    }

    dest = (FMSTR_ADDR)(d16);
  }
#endif
  {
    volatile FMSTR_U8 *d8 = (FMSTR_U8 *) dest;
    FMSTR_U8 m, s;
    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE;
    /* remaining byte(s) */
    while (size >= 1)
    {
      m =*mask8++;
      s = (FMSTR_U8)(*src8++ & m);
      s |= (FMSTR_U8)(*d8 & (~m));
      *d8++= s;
      size--;
    }
    TX_RESTORE;

  }

  FMSTR_ASSERT(size == 0);
}

/**************************************************************************//*!
*
* @brief  Generic memory copy routine without alignment and transfer size requirements
*
* @param  destAddr - destination memory address
* @param  srcBuff  - pointer to source memory in communication buffer
* @param  size     - buffer size (always in bytes)
*
******************************************************************************/

void _FMSTR_MemCpy(void *dest, const void *src, FMSTR_SIZE size)
{
  FMSTR_MemCpyTo((FMSTR_ADDR)(dest), (FMSTR_ADDR)(src), size);
}

/**************************************************************************//*!
*
* @brief  Copy data. Reading from source memory is as aligned as it can be.
*
* @param  destAddr - destination memory address
* @param  srcAddr  - source memory address
* @param  size     - buffer size in bytes
*
******************************************************************************/

FMSTR_WEAK void FMSTR_MemCpyFrom(FMSTR_ADDR destAddr, FMSTR_ADDR srcAddr, FMSTR_SIZE size)
{
  FMSTR_U8 *dest8 = (FMSTR_U8 *) destAddr;

#if FMSTR_MEMCPY_MAX_SIZE >= 2
  /* misaligned odd byte */
  if (TEST_MISALIGNED(srcAddr, 1) && size >= sizeof(FMSTR_U8))
  {
    FMSTR_U8 *s8 = (FMSTR_U8 *) srcAddr;
    *dest8++=*s8++;
    size -= sizeof(FMSTR_U8);
    srcAddr = (FMSTR_ADDR)(s8);
  }
#if FMSTR_MEMCPY_MAX_SIZE >= 4
  /* misaligned odd halfword */
  if (TEST_MISALIGNED(srcAddr, 2) && size >= sizeof(FMSTR_U16))
  {
    FMSTR_U16 *s16 = (FMSTR_U16 *) srcAddr;
    _FMSTR_CopySrcAligned_U16(dest8, s16);
    size -= sizeof(FMSTR_U16);
    dest8 += sizeof(FMSTR_U16);
    s16++;
    srcAddr = (FMSTR_ADDR)(s16);
  }
#if FMSTR_MEMCPY_MAX_SIZE >= 8
  /* misaligned odd word */
  if (TEST_MISALIGNED(srcAddr, 3) && size >= sizeof(FMSTR_U32))
  {
    FMSTR_U32 *s32 = (FMSTR_U32 *) srcAddr;
    _FMSTR_CopySrcAligned_U32(dest8, s32);
    size -= sizeof(FMSTR_U32);
    dest8 += sizeof(FMSTR_U32);
    s32++;
    srcAddr = (FMSTR_ADDR)(s32);
  }
#endif
#endif
#endif

  /* the rest is already aligned */
  _FMSTR_MemCpySrcAligned((FMSTR_ADDR)(dest8), srcAddr, size);
}

/**************************************************************************//*!
*
* @brief  Copy data. Writing to destination memory is as aligned as it can be.
*
* @param  destAddr - destination memory address
* @param  srcAddr  - source memory address
* @param  size     - buffer size in bytes
*
******************************************************************************/

FMSTR_WEAK void FMSTR_MemCpyTo(FMSTR_ADDR destAddr, FMSTR_ADDR srcAddr, FMSTR_SIZE size)
{
  FMSTR_U8 *src8 = (FMSTR_U8 *) srcAddr;

#if FMSTR_MEMCPY_MAX_SIZE >= 2
  /* misaligned odd byte */
  if (TEST_MISALIGNED(destAddr, 1) && size >= sizeof(FMSTR_U8))
  {
    FMSTR_U8 *d8 = (FMSTR_U8 *) destAddr;
    *d8++=*src8++;
    size -= sizeof(FMSTR_U8);
    destAddr = (FMSTR_ADDR)(d8);
  }
#if FMSTR_MEMCPY_MAX_SIZE >= 4
  /* misaligned odd halfword */
  if (TEST_MISALIGNED(destAddr, 2) && size >= sizeof(FMSTR_U16))
  {
    FMSTR_U16 *d16 = (FMSTR_U16 *) destAddr;
    _FMSTR_CopyDstAligned_U16(d16, src8);
    size -= sizeof(FMSTR_U16);
    src8 += sizeof(FMSTR_U16);
    d16++;
    destAddr = (FMSTR_ADDR)(d16);
  }
#if FMSTR_MEMCPY_MAX_SIZE >= 8
  /* misaligned odd word */
  if (TEST_MISALIGNED(destAddr, 3) && size >= sizeof(FMSTR_U32))
  {
    FMSTR_U32 *d32 = (FMSTR_U32 *) destAddr;
    _FMSTR_CopyDstAligned_U32(d32, src8);
    size -= sizeof(FMSTR_U32);
    src8 += sizeof(FMSTR_U32);
    d32++;
    destAddr = (FMSTR_ADDR)(d32);
  }
#endif
#endif
#endif

  /* the rest is already aligned */
  _FMSTR_MemCpyDstAligned(destAddr, (FMSTR_ADDR)(src8), size);
}

/**************************************************************************//*!
*
* @brief  Copy data with mask. Write to destination memory is as aligned as it can be.
*
* @param  destAddr - destination memory address
* @param  srcAddr  - source memory address
* @param  maskAddr - source mask address
* @param  size     - buiffer size in bytes
*
******************************************************************************/
FMSTR_WEAK void FMSTR_MemCpyToMasked(FMSTR_ADDR destAddr, FMSTR_ADDR srcAddr, FMSTR_ADDR maskAddr, FMSTR_SIZE size)
{
  FMSTR_U8 *src8 = (FMSTR_U8 *) srcAddr;
  FMSTR_U8 *mask8 = (FMSTR_U8 *) maskAddr;

#if FMSTR_MEMCPY_MAX_SIZE >= 2
  /* misaligned odd byte */
  if (TEST_MISALIGNED(destAddr, 1) && size >= sizeof(FMSTR_U8))
  {
    FMSTR_U8 *d8 = (FMSTR_U8 *) destAddr;
    FMSTR_U8 m, s;

    m =*mask8++;
    s =*src8++ & m;
    s |=*d8 & (~m);
    *d8++= s;
    size -= sizeof(FMSTR_U8);
    destAddr = (FMSTR_ADDR)(d8);
  }
#endif
#if FMSTR_MEMCPY_MAX_SIZE >= 4
  /* misaligned odd halfword */
  if (TEST_MISALIGNED(destAddr, 2) && size >= sizeof(FMSTR_U16))
  {
    FMSTR_U16 *d16 = (FMSTR_U16 *) destAddr;
    _FMSTR_CopyMaskedDstAligned_U16(d16, src8, mask8);
    size -= sizeof(FMSTR_U16);
    src8 += sizeof(FMSTR_U16);
    mask8 += sizeof(FMSTR_U16);
    d16++;
    destAddr = (FMSTR_ADDR)(d16);
  }
#endif
#if FMSTR_MEMCPY_MAX_SIZE >= 8
  /* misaligned odd word */
  if (TEST_MISALIGNED(destAddr, 3) && size >= sizeof(FMSTR_U32))
  {
    FMSTR_U32 *d32 = (FMSTR_U32 *) destAddr;
    _FMSTR_CopyMaskedDstAligned_U32(d32, src8, mask8);
    size -= sizeof(FMSTR_U32);
    src8 += sizeof(FMSTR_U32);
    mask8 += sizeof(FMSTR_U32);
    d32++;
    destAddr = (FMSTR_ADDR)(d32);
  }
#endif

  /* the rest is already aligned */
  _FMSTR_MemCpyMaskedDstAligned(destAddr, (FMSTR_ADDR)(src8), (FMSTR_ADDR)(mask8), size);
}

/**************************************************************************//*!
*
* @brief  Write to the communication buffer memory
*
* @param  destBuff - pointer to destination memory in communication buffer
* @param  srcAddr  - source memory address
* @param  size     - buffer size in bytes
*
* @return This function returns a pointer to next byte in comm. buffer
*
******************************************************************************/

FMSTR_WEAK FMSTR_BPTR FMSTR_CopyToBuffer(FMSTR_BPTR destBuff, FMSTR_ADDR srcAddr, FMSTR_SIZE size)
{
  FMSTR_MemCpyFrom((FMSTR_ADDR)(destBuff), srcAddr, size);
  return destBuff+size;
}

/**************************************************************************//*!
*
* @brief  Read from communication buffer memory
*
* @param  destAddr - destination memory address
* @param  srcBuff  - pointer to source memory in communication buffer
* @param  size     - buffer size in bytes
*
* @return This function returns a pointer to next byte in comm. buffer
*
******************************************************************************/

FMSTR_WEAK FMSTR_BPTR FMSTR_CopyFromBuffer(FMSTR_ADDR destAddr, FMSTR_BPTR srcBuff, FMSTR_SIZE size)
{
  FMSTR_MemCpyTo(destAddr, (FMSTR_ADDR)(srcBuff), size);
  return srcBuff+size;
}

/**************************************************************************//*!
*
* @brief  Read from communication buffer memory and copy bytes with masking
*
* @param  destAddr - destination memory address
* @param  srcBuff  - pointer to source memory and mask in communication buffer
* @param  size     - buffer size in bytes
*
* @return This function returns a pointer to next byte in comm. buffer
*
******************************************************************************/

FMSTR_WEAK void FMSTR_CopyFromBufferWithMask(FMSTR_ADDR destAddr, FMSTR_BPTR srcBuff, FMSTR_SIZE size)
{
  FMSTR_MemCpyToMasked(destAddr, (FMSTR_ADDR)(srcBuff), (FMSTR_ADDR)(srcBuff+size), size);
}

/**************************************************************************//*!
*
* @brief  Store address in LEB format to communication buffer.
*
******************************************************************************/

FMSTR_WEAK FMSTR_BPTR FMSTR_AddressToBuffer(FMSTR_BPTR dest, FMSTR_ADDR addr)
{
  return FMSTR_UlebEncode(dest,&addr, sizeof(addr));
}

/**************************************************************************//*!
*
* @brief  Fetch address in LEB format from communication buffer
*
******************************************************************************/

FMSTR_WEAK FMSTR_BPTR FMSTR_AddressFromBuffer(FMSTR_ADDR *paddr, FMSTR_BPTR src)
{
  return FMSTR_UlebDecode(src, paddr, sizeof(*paddr));
}

/**************************************************************************//*!
*
* @brief  Store size in LEB format to communication buffer.
*
******************************************************************************/

FMSTR_WEAK FMSTR_BPTR FMSTR_SizeToBuffer(FMSTR_BPTR dest, FMSTR_SIZE size)
{
  return FMSTR_UlebEncode(dest,&size, sizeof(size));
}

/**************************************************************************//*!
*
* @brief  Fetch size in LEB format from communication buffer
*
******************************************************************************/

FMSTR_WEAK FMSTR_BPTR FMSTR_SizeFromBuffer(FMSTR_SIZE *psize, FMSTR_BPTR src)
{
  return FMSTR_UlebDecode(src, psize, sizeof(*psize));
}

/**************************************************************************//*!
*
* @brief  Fetch index in signed LEB format from communication buffer
*
******************************************************************************/

FMSTR_WEAK FMSTR_BPTR FMSTR_IndexFromBuffer(FMSTR_INDEX *pindex, FMSTR_BPTR src)
{
  return FMSTR_SlebDecode(src, pindex, sizeof(*pindex));
}

/**************************************************************************//*!
*
* @brief  Store generic U32 number to communication buffer as ULEB
*
******************************************************************************/

FMSTR_BPTR FMSTR_ULebToBuffer(FMSTR_BPTR dest, FMSTR_U32 num)
{
  return FMSTR_UlebEncode(dest,&num, sizeof(num));
}

/**************************************************************************//*!
*
* @brief  Fetch generic U32 value as ULEB from communication buffer
*
******************************************************************************/

FMSTR_BPTR FMSTR_ULebFromBuffer(FMSTR_U32 *pnum, FMSTR_BPTR src)
{
  return FMSTR_UlebDecode(src, pnum, sizeof(*pnum));
}

/**************************************************************************//*!
*
* @brief  Return number of bytes that given address needs to add in order to
*         get properly aligned.
*
******************************************************************************/

FMSTR_WEAK FMSTR_SIZE FMSTR_GetAlignmentCorrection(FMSTR_ADDR addr, FMSTR_SIZE size)
{
  FMSTR_U32 addrn = (FMSTR_U32)addr;
  FMSTR_U32 aligned = addrn;

  FMSTR_ASSERT(size == 0 || size == 1 || size == 2 || size == 4 || size == 8);

  if (size > 0)
  {
    aligned += size-1;
    aligned &= ~(size-1);
  }

  return (FMSTR_SIZE)(aligned - addrn);
}

/**************************************************************************//*!
*
* @brief Decode LEB number to destination variable
*
* @param in Pointer to input data
* @param result Pointer to destination variable
* @param size Size of the destination variable
* @param sleb True when decoding SLEB format
*
******************************************************************************/

static FMSTR_BPTR FMSTR_LebDecode(FMSTR_BPTR in, void *result, FMSTR_SIZE size, FMSTR_BOOL sleb)
{
  FMSTR_BCHR b;
  FMSTR_U8 v;
  FMSTR_U8 *dest;
  FMSTR_INDEX dadd;
  FMSTR_INDEX shift = 0;

  /* Initialize result to 0 value */
  FMSTR_MemSet(result, 0, size);

#if FMSTR_PLATFORM_BIG_ENDIAN
  dest =((FMSTR_U8 *)result)+ size - 1;
  dadd = -1;
#else
  dest = (FMSTR_U8 *)result;
  dadd = 1;
#endif

  do
  {
    b =*in++;
    v = (FMSTR_U8)(b & 0x7f);

    if (size > 0)
    {
      *dest |= (FMSTR_U8)((v << shift));
      shift += 7;

      if (shift >= 8)
      {
        shift -= 8;
        dest += dadd;
        size--;

        if (size > 0 && shift > 0) *dest |= (FMSTR_U8)(v >>(7-shift));
      }
    }
  } while (b & 0x80);

  /* negative number? */
  if (sleb && (b & 0x40))
  {
    if (size-- > 0)
    {
      *dest |= (FMSTR_U8)(0xff << shift);
      dest += dadd;
    }

    while (size-- > 0)
    {
      *dest = 0xff;
      dest += dadd;
    }
  }

  return in;
}

FMSTR_BPTR FMSTR_UlebDecode(FMSTR_BPTR in, void *result, FMSTR_SIZE size)
{
  return FMSTR_LebDecode(in, result, size, FMSTR_FALSE);
}

FMSTR_BPTR FMSTR_SlebDecode(FMSTR_BPTR in, void *result, FMSTR_SIZE size)
{
  return FMSTR_LebDecode(in, result, size, FMSTR_TRUE);
}

/**************************************************************************//*!
*
* @brief Encode unsigned variable to ULEB record
*
******************************************************************************/

FMSTR_BPTR FMSTR_UlebEncode(FMSTR_BPTR out, void *source, FMSTR_SIZE size)
{
  FMSTR_BCHR b;
  FMSTR_U8 v;
  FMSTR_U8 *src;
  FMSTR_INDEX sadd;
  FMSTR_INDEX shift = 0;
  FMSTR_SIZE zeroes = 0;

#if FMSTR_PLATFORM_BIG_ENDIAN
  src = (FMSTR_U8 *)source;
  while (zeroes < size && !*src++) zeroes++;
  src =((FMSTR_U8 *)source)+ size - 1;
  sadd = -1;
#else
  src =((FMSTR_U8 *)source)+ size - 1;
  while (zeroes < size && !*src--) zeroes++;
  src = (FMSTR_U8 *)source;
  sadd = 1;
#endif

  /* now: 'zeroes' is number of useless most-significant zero bytes
   *      'src' points to least-significant byte and 'sadd' is a direction */
  if (zeroes < size)
  {
    /* we will not encode the zero bytes */
    size -= zeroes;

    /* start with zero (will be or-ing to it) */
    *out = 0;

    while (size--)
    {
      v =*src;
      src += sadd;

      b = (FMSTR_BCHR)((v << shift) & 0x7F);
      *out |= b;

      // shift is number of bits remaining in v
      v = (FMSTR_U8)(v >>(7-shift));

      // other bits to the next out byte
      if (size || v)
      {
        *out++|= 0x80;
        *out = (FMSTR_BCHR)(v & 0x7f);
      }

      shift++;

      if (shift >= 8)
      {
        v >>= 7;

        if (size || v)
        {
          *out++|= 0x80;
          *out = (FMSTR_BCHR)(v & 0x7f);
        }

        shift = 1;
      }
    }

    out++;
  }
  else
  {
    /* variable is equal to 0, this encodes as a single 0 byte */
    *out++= 0;
  }

  return out;
}

/**************************************************************************//*!
*
* @brief  Skip the LEB field in buffer (it doesn't matter if signed or unsigned)
*
* @param  dest - Pointer to LEB field
*
* @return  pointer to buffer just behind LEB field
*
******************************************************************************/

FMSTR_BPTR FMSTR_SkipInBufferLeb(FMSTR_BPTR dest)
{
  FMSTR_BCHR b;
  do
  {
    b =*(dest++);
  } while (b & 0x80);

  return dest;
}

/**************************************************************************//*!
*
* @brief Get string from incomming buffer
*
******************************************************************************/

FMSTR_BPTR FMSTR_StringFromBuffer(FMSTR_BPTR in, FMSTR_CHAR *pStr, FMSTR_SIZE maxSize)
{
  FMSTR_BCHR b;

  do
  {
    in = FMSTR_ValueFromBuffer8(&b, in);

    if (maxSize > 0)
    {
      maxSize--;
      *pStr++= (FMSTR_CHAR)(maxSize ? b : 0);
    }
  } while (b);

  return in;
}

/**************************************************************************//*!
*
* @brief Copy string from memory to outcomming buffer
*
******************************************************************************/

FMSTR_BPTR FMSTR_StringCopyToBuffer(FMSTR_BPTR out, const FMSTR_CHAR *pStr)
{
  while (*pStr)
  {
    out = FMSTR_ValueToBuffer8(out, (FMSTR_U8)*(pStr++));
  }

  out = FMSTR_ValueToBuffer8(out, (FMSTR_U8)0);
  return out;
}

/**************************************************************************//*!
*
* @brief Initialize CRC16 calculation
*
******************************************************************************/

void FMSTR_Crc16Init(FMSTR_U16 *crc)
{
  *crc = FMSTR_CRC16_CCITT_SEED;
}

/**************************************************************************//*!
*
* @brief Add new byte to CRC16 calculation
*
******************************************************************************/

void FMSTR_Crc16AddByte(FMSTR_U16 *crc, FMSTR_U8 data)
{
  FMSTR_INDEX x;

  *crc ^= data << 8;                   /* XOR hi-byte of CRC w/dat    */
  for (x = 8; x; --x)                  /* Then, for 8 bit shifts...   */
  {
    if (*crc & 0x8000)               /* Test hi order bit of CRC    */
      *crc =*crc << 1 ^ 0x1021;   /* if set, shift & XOR w/$1021 */
    else *crc <<= 1;                  /* Else, just shift left once. */
  }
}

/**************************************************************************//*!
*
* @brief Initialize CRC8 calculation
*
******************************************************************************/

void FMSTR_Crc8Init(FMSTR_U8 *crc)
{
  *crc = FMSTR_CRC8_CCITT_SEED;
}

/**************************************************************************//*!
*
* @brief Add new byte to CRC8 calculation
*
******************************************************************************/

void FMSTR_Crc8AddByte(FMSTR_U8 *crc, FMSTR_U8 data)
{
  FMSTR_INDEX x;

  *crc ^= data;                        /* XOR hi-byte of CRC w/dat    */
  for (x = 8; x; --x)                  /* Then, for 8 bit shifts...   */
  {
    if (*crc & 0x80)                 /* Test hi order bit of CRC    */
      *crc = (FMSTR_U8)((*crc << 1)^ 0x07);   /* if set, shift & XOR w/$07 */
    else *crc <<= 1;                  /* Else, just shift left once. */
  }
}

/**************************************************************************//*!
*
* @brief Get array of random numbers
*
******************************************************************************/

  #if FMSTR_CFG_F1_RESTRICTED_ACCESS

FMSTR_BPTR FMSTR_RandomNumbersToBuffer(FMSTR_U8 *out, FMSTR_SIZE length)
{
  FMSTR_INDEX i;
  FMSTR_SIZE sz;
  FMSTR_U32 r;
  FMSTR_U8 *dest = out;
  FMSTR_U8 *ret = out;
  FMSTR_U8 div;

  for (i=0; i < length; i+=4)
  {
    /* TODO: this generator uses stdlib rand implemntation by default, which is weak.
     * replace this by defining your own FMSTR_Rand() macro and replace it by
     * a true random number engine. */

    /* Achieve somewhat better entropy by skipping random number of sequenced numbers */
    div = (FMSTR_U8)FMSTR_Rand();
    while (div-- > 0) r = FMSTR_Rand();

    sz = length - i;
    if (sz > 4) sz = 4;

    ret = FMSTR_CopyToBuffer(&dest[i], (FMSTR_ADDR)&r, sz);
  }

  return ret;
}

  #endif /* FMSTR_CFG_F1_RESTRICTED_ACCESS */

/**************************************************************************//*!
*
* @brief Get array of random numbers
*
******************************************************************************/

  #if FMSTR_CFG_F1_RESTRICTED_ACCESS

void FMSTR_Randomize(FMSTR_U32 entropy)
{
  /* skip next few numbers in rand sequence to achieve a better behavior */
  entropy &= 15;

  do
  {
    FMSTR_Rand();
  } while (entropy-- > 0);
}

  #endif /* FMSTR_CFG_F1_RESTRICTED_ACCESS */

/**************************************************************************//*!
*
* @brief The function prepares ring buffer
*
******************************************************************************/

void _FMSTR_RingBuffCreate(FMSTR_RING_BUFFER *ringBuff, FMSTR_BPTR buffer, FMSTR_U32 size)
{
  FMSTR_ASSERT(ringBuff != NULL);
  FMSTR_ASSERT(buffer != NULL);

  FMSTR_MemSet(ringBuff, 0, sizeof(FMSTR_RING_BUFFER));

  ringBuff->buffer = buffer;
  ringBuff->size = size;
  ringBuff->rp = buffer;
  ringBuff->wp = buffer;
}

/**************************************************************************//*!
*
* @brief The function adds character into ring buffer
*
******************************************************************************/

void _FMSTR_RingBuffPut(FMSTR_RING_BUFFER *ringBuff, FMSTR_BCHR nRxChar)
{
  FMSTR_BPTR wpnext;

  FMSTR_ASSERT(ringBuff != NULL);

  /* future value of write pointer */
  wpnext = ringBuff->wp + 1;

  if (wpnext >= (ringBuff->buffer + ringBuff->size)) wpnext = ringBuff->buffer;

  /* any space in queue? */
  if (wpnext != ringBuff->rp)
  {
    *ringBuff->wp = (FMSTR_U8) nRxChar;
    ringBuff->wp = wpnext;
  }
}

/**************************************************************************//*!
*
* @brief The function gets character from ring buffer
*
******************************************************************************/

FMSTR_BCHR _FMSTR_RingBuffGet(FMSTR_RING_BUFFER *ringBuff)
{
  FMSTR_BCHR nChar = 0U;

  FMSTR_ASSERT(ringBuff != NULL);

  /* get all queued characters */
  if (ringBuff->rp != ringBuff->wp)
  {
    FMSTR_BPTR rpnext = ringBuff->rp;
    nChar =*rpnext++;

    if (rpnext >= (ringBuff->buffer + ringBuff->size)) rpnext = ringBuff->buffer;

    ringBuff->rp = rpnext;
  }

  return nChar;
}

/**************************************************************************//*!
*
* @brief The function returns true, when is space in ring buffer
*
******************************************************************************/

FMSTR_BOOL _FMSTR_RingBuffIsSpace(FMSTR_RING_BUFFER *ringBuff)
{
  FMSTR_BPTR wpnext;

  FMSTR_ASSERT(ringBuff != NULL);

  wpnext = ringBuff->wp + 1;

  /* Is any space in buffer? */
  if (wpnext != ringBuff->rp) return FMSTR_TRUE;

  return FMSTR_FALSE;
}

/**************************************************************************//*!
*
* @brief The function returns true, when some data in ring buffer
*
******************************************************************************/

FMSTR_BOOL _FMSTR_RingBuffHasData(FMSTR_RING_BUFFER *ringBuff)
{
  FMSTR_ASSERT(ringBuff != NULL);

  /* Is any data available to get from buffer? */
  if (ringBuff->rp != ringBuff->wp) return FMSTR_TRUE;

  return FMSTR_FALSE;
}

/**************************************************************************//*!
*
* @brief Compare helper
*
******************************************************************************/

FMSTR_INLINE FMSTR_INDEX _FMSTR_Compare(FMSTR_U8 c1, FMSTR_U8 c2)
{
  if (c1 < c2) return -1;
  if (c1 > c2) return +1;
  return 0;
}

/**************************************************************************//*!
*
* @brief Standard strcmp library function
*
******************************************************************************/

FMSTR_INDEX _FMSTR_StrCmp(const FMSTR_CHAR *str1, const FMSTR_CHAR *str2)
{
  FMSTR_INDEX cmp = 0;
  FMSTR_CHAR c1, c2;

  if (str1 == str2) return 0;

  FMSTR_ASSERT_RETURN(str1 != NULL, 1);
  FMSTR_ASSERT_RETURN(str2 != NULL, -1);

  do
  {
    c1 =*str1++;
    c2 =*str2++;
    cmp = _FMSTR_Compare((FMSTR_U8)c1, (FMSTR_U8)c2);
    if (cmp) return cmp;
  }while (c1 && c2);

  return 0;
}

FMSTR_INDEX _FMSTR_MemCmp(const void *b1, const void *b2, FMSTR_SIZE size)
{
  FMSTR_INDEX cmp = 0;
  FMSTR_U8 *p1 = (FMSTR_U8 *)b1;
  FMSTR_U8 *p2 = (FMSTR_U8 *)b2;
  FMSTR_U8 c1, c2;

  if (p1 == p2) return 0;

  FMSTR_ASSERT_RETURN(p1 != NULL, 1);
  FMSTR_ASSERT_RETURN(p2 != NULL, -1);

  while (size-- > 0)
  {
    c1 =*p1++;
    c2 =*p2++;
    cmp = _FMSTR_Compare(c1, c2);
    if (cmp) return cmp;
  }

  return 0;
}

FMSTR_SIZE _FMSTR_StrLen(const FMSTR_CHAR *str)
{
  const FMSTR_CHAR *s = str;
  while (*s) s++;
  return (FMSTR_SIZE)(s - str);
}

void _FMSTR_MemSet(void *dest, FMSTR_U8 fill, FMSTR_SIZE size)
{
  FMSTR_U8 *d = dest;
  while (size-- > 0) *d++= fill;
}

/* Random number generation not yet implemented, use stdlib function
FMSTR_U32 _FMSTR_Rand(void)
{
    return 0;
}
*/

#endif /* !FMSTR_DISABLE */
