/**
 * \file
 *
 * \brief FLIP protocol definitions.
 *
 * Copyright (c) 2011-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef _FLIP_PROTOCOL_H_
#define _FLIP_PROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup flip_protocol Atmel FLIP Protocol definitions
 *
 * To perform firmware upgrade, Atmel has developed an application called "FLIP"
 * This software is a flexible application which lets you program and
 * configure Atmel's microcontroller devices in their final environment
 * without needing any dedicated hardware.
 * @{
 */

//! \defgroup FLIP Protocol versions
//@{
#define  FLIP_PROTOCOL_VERSION_2     2 // Used by UC3 parts and Xmega parts
#define  FLIP_PROTOCOL_VERSION_1     1 // Used by Mega
#define  FLIP_PROTOCOL_VERSION_0     0 // Used by C51 parts
//@}


/**
 * \defgroup flip_pv1 Definitions for FLIP Protocol version 1
 *
 * Note: Used by Mega parts.
 *
 * @{
 */

COMPILER_PACK_SET(1)
/**
 * \brief FLIP message structure
 */
typedef struct {
	uint8_t cmd_id;
	uint8_t arg[5];
} flip_msg_v1_t;
COMPILER_PACK_RESET()

//! Size of buffer used by FLIP to receive or send data
#define  FLIP_V1_BUF_TRANS_SIZE  1024

//! \name FLIP commands
//@{
#define  FLIP_V1_CMD_PROG_START            0x01
#define  FLIP_V1_CMD_READ                  0x03
#define  FLIP_V1_CMD_WRITE                 0x04
#define  FLIP_V1_CMD_READ_ID               0x05
#define  FLIP_V1_CMD_CHANGE_BASE_ADDR      0x06

//! \name FLIP_V1_CMD_PROG_START first argument
//@{
#define FLIP_V1_CMD_PROG_START_ARG_FLASH   0x00
#define FLIP_V1_CMD_PROG_START_ARG_EEPROM  0x01
#define FLIP_V1_CMD_PROG_START_ARG_CUSTOM  0x08
//@}

//! \name FLIP_V1_CMD_READ first argument
//@{
#define FLIP_V1_CMD_READ_ARG_FLASH      0x00
#define FLIP_V1_CMD_READ_ARG_FLASHCHECK 0x01
#define FLIP_V1_CMD_READ_ARG_EEPROM     0x02
#define FLIP_V1_CMD_READ_ARG_CUSTOM     0x03
//@}

//! \name FLIP_V1_CMD_WRITE first argument
//@{
#define FLIP_V1_CMD_WRITE_ARG_ERASE     0x00
#define FLIP_V1_CMD_WRITE_ARG_RST       0x03
//! \name FLIP_V1_CMD_WRITE_ARG_ERASE following argument
//@{
#define FLIP_V1_CMD_WRITE_ARG_ERASE_CHIP 0xFF
//@}
//! \name FLIP_V1_CMD_WRITE_ARG_RST following arguments
//@{
#define FLIP_V1_CMD_WRITE_ARG_RST_HW    0x00
#define FLIP_V1_CMD_WRITE_ARG_RST_SF    0x01
//@}
//@}

//! \name FLIP_V1_CMD_READ_ID first argument
//@{
#define FLIP_V1_CMD_READ_ID_ARG_BOOTLOADER 0x00
#define FLIP_V1_CMD_READ_ID_ARG_SIGNATURE  0x01
//! \name FLIP_V1_CMD_READ_ID_ARG_SIGNATURE following arguments
//@{
#define FLIP_V1_CMD_READ_ID_SIGNATURE_ARG_MANUF              0x30
#define FLIP_V1_CMD_READ_ID_SIGNATURE_ARG_FAMILY             0x31
#define FLIP_V1_CMD_READ_ID_SIGNATURE_ARG_PRODUCT            0x60
#define FLIP_V1_CMD_READ_ID_SIGNATURE_ARG_REVISION           0x61
//@}
//@}

//! \name FLIP_V1_CMD_CHANGE_BASE_ADDR first argument
//@{
#define FLIP_V1_CMD_CHANGE_BASE_ADDR_ARG0   0x03
#define FLIP_V1_CMD_CHANGE_BASE_ADDR_ARG1   0x00
//@}

//@}
//@}


