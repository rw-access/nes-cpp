import ctypes
import sys


def main(dll_path: str, rom: str):
    dll = ctypes.CDLL(dll_path)

    dll.CreateInteractiveConsole.argtypes = [ctypes.c_char_p]
    dll.CreateInteractiveConsole.restype = ctypes.c_void_p

    dll.Done.argtypes = [ctypes.c_void_p]
    dll.Done.restype = ctypes.c_bool
    dll.StepFrame.argtypes = [ctypes.c_void_p]
    dll.HandleInteraction.argtypes = [ctypes.c_void_p]
    dll.UpdateButtons.argtypes = [ctypes.c_void_p, ctypes.c_uint8]
    dll.FrameLimit.argtypes = [ctypes.c_void_p]

    # load up the console
    console = dll.CreateInteractiveConsole(rom.encode("utf8"))

    # skip ahead 10s
    frame = 0
    while not dll.Done(console):
        dll.StepFrame(console)
        dll.HandleInteraction(console)
        frame += 1

        if frame > 60 * 15:
            #     A      = 0,
            #     B      = 1,
            #     Select = 2,
            #     Start  = 3,
            #     Up     = 4,
            #     Down   = 5,
            #     Left   = 6,
            #     Right  = 7,

            # dll.UpdateButtons(console, frame % 60 == 2)
            pass

        dll.FrameLimit(console)


if __name__ == "__main__":
    # DLL, ROM
    main(sys.argv[1], sys.argv[2])
