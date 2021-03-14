/*
 * interface.h
 *
 *  Created on: Feb 22, 2021
 *      Author: jgamm
 */

#ifndef APP_HEADERS_INTERFACE_H_
#define APP_HEADERS_INTERFACE_H_

#define MAX_CMD_NUM (4U)
#define MAX_CMD_LEN (64U)

#define IF_ACK_CMD         "a"
#define IF_NACK_CMD        "nack"

#define IF_CMDDELIM_CHAR   '\n'
#define IF_ARGDELIM_CHAR   ':'
#define IF_CMDDELIM_STR    "\n"

#define IF_IDEN_PREF       "IDEN"
#define IF_IDEN_RV_TEST    "TEST"
#define IF_IDEN_RV_PROBE   "PROBE"

#define IF_ALIGN_PREF      "ALIGN"
#define IF_ALIGN_ARG0_PHI  "PHI"
#define IF_ALIGN_ARG0_THETA "THETA"

#define IF_WLASER_PREF     "WLASER"
#define IF_WLASER_ARG0_ON  "ON"
#define IF_WLASER_ARG0_OFF "OFF"

#define IF_RSENSOR_PREF    "RSENSOR"

#define IF_RASSERT_PREF    "RASSERT"

#define IF_INVBSL_PREF     "INVBSL"

#define IF_MOVE_PREF       "MOVE"
#define IF_MOVE_ARG0_PHI   "PHI"
#define IF_MOVE_ARG0_THETA "THETA"
#define IF_MOVE_ARG1_CW    "CW"
#define IF_MOVE_ARG1_CCW   "CC"


void IF_informRx(void);

void IF_informTx(void);

void IF_init(void);



#endif /* APP_HEADERS_INTERFACE_H_ */
