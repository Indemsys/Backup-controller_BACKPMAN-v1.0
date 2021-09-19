#ifndef BACKPMAN10_FLASH_H
  #define BACKPMAN10_FLASH_H

#define IFR_BUF_SZ   1024

#define PROGRAM_FLASH_SECTOR_SIZE  4096
#define DATA_FLASH_SECTOR_SIZE     2048


#define F_CCIF      BIT(7)
#define F_RDCOLERR  BIT(6)
#define F_ACCERR    BIT(5)
#define F_FPVIOL    BIT(4)
#define F_MGSTAT0   BIT(0)


#define FLASH_CMD_VERIFY_BLOCK                    0x00U
#define FLASH_CMD_VERIFY_SECTION                  0x01U
#define FLASH_CMD_PROGRAM_CHECK                   0x02U
#define FLASH_CMD_READ_RESOURCE                   0x03U
#define FLASH_CMD_PROGRAM_LONGWORD                0x06U
#define FLASH_CMD_PROGRAM_PHRASE                  0x07U
#define FLASH_CMD_ERASE_BLOCK                     0x08U
#define FLASH_CMD_ERASE_SECTOR                    0x09U
#define FLASH_CMD_PROGRAM_SECTION                 0x0BU
#define FLASH_CMD_GENERATE_CRC                    0x0CU
#define FLASH_CMD_VERIFY_ALL_BLOCK                0x40U
#define FLASH_CMD_READ_ONCE                       0x41U
#define FLASH_CMD_PROGRAM_ONCE                    0x43U
#define FLASH_CMD_ERASE_ALL_BLOCK                 0x44U
#define FLASH_CMD_SECURITY_BY_PASS                0x45U
#define FLASH_CMD_SWAP_CONTROL                    0x46U
#define FLASH_CMD_ERASE_ALL_BLOCK_UNSECURE        0x49U
#define FLASH_CMD_VERIFY_ALL_EXECUTE_ONLY_SEGMENT 0x4AU
#define FLASH_CMD_ERASE_ALL_EXECUTE_ONLY_SEGMENT  0x4BU
#define FLASH_CMD_PROGRAM_PARTITION               0x80U
#define FLASH_CMD_SET_FLEXRAM_FUNCTION            0x81U

#define FLASH_PFLASH_IFR_ID                       0x00
#define FLASH_DFLASH_IFR_ID                       0x00
#define FLASH_VERSION_ID                          0x01


#define EE_SIZE_CODE__0                           0x3F   // EEPROM Size = 0    Bytes
#define EE_SIZE_CODE__32                          0x39   // EEPROM Size = 32   Bytes
#define EE_SIZE_CODE__64                          0x38   // EEPROM Size = 64   Bytes
#define EE_SIZE_CODE__128                         0x37   // EEPROM Size = 128  Bytes
#define EE_SIZE_CODE__256                         0x36   // EEPROM Size = 256  Bytes
#define EE_SIZE_CODE__512                         0x35   // EEPROM Size = 512  Bytes
#define EE_SIZE_CODE__1024                        0x34   // EEPROM Size = 1024 Bytes
#define EE_SIZE_CODE__2048                        0x33   // EEPROM Size = 2048 Bytes
#define EE_SIZE_CODE__4096                        0x32   // EEPROM Size = 4096 Bytes

#define DF_PART_CODE__DF64_EE00                   0x00   // Data flash Size (Kbytes) = 64,  EEPROM-backup Size (Kbytes) =  0
#define DF_PART_CODE__DF32_EE32                   0x03   // Data flash Size (Kbytes) = 32,  EEPROM-backup Size (Kbytes) = 32
#define DF_PART_CODE__DF00_EE64                   0x04   // Data flash Size (Kbytes) =  0,  EEPROM-backup Size (Kbytes) = 64
#define DF_PART_CODE__DF00_EE64_                  0x08   // Data flash Size (Kbytes) =  0,  EEPROM-backup Size (Kbytes) = 64
#define DF_PART_CODE__DF16_EE48                   0x0A   // Data flash Size (Kbytes) = 16,  EEPROM-backup Size (Kbytes) = 48
#define DF_PART_CODE__DF32_EE32_                  0x0B   // Data flash Size (Kbytes) = 32,  EEPROM-backup Size (Kbytes) = 32
#define DF_PART_CODE__DF64_EE00_                  0x0D   // Data flash Size (Kbytes) = 64,  EEPROM-backup Size (Kbytes) =  0

#define FLEXRAM_IS_NOT_LOADED_ON_RESET            1
#define FLEXRAM_IS_LOADED_ON_RESET                0

#define FLEXRAM_MAKE_AS_RAM                       0xFF
#define FLEXRAM_MAKE_AS_EEPROM                    0x00

typedef struct
{
  int32_t  eerst_flag;   // Значение флага EERST. 0- is not loaded, 1 -is loaded after reset
  int32_t  ee_size;      // Размер эмулируемой EEPROM
  int32_t  df_size;      // Размер доступной Data Flash
  int32_t  eebckup_size; // Размер области отведенной во Data Flash под данные EEProm
                         // Значение -1 означает неопределенность

} T_DFlash_partition_info;




uint8_t   FLASH_Get_state(void);
uint32_t  FLASH_read_resource(uint32_t addr, uint8_t rsrs_code, uint32_t sz, uint8_t *buf);
int32_t   FLASH_get_partition_info(T_DFlash_partition_info *inf);
uint32_t  FLASH_program_partition(int32_t eerst_flag, int32_t eeprom_sz, int32_t df_partition);
int32_t   FLASH_set_EEPROM_condition(void);
uint32_t  FLASH_write_to_EEPROM(uint32_t offs, uint32_t sz, uint8_t *src_buf);
uint32_t  FLASH_read_from_EEPROM(uint32_t offs, uint32_t sz, uint8_t *dst_buf);
uint32_t  FLASH_Erase_All_Unsecure(void);

uint32_t  Flash_Init(void);
uint32_t  Flash_ByteProgram(uint32_t addr,unsigned char *buf, uint32_t sz);
uint32_t  Flash_SectorErase(uint32_t addr);


#endif // BCKPC01_FLASH_H



