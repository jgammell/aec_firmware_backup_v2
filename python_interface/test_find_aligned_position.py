from motor_driver_interface_v2 import MotorDriver
from find_aligned_position import findAlignedPosition

COM = r'COM15'
MD = MotorDriver(COM)

findAlignedPosition(MD, MD)