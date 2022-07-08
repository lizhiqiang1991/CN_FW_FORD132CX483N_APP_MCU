/* -- Includes -- */
#include "M_FixedFlashAccess.h"

/* -- Marco Define -- */
#define FIXED_FLASH_ACCESS_FLASH_ADDRESS 0x0001E000

#define FIXED_FLASH_ACCESS_ROW_BYTE_SIZE CY_FLASH_SIZEOF_ROW /** < 256 Bytes. */
#define FIXED_FLASH_ACCESS_ROW_SIZE 1U
#define FIXED_FLASH_ACCESS_TOTLA_BYTE_SIZE (CY_FLASH_SIZEOF_ROW * FIXED_FLASH_ACCESS_ROW_SIZE)
#define FIXED_FLASH_ACCESS_PAGE_SIZE (FIXED_FLASH_ACCESS_TOTLA_BYTE_SIZE / FIXED_FLASH_ACCESS_PAGE_BYTE_SIZE)

/* -- Type Define -- */

/* -- Global Variables -- */
static uint8_t MFixedFlashAccess_Buffer[FIXED_FLASH_ACCESS_TOTLA_BYTE_SIZE] = {0x00};

/* -- Local Functions -- */

/* -- Global Functions -- */
bool MFixedFlashAccess_WritePage(uint32_t u32AddrOffset, uint8_t *ExternalSetBuffer, uint8_t ExternalSetBufferSize)
{
    if(((u32AddrOffset % FIXED_FLASH_ACCESS_PAGE_BYTE_SIZE) != 0)\
        && (u32AddrOffset > (FIXED_FLASH_ACCESS_TOTLA_BYTE_SIZE - FIXED_FLASH_ACCESS_PAGE_BYTE_SIZE))\
        && (ExternalSetBuffer == NULL)\
        && (ExternalSetBufferSize <= FIXED_FLASH_ACCESS_PAGE_BYTE_SIZE))
    {
        return false;
    }
    else
    {
        /* Read Total Flash */
        memcpy(MFixedFlashAccess_Buffer, FIXED_FLASH_ACCESS_FLASH_ADDRESS, FIXED_FLASH_ACCESS_TOTLA_BYTE_SIZE);
        /* Recover Page Data */
        memcpy((MFixedFlashAccess_Buffer + u32AddrOffset), ExternalSetBuffer, ExternalSetBufferSize);
        /* Write Flash */
        if(CY_FLASH_DRV_SUCCESS != Cy_Flash_WriteRow(FIXED_FLASH_ACCESS_FLASH_ADDRESS , (uint32 *)MFixedFlashAccess_Buffer))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}

bool MFixedFlashAccess_ReadPage(uint32_t u32AddrOffset, uint8_t *ExternalGetBuffer, uint8_t ExternalGetBufferSize)
{
    if(((u32AddrOffset % FIXED_FLASH_ACCESS_PAGE_BYTE_SIZE) != 0)\
        && (u32AddrOffset > (FIXED_FLASH_ACCESS_TOTLA_BYTE_SIZE - FIXED_FLASH_ACCESS_PAGE_BYTE_SIZE))\
        && (ExternalGetBuffer == NULL)\
        && (ExternalGetBufferSize <= FIXED_FLASH_ACCESS_PAGE_BYTE_SIZE))
    {
        return false;
    }
    else
    {
        memcpy((void *)ExternalGetBuffer, (FIXED_FLASH_ACCESS_FLASH_ADDRESS + u32AddrOffset), ExternalGetBufferSize);
        return true;
    }
}