/**
 * \defgroup flip_pv2 Definitions for Atmel FLIP Protocol version 2
 *
 * Note: Used by uc3 and Xmega parts
 *
 * @{
 */

COMPILER_PACK_SET(1)
/**
 * \brief FLIP message structure
 */
typedef struct {
	uint8_t group;
	uint8_t cmd_id;
	uint8_t arg[4];
} flip_msg_v2_t;
COMPILER_PACK_RESET()

//! Size of buffer used by FLIP to receive or send data
//! Note:  Write uses 2K and Read uses 1KB
#define  FLIP_V2_BUF_TRANS_SIZE  (2*1024)

//! \name Command Groups
//@{
#define FLIP_V2_CMD_GRP_DNLOAD                0x01
#define FLIP_V2_CMD_GRP_UPLOAD                0x03
#define FLIP_V2_CMD_GRP_EXEC                  0x04
#define FLIP_V2_CMD_GRP_SELECT                0x06
//@}

//! \name FLIP_V2_CMD_GRP_DNLOAD first argument
//@{
#define FLIP_V2_CMD_PROGRAM_START             0x00
//@}


//! \name FLIP_V2_CMD_GRP_UPLOAD first argument
//@{
#define FLIP_V2_CMD_READ_MEMORY               0x00
#define FLIP_V2_CMD_BLANK_CHECK               0x01
//@}

//! \name FLIP_V2_CMD_GRP_EXEC first argument
//@{
#define FLIP_V2_CMD_ERASE                     0x00
#define FLIP_V2_CMD_START_APPLI               0x03
//! \name FLIP_V2_CMD_ERASE following argument
//@{
#define FLIP_V2_CMD_ERASE_ARG_CHIP            0xFF
//@}
//! \name FLIP_V2_CMD_START_APPLI following arguments
//@{
#define FLIP_V2_CMD_START_APPLI_ARG_RESET     0x00
#define FLIP_V2_CMD_START_APPLI_ARG_NO_RESET  0x01
//@}
//@}

//! \name FLIP_V2_CMD_GRP_SELECT first argument
//@{
#define FLIP_V2_CMD_SELECT_MEMORY             0x03
//! \name FLIP_V2_CMD_SELECT_MEMORY following arguments
//@{
#define FLIP_V2_CMD_SELECT_MEMORY_ARG_UNIT    0x00
#define FLIP_V2_CMD_SELECT_MEMORY_ARG_PAGE    0x01
//@}
//! \name FLIP_V2_CMD_SELECT_MEMORY_ARG_UNIT following arguments
//! Memory units field
//@{
#define FLIP_V2_CMD_SELECT_MEM_FLASH          0x00
#define FLIP_V2_CMD_SELECT_MEM_EEPROM         0x01
#define FLIP_V2_CMD_SELECT_MEM_SECURITY       0x02
#define FLIP_V2_CMD_SELECT_MEM_CONFIGURATION  0x03
#define FLIP_V2_CMD_SELECT_MEM_BOOTLOADER     0x04
#define FLIP_V2_CMD_SELECT_MEM_SIGNATURE      0x05
#define FLIP_V2_CMD_SELECT_MEM_USER           0x06
#define FLIP_V2_CMD_SELECT_MEM_INT_RAM        0x07
#define FLIP_V2_CMD_SELECT_MEM_EXT_MEM_CS0    0x08
#define FLIP_V2_CMD_SELECT_MEM_EXT_MEM_CS1    0x09
#define FLIP_V2_CMD_SELECT_MEM_EXT_MEM_CS2    0x0A
#define FLIP_V2_CMD_SELECT_MEM_EXT_MEM_CS3    0x0B
#define FLIP_V2_CMD_SELECT_MEM_EXT_MEM_CS4    0x0C
#define FLIP_V2_CMD_SELECT_MEM_EXT_MEM_CS5    0x0D
#define FLIP_V2_CMD_SELECT_MEM_EXT_MEM_CS6    0x0E
#define FLIP_V2_CMD_SELECT_MEM_EXT_MEM_CS7    0x0F
#define FLIP_V2_CMD_SELECT_MEM_EXT_MEM_DF     0x10
#define FLIP_V2_CMD_SELECT_MEM_COUNT          0x11	// Number of memory units
//@}
//@}

//@}

//@}


#ifdef __cplusplus
}
#endif

#endif // _FLIP_PROTOCOL_H_
