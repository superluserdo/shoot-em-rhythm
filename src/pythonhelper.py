print("Began importing ",__file__)

import ctypes
import pdb
import structdef

def launch_interpreter_with_state(capsule):
    print("You are in python right now")
    ctypes.pythonapi.PyCapsule_GetPointer.argtypes=[ctypes.py_object, ctypes.c_char_p]
    ctypes.pythonapi.PyCapsule_GetPointer.restype = ctypes.POINTER(structdef.struct_status_struct)
    ptr=ctypes.pythonapi.PyCapsule_GetPointer(capsule,ctypes.c_char_p(None))
    print("Handing control to the python interpreter, reading from the pipe pypipe")
    while_exec(ptr)
    print("Returning to the C program (main loop)")
    return capsule

def once_exec():
    try:
        exec(open('pypipe').readline(),globals())
    except Exception as e: print(e)

def while_exec(ptr):
    try:
        pipe = open('pypipe')
    except Exception as e:
        print(e)
        return

    while True:
        try:
            instr = pipe.readline()
            if instr == 'break\n':
                print("Leaving exec loop")
                break
            else:
                exec(instr,locals())
        except Exception as e: print(e)
print("Finished importing ",__file__)
