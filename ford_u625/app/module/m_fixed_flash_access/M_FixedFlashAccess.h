#ifndef M_FIXEDLASHACCESS_H
#define M_FIXEDLASHACCESS_H

#ifdef __cplusplus
extern "C" {
#endif

/* -- Includes -- */
#include "main.h"

/* -- Marco Define -- */
#define FIXED_FLASH_ACCESS_PAGE_BYTE_SIZE 32U

/* -- Data Type Define -- */

/* -- Extern Global Variables -- */

/* -- Extern Functions -- */
extern bool MFixedFlashAccess_WritePage(uint32_t u32AddrOffset, uint8_t *ExternalSetBuffer, uint8_t ExternalSetBufferSize);
extern bool MFixedFlashAccess_ReadPage(uint32_t u32AddrOffset, uint8_t *ExternalGetBuffer, uint8_t ExternalGetBufferSize);

#endif
