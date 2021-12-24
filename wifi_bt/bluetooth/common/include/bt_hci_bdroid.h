/******************************************************************************
 *
 *  Copyright 2021-2021 AML Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#pragma once

#ifndef HCI_ACL_MAX_SIZE
#define HCI_ACL_MAX_SIZE 1024
#endif
#ifndef HCI_MAX_FRAME_SIZE
#define HCI_MAX_FRAME_SIZE (HCI_ACL_MAX_SIZE + 4)
#endif

/* Message event mask across Host/Controller lib and stack */
#ifndef MSG_EVT_MASK
#define MSG_EVT_MASK 0xFF00     /* eq. BT_EVT_MASK */
#endif
#ifndef MSG_SUB_EVT_MASK
#define MSG_SUB_EVT_MASK 0x00FF /* eq. BT_SUB_EVT_MASK */
#endif

/* Message event ID passed from Host/Controller lib to stack */
#ifndef MSG_HC_TO_STACK_HCI_ACL
#define MSG_HC_TO_STACK_HCI_ACL 0x1100      /* eq. BT_EVT_TO_BTU_HCI_ACL */
#endif
#ifndef MSG_HC_TO_STACK_HCI_SCO
#define MSG_HC_TO_STACK_HCI_SCO 0x1200      /* eq. BT_EVT_TO_BTU_HCI_SCO */
#endif
#ifndef MSG_HC_TO_STACK_HCI_ERR
#define MSG_HC_TO_STACK_HCI_ERR 0x1300      /* eq. BT_EVT_TO_BTU_HCIT_ERR */
#endif
#ifndef MSG_HC_TO_STACK_HCI_ISO
#define MSG_HC_TO_STACK_HCI_ISO 0x1700      /* eq. BT_EVT_TO_BTU_HCI_ISO */
#endif
#ifndef MSG_HC_TO_STACK_HCI_EVT
#define MSG_HC_TO_STACK_HCI_EVT 0x1000      /* eq. BT_EVT_TO_BTU_HCI_EVT */
#endif

/* Message event ID passed from stack to vendor lib */
#ifndef MSG_STACK_TO_HC_HCI_ACL
#define MSG_STACK_TO_HC_HCI_ACL 0x2100 /* eq. BT_EVT_TO_LM_HCI_ACL */
#endif
#ifndef MSG_STACK_TO_HC_HCI_SCO
#define MSG_STACK_TO_HC_HCI_SCO 0x2200 /* eq. BT_EVT_TO_LM_HCI_SCO */
#endif
#ifndef MSG_STACK_TO_HC_HCI_ISO
#define MSG_STACK_TO_HC_HCI_ISO 0x2d00 /* eq. BT_EVT_TO_LM_HCI_ISO */
#endif
#ifndef MSG_STACK_TO_HC_HCI_CMD
#define MSG_STACK_TO_HC_HCI_CMD 0x2000 /* eq. BT_EVT_TO_LM_HCI_CMD */
#endif

/* Local Bluetooth Controller ID for BR/EDR */
#ifndef LOCAL_BR_EDR_CONTROLLER_ID
#define LOCAL_BR_EDR_CONTROLLER_ID 0
#endif

/******************************************************************************
 *  Type definitions and return values
 *****************************************************************************/

typedef struct {
  uint16_t event;
  uint16_t len;
  uint16_t offset;
  uint16_t layer_specific;
  uint8_t data[];
} HC_BT_HDR;

#ifndef BT_HC_HDR_SIZE
#define BT_HC_HDR_SIZE (sizeof(HC_BT_HDR))
#endif