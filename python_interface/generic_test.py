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

import time
t_0 = time.time()
MD.turnMotor('theta', degreesToSteps(10), 'cw')
MD.turnMotor('theta', degreesToSteps(10), 'ccw')
print(time.time()-t_0)

MD.turnMotor('theta', 10000, 'ccw')
MD.turnMotor('theta', degreesToSteps(180), 'ccw')
MD.turnMotor('phi', 100000, 'ccw')
MD.turnMotor('phi', degreesToSteps(1), 'ccw')
MD.alignMotor('theta')
MD.alignMotor('phi')


#%%

MD.startTurnMotor('theta', 4500000, 'cw')
MD.startTurnMotor('phi', 9187000, 'cw')
MD.waitForDone()
MD.waitForDone()

# Theta motor: roughly 4499000
# Phi motor: roughly 9142000

phi_angle = 9187000

#%%

import time
for i in range(50):
    MD.turnMotor('theta', 1000, 'cw')
    time.sleep(.25)
    MD.turnMotor('theta', 1000, 'ccw')
    time.sleep(.25)

MD.turnMotor('theta', 4494400, 'cw')
MD.turnMotor('phi', 9142000, 'cw')
#%%

angle = 0
MD.turnMotor('theta', 100, 'cw')
angle += 100
MD.turnMotor('theta', 100, 'ccw')
angle -= 100

# Alignment checker control

MD.writeLaser(True)
MD.writeLaser(False)
print(MD.readSensor())

#%%

# Check status

MD.checkAssertInfo()
MD.invokeBsl()