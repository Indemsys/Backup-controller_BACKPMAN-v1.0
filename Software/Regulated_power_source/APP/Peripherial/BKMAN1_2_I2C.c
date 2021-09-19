// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020-11-12
// 17:54:12
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



lpi2c_master_handle_t               g_i2c1_handle;
lpi2c_master_handle_t               g_i2c0_handle;

static      TX_EVENT_FLAGS_GROUP    i2c1_flags;
static      TX_EVENT_FLAGS_GROUP    i2c0_flags;

#define     I2C_FLAG_SUCCESS        BIT(0)
#define     I2C_FLAG_NAK            BIT(1)


uint32_t                            DCDC_DAC_comm_errors;
uint32_t                            g_i2c0_write_errors_cnt;

LPI2C_Type *i2c_intfs[2] =  {LPI2C0, LPI2C1};

/*-----------------------------------------------------------------------------------------------------


  \param base
  \param handle
  \param status
  \param userData
-----------------------------------------------------------------------------------------------------*/
static void I2C1_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t status, void *userData)
{
  if (status == kStatus_Success)
  {
    tx_event_flags_set(&i2c1_flags, I2C_FLAG_SUCCESS, TX_OR);
  }
  else if (status == kStatus_LPI2C_Nak)
  {
    tx_event_flags_set(&i2c1_flags, I2C_FLAG_NAK, TX_OR);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param base
  \param handle
  \param status
  \param userData
-----------------------------------------------------------------------------------------------------*/
static void I2C0_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t status, void *userData)
{
  if (status == kStatus_Success)
  {
    tx_event_flags_set(&i2c0_flags, I2C_FLAG_SUCCESS, TX_OR);
  }
  else if (status == kStatus_LPI2C_Nak)
  {
    tx_event_flags_set(&i2c0_flags, I2C_FLAG_NAK, TX_OR);
  }
}
/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_I2C1_Init(void)
{
  lpi2c_master_config_t masterConfig;

  tx_event_flags_create(&i2c1_flags, "I2C1");

  CLOCK_SetIpSrc(kCLOCK_Lpi2c1, kCLOCK_IpSrcSysPllAsync);

  LPI2C_MasterGetDefaultConfig(&masterConfig);
  masterConfig.debugEnable = false;
  masterConfig.ignoreAck = false;
  masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
  masterConfig.baudRate_Hz = 400000U;
  masterConfig.busIdleTimeout_ns = 0;
  masterConfig.pinLowTimeout_ns = 0;
  masterConfig.sdaGlitchFilterWidth_ns = 0;
  masterConfig.sclGlitchFilterWidth_ns = 0;
  LPI2C_MasterInit(LPI2C1,&masterConfig, CLOCK_GetIpFreq(kCLOCK_Lpi2c1));
  LPI2C_MasterTransferCreateHandle(LPI2C1,&g_i2c1_handle, I2C1_master_callback, NULL);

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_I2C1_DeInit(void)
{
  LPI2C_MasterDeinit(LPI2C1);
  tx_event_flags_delete(&i2c1_flags);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_I2C0_Init(void)
{
  lpi2c_master_config_t masterConfig;

  tx_event_flags_create(&i2c0_flags, "I2C0");

  CLOCK_SetIpSrc(kCLOCK_Lpi2c0, kCLOCK_IpSrcSysPllAsync);

  LPI2C_MasterGetDefaultConfig(&masterConfig);
  masterConfig.debugEnable = false;
  masterConfig.ignoreAck = false;
  masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
  masterConfig.baudRate_Hz = 400000U;
  masterConfig.busIdleTimeout_ns = 0;
  masterConfig.pinLowTimeout_ns = 0;
  masterConfig.sdaGlitchFilterWidth_ns = 0;
  masterConfig.sclGlitchFilterWidth_ns = 0;
  LPI2C_MasterInit(LPI2C0,&masterConfig, CLOCK_GetIpFreq(kCLOCK_Lpi2c0));
  LPI2C_MasterTransferCreateHandle(LPI2C0,&g_i2c0_handle, I2C0_master_callback, NULL);

}





/*-----------------------------------------------------------------------------------------------------


  \param intf_num
  \param addr_mode
  \param addr
  \param wrlen
  \param outbuf
  \param rdlen
  \param inbuf
  \param timeout

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t I2C1_WriteRead(T_I2C_addr_mode addr_mode, uint16_t addr, uint32_t wrlen, uint8_t *outbuf, uint32_t rdlen, uint8_t *inbuf, uint32_t timeout)
{
  ULONG                    actual_flags;
  int32_t                  res = kStatus_Fail;
  lpi2c_master_transfer_t  i2c_cmd;

  i2c_cmd.slaveAddress   = addr;
  i2c_cmd.subaddress     = 0;
  i2c_cmd.subaddressSize = 0;
  if (wrlen != 0)
  {
    i2c_cmd.direction      = kLPI2C_Write;
    i2c_cmd.data           = outbuf;
    i2c_cmd.dataSize       = wrlen;
  }
  else
  {
    i2c_cmd.direction      = kLPI2C_Read;
    i2c_cmd.data           = inbuf;
    i2c_cmd.dataSize       = rdlen;
  }
  i2c_cmd.flags          = kLPI2C_TransferDefaultFlag;

  res = LPI2C_MasterTransferNonBlocking(LPI2C1,&g_i2c1_handle,&i2c_cmd);
  if (res != kStatus_Success)
  {
    DCDC_DAC_comm_errors++;
    return RES_ERROR;
  }

  if (tx_event_flags_get(&i2c1_flags, 0xFFFFFFFF, TX_OR_CLEAR , &actual_flags, MS_TO_TICKS(timeout)) != TX_SUCCESS)
  {
    DCDC_DAC_comm_errors++;
    return RES_ERROR;
  }

  if (actual_flags == I2C_FLAG_NAK)
  {
    DCDC_DAC_comm_errors++;
    return RES_ERROR;
  }

  return RES_OK;
}
