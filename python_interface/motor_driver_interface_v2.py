# -*- coding: utf-8 -*-

import serial
import time

CMDDELIM = '\n'
ARGDELIM = ':'
ACK      = 'a'
NACK     = 'nack'
IDEN     = 'IDEN'
ALIGN    = 'ALIGN'
WLASER   = 'WLASER'
RSENSOR  = 'RSENSOR'
RASSERT  = 'RASSERT'
INVBSL   = 'INVBSL'
MOVE     = 'MOVE'


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
    def _txCmd(self, args):
        assert type(args)==list
        assert len(args)>=1
        msg = ARGDELIM.join(args)+CMDDELIM
        self.ser.write(msg.encode('ASCII'))
        return msg[:-1]
    def _rxCmd(self):
        msg = []
        while (len(msg)==0) or (msg[-1] != CMDDELIM):
            while self.ser.in_waiting == 0:
                pass
            msg.append(self.ser.read().decode('ASCII'))
        print(msg)
        msg = ''.join(msg)
        return msg[:-1]
    def getId(self):
        args = [IDEN]
        cmd = self._txCmd(args)
        assert self._rxCmd() == ACK
        rv = self._rxCmd()
        assert self._rxCmd() == cmd
        return rv
    def getAssertInfo(self):
        args = [RASSERT]
        cmd = self._txCmd(args)
        assert self._rxCmd() == ACK
        rv = []
        for i in range(3):
            rv.append(self._rxCmd())
        assert self._rxCmd() == cmd
        return {'File': rv[0], 'Condition': rv[1], 'Line': int(rv[2])}
    def invokeBsl(self):
        args = [INVBSL]
        cmd = self._txCmd(args)
        assert self._rxCmd() == ACK
        assert self._rxCmd() == cmd
    def writeLaser(self, state):
        args = [WLASER]
        args.append('ON' if state else 'OFF')
        cmd = self._txCmd(args)
        assert self._rxCmd() == ACK
        assert self._rxCmd() == cmd
    def readSensor(self):
        args = [RSENSOR]
        cmd = self._txCmd(args)
        assert self._rxCmd() == ACK
        rv = self._rxCmd()
        assert self._rxCmd() == cmd
        return int(rv)
    def turnMotor(self, motor, num_steps, direction):
        assert motor in ['theta', 'phi']
        assert type(num_steps) == int
        assert num_steps == num_steps&0xFFFFFFFF
        assert direction in ['cw', 'ccw']
        args = [MOVE]
        args.append('PHI' if motor=='phi' else 'THETA')
        args.append('CW' if direction=='cw' else 'CC')
        args.append('%d'%(num_steps))
        cmd = self._txCmd(args)
        t0 = time.time()
        assert self._rxCmd() == ACK
        assert self._rxCmd() == cmd
        return time.time()-t0