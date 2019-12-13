#include "protocol.h"

/*
 * validateString
 *      Checks if a string is valid and null terminated
 *
 *  string: String to check
 *  size: Maximum potential size of the string.
 *
 *  returns:
 *      int:
 *          -1: Not null terminated
 *           0: No modifications required
 */
int validateString(char *string, int size) {
  /*
   * If string is larger than size, we're just going to cut off
   * the extranous text by inserting the null term.  As things such
   * as strlen() check for the first null temr.
   * Checking for null termination
   */
  return (string[size - 1] == '\0') ? 0 : 1;
}

/*
 * validateProto(msgProto *packet)
 *      Validates and attempts to auto repair a msgProto struct before wrapping
 *      into a sent packet.
 *
 *  packet: msgProto struct to be analyzed and repaired
 *
 *  returns:
 *      int:
 *           0: Struct is fine to send
 *          -1: Struct should not be sent, unusable info
 */
int validateProto(msgProto *packet) {
  int i;
  enum connectionType connection_type;
  // Non repairables
  // connectionType
  connection_type = packet->connection_type;
  for (i = ESTCONN; i == CLOCONN; i++) {
    if (connection_type > CLOCONN) {
      return -1; // Packet isn't usable
    }
  }
  // ?  Will do channel when I figure out if I need it or not.

  // Repairables
  if (validateString(packet->msg, MAX_MSG_SIZE) != 0) {
    packet->msg[MAX_MSG_SIZE - 1] = '\0';
  }
  if (validateString(packet->msg, MAX_USERNAME_SIZE) != 0) {
    packet->msg[MAX_USERNAME_SIZE - 1] = '\0';
  }
}
