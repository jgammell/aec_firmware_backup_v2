from motor_driver_interface_v2 import MotorDriver
import keyboard
import os
import time

def flush_input():
    try:
        import sys, termios
        termios.tcflush(sys.stdin, termios.TCIOFLUSH)
    except ImportError:
        import msvcrt
        while msvcrt.kbhit():
            msvcrt.getch()

def _printInstructions(status, increment, sensor_output, test_theta, test_phi, probe_phi, mcu_updated):
    os.system('cls' if os.name == 'nt' else 'clear')
    print('Calibration routine')
    print('\tControls:')
    print('\t\ta / d: rotate test-theta CCW / CW (+spacebar: find end switch)')
    print('\t\tw / s: rotate test-phi   CCW / CW (+spacebar: find end switch)')
    print('\t\tq / e: rotate probe-phi  CCW / CW (+spacebar: find end switch)')
    print('\t\tz:     set steps per press')
    print('\t\tc:     indicate present state as aligned')
    print('\t\tv:     get current sensor output')
    print('\t\tx:     exit')
    print('\tCurrent motor orientations:')
    if test_theta != None:
        print('\t\tTest-theta:     %d'%(test_theta))
    else:
        print('\t\tTest-theta:     Unknown -- must find end switch.')
    if test_phi != None:
        print('\t\tTest-phi:       %d'%(test_phi))
    else:
        print('\t\tTest-phi:       Unknown -- must find end switch.')
    if probe_phi != None:
        print('\t\tProbe-phi:      %d'%(probe_phi))
    else:
        print('\t\tProbe-phi:      Unknown -- must find end switch.')
    print('\t\tMCU up to date: %s'%(mcu_updated))
    print('\tCurrent steps per press:')
    print('\t\t%d'%(increment))
    if sensor_output != None:
        print('\tLast sensor reading:')
        print('\t\t%d'%(sensor_output))
    print('\n\t%s'%(status))

def findAlignedPosition(MD_test, MD_probe):
    increment = 1000
    sensor_output = None
    test_theta = None
    test_phi = None
    probe_phi = None
    mcu_updated = False
    
    def printInstructions(status):
        _printInstructions(status, increment, sensor_output, test_theta, test_phi, probe_phi, mcu_updated)
    
    printInstructions('Ready for next command.')
    
    while True:
        if keyboard.is_pressed('a'): # Rotate test-theta CW
            mcu_updated = False
            if keyboard.is_pressed('space'): # Find end switch
                printInstructions('Aligning test-theta with end switch CW...')
                MD_test.findEndSwitch('theta', 'ccw')
                test_theta = 0
            else:
                printInstructions('Rotating test-theta CW...')
                MD_test.turnMotor('theta', increment, 'ccw')
                if test_theta != None:
                    test_theta -= increment
            while keyboard.is_pressed('a'):
                pass
            printInstructions('Ready for next command.')
                
        if keyboard.is_pressed('d'): # Rotate test-theta CCW
            mcu_updated = False
            if keyboard.is_pressed('space'): # Find end switch
                printInstructions('Aligning test-theta with end switch CCW...')
                MD_test.findEndSwitch('theta', 'cw')
                test_theta = 0
            else:
                printInstructions('Rotating test-theta CCW...')
                MD_test.turnMotor('theta', increment, 'cw')
                if test_theta != None:
                    test_theta += increment
            while keyboard.is_pressed('d'):
                pass
            printInstructions('Ready for next command.')
                
        if keyboard.is_pressed('w'): # Rotate test-phi CCW
            mcu_updated = False
            if keyboard.is_pressed('space'): # Find end switch
                printInstructions('Aligning test-phi with end switch CCW...')
                MD_test.findEndSwitch('phi', 'ccw')
                test_phi = 0
            else:
                printInstructions('Rotating test-phi CCW...')
                MD_test.turnMotor('phi', increment, 'ccw')
                if test_phi != None:
                    test_phi += increment
            while keyboard.is_pressed('w'):
                pass
            printInstructions('Ready for next command.')
                
        if keyboard.is_pressed('s'): # Rotate test-phi CW
            mcu_updated = False
            if keyboard.is_pressed('space'): # Find end switch
                printInstructions('Aligning test-phi with end switch CW...')
                MD_test.findEndSwitch('phi', 'cw')
                test_phi = 0
            else:
                printInstructions('Rotating test-phi CW...')
                MD_test.turnMotor('phi', increment, 'cw')
                if test_phi != None:
                    test_phi -= increment
            while keyboard.is_pressed('s'):
                pass
            printInstructions('Ready for next command.')
                
        if keyboard.is_pressed('q'): # Rotate probe-phi CCW
            mcu_updated = False
            if keyboard.is_pressed('space'): # Find end switch
                printInstructions('Aligning probe-phi with end switch CCW...')
                MD_probe.findEndSwitch('phi', 'ccw')
                probe_phi = 0
            else:
                printInstructions('Rotating probe-phi CCW...')
                MD_probe.turnMotor('phi', increment, 'ccw')
                if probe_phi != None:
                    probe_phi -= increment
            while keyboard.is_pressed('q'):
                pass
            printInstructions('Ready for next command.')
                
        if keyboard.is_pressed('e'): # Rotate probe-phi CW
            mcu_updated = False
            if keyboard.is_pressed('space'): # Find end switch
                printInstructions('Aligning probe-phi with end switch CW...')
                MD_probe.findEndSwitch('phi', 'cw')
                probe_phi = 0
            else:
                printInstructions('Rotating probe-phi CW...')
                MD_probe.turnMotor('phi', increment, 'cw')
                if probe_phi != None:
                    probe_phi += increment
            while keyboard.is_pressed('e'):
                pass
            printInstructions('Ready for next command.')
                
        if keyboard.is_pressed('z'):
            while keyboard.is_pressed('z'):
                pass
            printInstructions('Configuring step size...')
            print('\n\t', end='')
            while True:
                try:
                    flush_input()
                    raw = input('Type step size, then enter.\n\t\t')
                    raw = int(raw)
                    assert 0 < raw < 0xFFFFFFFF
                    increment = raw
                    break
                except:
                    printInstructions('Configuring step size...')
                    print('\n\tInvalid input. ', end='')
            printInstructions('Ready for next command.')
        
        if keyboard.is_pressed('c'):
            printInstructions('Updating aligned orientation on MCU...')
            MD_test.setAlignedOrientation(test_theta, test_phi)
            MD_probe.setAlignedOrientation(0, probe_phi)
            mcu_updated = True
            while keyboard.is_pressed('c'):
                pass
            printInstructions('Ready for next command.')
        
        if keyboard.is_pressed('v'):
            printInstructions('Reading sensor value...')
            MD_test.writeLaser(True)
            time.sleep(.1)
            sensor_output = MD_probe.readSensor()
            MD_test.writeLaser(False)
            printInstructions('Ready for next command.')
                
        if keyboard.is_pressed('x'): # Exit function
            printInstructions('Exited program.')
            flush_input()
            break