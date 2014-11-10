"""

       ( HOLA!!!MY FRIEND!!! )
                /\\               ( THIS IS IMMUNITY DEBUGGER SCRIPT  )
                \ \\  \__/ \__/  / ( THAT PATCHES CPCSPI.DLL THAT IS )
                 \ \\ (oo) (oo) /    ( LOADED INTO PROCESS WE DEBUG!!! )
                  \_\\/~~\_/~~\_
                 _.-~===========~-._
                (___/_______________)
                   /  \_______/
       ( YOU ARE FREE TO USE & MODIFY THIS SCRIPT AT YOUR OWN RISK!!! )

                                                                by 0ang3el
"""

from immlib import *

class cpHook(LogBpHook):
    def __init__(self):
        LogBpHook.__init__(self)
        self.imm = Debugger()

    def run(self, regs):
        ebp = regs["EBP"]

        expFlag = self.imm.readLong(ebp+0x1C)

        if expFlag & 0xFFF == 0x998: # 0x998 - not exportable container
            self.imm.writeLong(ebp+0x1C,0x99C) # 0x99c - exportable container


def main(args):
    opcode = '\xf7\x45\x1c\x00\x08\x00\x00' # TEST DWORD PTR SS:[EBP+1C],800
    dll = "cpcspi.dll"

    imm = Debugger()

    addresses = imm.search(opcode)

    m = imm.getModule(dll)
    if m:
        start = m.getBaseAddress()
        end = start + m.getSize()

        addrOfInterest = [x for x in addresses if x > start and x < end]

        if len(addrOfInterest) != 1:
            return "[-] Error, can not find exact address inside '%s'"%dll

        hook = cpHook()
        hook.add("%08X"%addrOfInterest[0],addrOfInterest[0])

        return "[+] LogBP planted for '%08X' inside '%s'"%(addrOfInterest[0],dll)
    else:
        return "[-] Error, module '%s' not loaded"%dll
