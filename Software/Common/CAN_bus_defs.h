#ifndef CAN_BUS_DEFS_H
  #define CAN_BUS_DEFS_H


  #define   REQUEST_STATE_TO_ALL              0x00000001  // Команда всем передать свое состояние

  #define   BCKPC_STATE_PACKET                0x00000060  // Ответ контроллера резервного питания.
  #define   BCKPC_CMD_PACKET                  0x00000070  // Команда контроллеру резервного питания.

  #define   ALL_ENTER_TO_WRK_MOD              0x00001000  // Команда всем перейти в рабочий режим


#endif // CAN_BUS_DEFS_H



