#include "App.h"


int32_t EEESIZE_MAP[16] = {-1,-1,4096,2048,1024,512,256,128,64,32,-1,-1,-1,-1,-1,0};
int32_t DEPART_MAP[16][2] ={{64,0},{-1,-1},{-1,-1},{32,32},{0,64},{-1,-1},{-1,-1},{-1,-1},{0,64},{-1,-1},{16,48},{32,32},{64,0},{-1,-1},{-1,-1},{64,0},};




static uint8_t _Exec_cmd(void);
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint8_t FLASH_Get_state(void)
{
  // Описание регистра FSTAT
  //  BIT(7) - CCIF     (w1c) | Command Complete Interrupt Flag. 1 FTFE command or EEPROM file system operation has completed
  //  BIT(6) - RDCOLERR (w1c) | FTFE Read Collision Error Flag. 1 Collision error detected
  //  BIT(5) - ACCERR   (w1c) | Flash Access Error Flag. 1 Access error detected
  //  BIT(4) - FPVIOL   (w1c) | Flash Protection Violation Flag. 1 Protection violation detected
  //  BIT(0) - MGSTAT0  (ro ) | Memory Controller Command Completion Status Flag.
  return FTFE->FSTAT;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
inline static void _Wait_ready(void)
{
  // Ожидаем пока бит CCIF не установится в 1
  while (!(FTFE->FSTAT & F_CCIF))
  {
    __no_operation();
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
inline static void _Wait_EE_ready(void)
{
  while (!(FTFE->FCNFG & BIT(0)))
  {
    __no_operation();
  }
}

/*-----------------------------------------------------------------------------------------------------
  Получить размер эмулируемой EEPROM по коду считанному из IFR

  \param code

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static int32_t _Get_ee_size(uint8_t code)
{
  return EEESIZE_MAP[code & 0x0F];
}

/*-----------------------------------------------------------------------------------------------------
  Получить размер доступной Data Flash по коду считанному из IFR

  \param code

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
static int32_t _Get_Data_flash_size(uint8_t code)
{
  return DEPART_MAP[code & 0x0F][0];
}


/*-----------------------------------------------------------------------------------------------------
   Получить размер области отведенной во Data Flash под данные EEProm по коду считанному из IFR

  \param code

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
static int32_t _Get_EEpromBackUp_size(uint8_t code)
{
  return DEPART_MAP[code & 0x0F][1];
}

/*-----------------------------------------------------------------------------------------------------
  Ожидание выполнение команды Flash

  \param

  \return __ramfunc void
-----------------------------------------------------------------------------------------------------*/
__ramfunc static uint8_t  _Exec_cmd(void)
{
  uint8_t stat;
  __disable_interrupt();
  FTFE->FSTAT = F_RDCOLERR + F_ACCERR + F_FPVIOL;   // Сбрасываем ошибки.
  FTFE->FSTAT = F_CCIF;                             // Сбрасываем бит CCIF. Нельзя эту операцию объединять с предыдущей операцией сброса ошибок

  // Ожидаем пока бит CCIF не установится в 1
  while (!(FTFE->FSTAT & F_CCIF))
  {
    __no_operation();
  }


  stat = FTFE->FSTAT & (F_RDCOLERR + F_ACCERR + F_FPVIOL);
  __enable_interrupt();
  return stat;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  Flash_Init(void)
{

  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param addr
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_SectorErase(uint32_t addr)
{

  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param buf
  \param sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_ByteProgram(uint32_t addr, unsigned char *buf, uint32_t sz)
{

  return 0;
}



/*-----------------------------------------------------------------------------------------------------
  Чтение содержимого областей IFR program и data Flash и Version ID

  \param addr       - адрес читаемых данных
  \param rsrs_code  - код ресурса
  \param buf        - указатель на буфер - приемник данных
-----------------------------------------------------------------------------------------------------*/
uint32_t FLASH_read_resource(uint32_t addr, uint8_t rsrs_code, uint32_t sz, uint8_t *buf)
{
  uint8_t stat;

  while (sz != 0)
  {
    _Wait_ready();

    FTFE->FCCOB0 = FLASH_CMD_READ_RESOURCE;
    FTFE->FCCOB1 =(addr >> 16) & 0xFF;
    FTFE->FCCOB2 =(addr >> 8) & 0xFF;
    FTFE->FCCOB3 =(addr >> 0) & 0xF8; // Выравнивание по гранеце 8-и байт
    FTFE->FCCOB4 =rsrs_code;

    stat = _Exec_cmd();
    if (stat != 0)
    {
      return  stat;
    }

    *buf++= FTFE->FCCOB7; if (!(--sz)) break;
    *buf++= FTFE->FCCOB6; if (!(--sz)) break;
    *buf++= FTFE->FCCOB5; if (!(--sz)) break;
    *buf++= FTFE->FCCOB4; if (!(--sz)) break;

    *buf++= FTFE->FCCOBB; if (!(--sz)) break;
    *buf++= FTFE->FCCOBA; if (!(--sz)) break;
    *buf++= FTFE->FCCOB9; if (!(--sz)) break;
    *buf++= FTFE->FCCOB8; if (!(--sz)) break;
    addr += 8;
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------
  Пограммирование распределения областей Data Flash

  \param eerst_flag    - 0 - FlexRAM loaded with valid EEPROM data during reset sequence
                         1 - FlexRAM not loaded during reset sequence
  \param eeprom_sz     - EEPROM Data Set Size Code
  \param df_partition  - FlexNVM Partition Code

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t FLASH_program_partition(int32_t eerst_flag, int32_t eeprom_sz, int32_t df_partition)
{
  uint8_t stat;

  _Wait_ready();
  FTFE->FCCOB0 = FLASH_CMD_PROGRAM_PARTITION;
  FTFE->FCCOB1 = 0xFF;
  FTFE->FCCOB2 = 0xFF;
  if (eerst_flag)
  {
    FTFE->FCCOB3 = 1;
  }
  else
  {
    FTFE->FCCOB3 = 0;
  }

  FTFE->FCCOB4 =eeprom_sz;
  FTFE->FCCOB5 =df_partition;

  stat = _Exec_cmd();
  if (stat != 0)
  {
    return  stat;
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------
  Назначем фнкцию для FlexRAM

  \param function  - 0xFF - Make FlexRAM available as RAM
                     0x00 - Make FlexRAM available for EEPROM

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t FLASH_set_FlexRAM_function(uint8_t func)
{
  uint8_t stat;

  _Wait_ready();

  FTFE->FCCOB0 = FLASH_CMD_SET_FLEXRAM_FUNCTION;
  FTFE->FCCOB1 = func;
  stat = _Exec_cmd();
  if (stat != 0)
  {
    return  stat;
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return __ramfunc uint32_t
-----------------------------------------------------------------------------------------------------*/
__ramfunc uint32_t FLASH_Erase_All_Unsecure(void)
{
  __disable_interrupt();

  // Ожидаем пока бит CCIF не установится в 1
  while (!(FTFE->FSTAT & F_CCIF))
  {
    __no_operation();
  }
  FTFE->FCCOB0 = FLASH_CMD_ERASE_ALL_BLOCK_UNSECURE;

  FTFE->FSTAT = F_RDCOLERR + F_ACCERR + F_FPVIOL;   // Сбрасываем ошибки.
  FTFE->FSTAT = F_CCIF;                             // Сбрасываем бит CCIF. Нельзя эту операцию объединять с предыдущей операцией сброса ошибок

  // Ожидаем пока бит CCIF не установится в 1
  while (!(FTFE->FSTAT & F_CCIF))
  {
    __no_operation();
  }

  // Зависаем, поскольку вся Flash память стерта и программы больше нет.
  while (true)
  {
    __no_operation();
  }


}

/*-----------------------------------------------------------------------------------------------------
  Запись в EEPROM

  \param addr     - смещение в EEPROM по которому будут записываться данные
  \param sz       - размер в байтах записываемых данных
  \param src_buf  - буфер-источник с данными

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint32_t FLASH_write_to_EEPROM(uint32_t offs, uint32_t sz, uint8_t *src)
{
  uint8_t stat;
  uint32_t start = EEPROM_ADDR + offs;


  while (sz > 0)
  {
    if ((!(start & 0x3U)) && (sz >= 4))
    {
      *(uint32_t *)start =*(uint32_t *)src;
      start += 4;
      src += 4;
      sz -= 4;
    }
    else if ((!(start & 0x1U)) && (sz >= 2))
    {
      *(uint16_t *)start =*(uint16_t *)src;
      start += 2;
      src += 2;
      sz -= 2;
    }
    else
    {
      *(uint8_t *)start =*src;
      start += 1;
      src += 1;
      sz -= 1;
    }

    _Wait_EE_ready();

    stat = FTFE->FSTAT;
    if (stat & BIT(4))
    {
      return stat;
    }
  }

  return 0;
}


/*-----------------------------------------------------------------------------------------------------
  Чтение из EEPROM

  \param offs    - смещение в EEPROM по которому будут записываться данные
  \param sz      - размер в байтах читаемых данных
  \param dst_buf - буфер-приемник данных

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t FLASH_read_from_EEPROM(uint32_t offs, uint32_t sz, uint8_t *dst_buf)
{
  uint32_t start = EEPROM_ADDR + offs;
  memcpy(dst_buf, (void *)(start), sz);
  return 0;
}



/*-----------------------------------------------------------------------------------------------------
  Получить информацию о распределении областей Data Flash

  \param void

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t FLASH_get_partition_info(T_DFlash_partition_info *inf)
{
  uint8_t    bf[8];
  uint32_t   ret;
  uint8_t    eeesize_code;
  uint8_t    depart_code;

  if (inf == NULL) return -1;
  inf->df_size = -1;
  inf->ee_size = -1;
  inf->eebckup_size = -1;

  ret = FLASH_read_resource(0x8003F8,FLASH_DFLASH_IFR_ID,8, bf);
  if (ret != 0) return ret;

  eeesize_code = bf[5];
  depart_code = bf[4];
  inf->eerst_flag =(eeesize_code>>6) & 1;
  inf->ee_size = _Get_ee_size(eeesize_code);
  inf->df_size = _Get_Data_flash_size(depart_code);
  inf->eebckup_size = _Get_EEpromBackUp_size(depart_code);
  return 0;
}


/*-----------------------------------------------------------------------------------------------------
  Установить распределении областей Data Flash по умолчанию

  \param

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t FLASH_set_EEPROM_condition(void)
{
  uint32_t   ret;
  T_DFlash_partition_info inf;

  eeprom_ok = false;
  _Wait_ready();
  //_Wait_EE_ready(); Если EEPROM не была прежде сконфигурирована в чипе, то на этой функции можем застрять

  ret = FLASH_get_partition_info(&inf);
  if (ret != 0)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"EEPROM partition read error");
    return ret;
  }
  else
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"EEPROM EERST=%d, EEPROM Size=%d, DataFlash Size=%d, DataFlash to EEPROM size=%d", inf.eerst_flag, inf.ee_size, inf.df_size, inf.eebckup_size);
  }

  if ((inf.eerst_flag != FLASH_DEFAULT_EE_RST_FLAG) || (inf.ee_size != FLASH_DEFAULT_EE_SIZE) || (inf.eebckup_size != FLASH_DEFAULT_EE_BACKUP_SZ))
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"EEPROM check error");
    ret = FLASH_program_partition(FLASH_SET_DEFAULT_EE_RST_FLAG ,FLASH_SET_DEFAULT_EE_SIZE, FLASH_SET_DEFAULT_EE_BACKUP_SZ);
    if (ret != 0) return ret;
    ret = FLASH_set_FlexRAM_function(FLEXRAM_MAKE_AS_EEPROM);
    if (ret != 0) return ret;
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"EEPROM configured successfully");
  }
  else
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"EEPROM check completed successfully");
  }

  eeprom_ok = true;
  return 0;
}
