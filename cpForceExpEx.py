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

def main(args):
    dll = "cpcspi.dll"
    opcode = '\xf7\x47\x68\x00\x00\x00\xF0\x74\x0D' # TEST DWORD PTR DS:[EDI+68],F0000000 \n JE 0D
    instructions = "cmp byte ptr ss:[ebp+1c],98 \n jne 7 \n mov byte ptr [ebp+1c],9C \n nop \n nop \n nop \n nop \n nop \n nop"     #'\x80\xbd\x1c\x00\x00\x00\x98\x75\x07\xc6\x85\x1c\x00\x00\x00\x9c\x90\x90\x90\x90\x90\x90' we patch 22 bytes
    
    imm = Debugger()
    addresses = imm.search(opcode)

    m = imm.getModule(dll)
    if m:
        start = m.getBaseAddress()
        end = start + m.getSize()

        addrOfInterest = [x for x in addresses if x > start and x < end]

        if len(addrOfInterest) != 1:
            return "[-] Error, can not find exact address inside '%s'"%dll

        imm = Debugger()
        patch = imm.assemble(instructions)
        imm.writeMemory(addrOfInterest[0],patch)

        return "[+] Patched '%s' bytes from address '%08X' inside '%s'"%(len(patch),addrOfInterest[0],dll)
    else:
        return "[-] Error, module '%s' not loaded"%dll
