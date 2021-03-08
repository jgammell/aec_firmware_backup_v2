#%%
# -*- coding: utf-8 -*-

# Connect to device


def degreesToSteps(degrees):
    return int(9144000*(degrees/360))

from motor_driver_interface import MotorDriver

COM = r'COM21'

MD = MotorDriver(COM)

#%%

# Motor control

MD.turnMotor('theta', degreesToSteps(10), 'cw')
MD.turnMotor('theta', degreesToSteps(10), 'ccw')
MD.turnMotor('phi', degreesToSteps(1), 'cw')
MD.turnMotor('phi', degreesToSteps(1), 'ccw')
MD.alignMotor('theta')
MD.alignMotor('phi')

#%%

# Alignment checker control

MD.writeLaser(True)
MD.writeLaser(False)
print(MD.readSensor())

#%%

# Check status

MD.checkAssertInfo()
MD.invokeBsl()