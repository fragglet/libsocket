/*
    vwin32.h
    Created by Richard Dawe by code from regdosz.c

    Copyright 1997, 1998 by the RegDos Group
*/

#ifndef _REGDOSZ_VWIN32_H
#define _REGDOSZ_VWIN32_H

#define VWIN32_VXD_ID 0x0001

/* OPCODE_REG_xxx VxD call ids */
#define OPCODE_REG_OPEN_KEY             0x0100
#define OPCODE_REG_CREATE_KEY           0x0101
#define OPCODE_REG_CLOSE_KEY            0x0102
#define OPCODE_REG_DELETE_KEY           0x0103
#define OPCODE_REG_SET_VALUE            0x0104
#define OPCODE_REG_QUERY_VALUE          0x0105
#define OPCODE_REG_ENUM_KEY             0x0106
#define OPCODE_REG_DELETE_VALUE         0x0107
#define OPCODE_REG_ENUM_VALUE           0x0108
#define OPCODE_REG_QUERY_VALUE_EX       0x0109
#define OPCODE_REG_SET_VALUE_EX         0x010A
#define OPCODE_REG_FLUSH_KEY            0x010B
#define OPCODE_REG_LOAD_KEY             0x010C
#define OPCODE_REG_UNLOAD_KEY           0x010D
#define OPCODE_REG_SAVE_KEY             0x010E
#define OPCODE_REG_RESTORE              0x010F
#define OPCODE_REG_REMAPPREDEFKEY       0x0110

#endif  /* _REGDOSZ_VWIN32_H */
