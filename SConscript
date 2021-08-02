from building import *

cwd  = GetCurrentDir()

inc = []

src = Split('''
    src/hci_transport_h4.c
    src/hci_transport_h4_uart.c
    src/chipset.c
    porting/init.c
''')

# For stack choice
src += [
	'porting/btstack/btstack.c',
    'porting/btstack/btstack_rtthread_hm.c',
]
inc += [
    cwd + '/porting/btstack'
]

# For chipset choice
src += [
    # 'porting/chipset/bcm.c',
    # 'porting/chipset/csr8311.c',
    'porting/chipset/zephyr.c'
]

# For test
# src += [
#     'tests/hci_transport_h4.c',
#     'tests/chipset.c'
# ]

inc += [cwd + '/include']

group = DefineGroup('hci-middleware', src, depend = [''], CPPPATH = inc)
Return ('group')