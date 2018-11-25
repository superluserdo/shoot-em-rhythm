print("Began importing ",__file__)

import ctypes
import pdb
import IPython
import inspect
import structdef


def skip_frames(n):
    for i in range(n):
        yield

def skip_frame():
    yield

def while_ipython(capsule):

    # Get game state from caller in C:
    ctypes.pythonapi.PyCapsule_GetPointer.argtypes=[ctypes.py_object, ctypes.c_char_p]
    ctypes.pythonapi.PyCapsule_GetPointer.restype = ctypes.POINTER(structdef.struct_status_struct)
    state_ptr=ctypes.pythonapi.PyCapsule_GetPointer(capsule,ctypes.c_char_p(None))

    do_break = yield

    def noreturn():
        # Reset python_interpreter_activate to False
        state_ptr[0].program[0].python_interpreter_activate = 0

    print("Handing control to the ipython interpreter:")

    while True:
        
        if do_break:
            print("Ending all Python code")
            exit()
        try:
            funcs = []
            IPython.terminal.embed.embed()
            for func in funcs:
                #if inspect.isgeneratorfunction(func):
                if inspect.isgenerator(func):
                    do_break = yield from func
                else:
                    func()
            print("Leaving exec loop")
            do_break = yield
        except Exception as e: print(e)
