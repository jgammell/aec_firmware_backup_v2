# -*- coding: utf-8 -*-

import serial

TERM_CHAR = '\n'
ACK_CHAR = 'a'
DONE_CHAR = 'd'

TURNMOTOR_CMD = '0'
ALIGNMOTOR_CMD = '1'
CHECKALIGNMENT_CMD = '2'
REPORTSTATUS_CMD = '3'

THETA_MSG = '0'
PHI_MSG = '1'

CW_MSG = '0'
CCW_MSG = '1'

class MotorDriver:
    def __init__(self, port,
                 baudrate=9600,
                 bytesize=serial.EIGHTBITS,
                 parity=serial.PARITY_NONE,
                 stopbits=serial.STOPBITS_ONE):
        self.ser = serial.Serial(port=port,
                                 baudrate=baudrate,
                                 bytesize=bytesize,
                                 parity=parity,
                                 stopbits=stopbits)
    def __del__(self):
        try:
            self.ser.close()
        except:
            pass
    def _transmitMsg(self, msg):
        msg = msg+TERM_CHAR
        msg = msg.encode()
        self.ser.write(msg)
    def _nextMsg(self):
        msg = []
        while (len(msg) == 0) or (msg[-1] != TERM_CHAR):
            while self.ser.in_waiting == 0:
                pass
            msg.append(self.ser.read().decode('ascii'))
        msg = ''.join(msg)
        return msg[:-1]
    def turnMotor(self, motor, num_steps, direction):
        assert motor in ['theta', 'phi']
        assert type(num_steps) == int
        assert num_steps == num_steps&0xFFFFFFFF
        assert direction in ['cw', 'ccw']
        msg = []
        msg.append(TURNMOTOR_CMD)
        if motor == 'theta':
            msg.append(THETA_MSG)
        else:
            msg.append(PHI_MSG)
        msg += '%010d'%(num_steps)
        if direction == 'cw':
            msg.append(CW_MSG)
        else:
            msg.append(CCW_MSG)
        self._transmitMsg(''.join(msg))
        print('sent message')
        rv = self._nextMsg()
        assert rv == ACK_CHAR
        print('acked')
        rv = self._nextMsg()
        assert rv == DONE_CHAR
        print('done')
    def alignMotor(self, motor):
        assert motor in ['theta', 'phi']
        msg = []
        msg.append(ALIGNMOTOR_CMD)
        if motor == 'theta':
            msg.append(THETA_MSG)
        else:
            msg.append(PHI_MSG)
        self._transmitMsg(''.join(msg))
        rv = self._nextMsg()
        assert rv == ACK_CHAR
        rv = self._nextMsg()
        assert rv == DONE_CHAR
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        