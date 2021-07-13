from building import *

cwd  = GetCurrentDir()

src = Split('''
    src/hci_transport_h4.c
    src/hci_transport_h4_uart.c

''')

src += [
    'tests/hci_transport_h4.c'
]

CPPPATH = [cwd + "/include"]

group = DefineGroup('hci-middleware', src, depend = [''], CPPPATH = CPPPATH)
Return ('group')