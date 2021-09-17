from building import *

cwd  = GetCurrentDir()
CPPPATH = [cwd + '/include']

src = Split('''
    src/hci_transport_h4.c
    src/hci_transport_h4_uart.c
    src/chipset.c
    src/hm_dump.c
    porting/init.c

    examples/advertisement.c
''')

# For stack choice.
if GetDepend(['HM_USING_STACK']):
    # Use BTStack
    if GetDepend(['HM_USING_STACK_BTSTACK']):
        src += Glob('porting/btstack/*.c')
        CPPPATH += [cwd + '/porting/btstack']

    # Use NimBLE
    elif GetDepend(['HM_USING_STACK_NIMBLE']):
        src += Glob('porting/nimble/nimble.c')

# For chipset choice.
if GetDepend(['HM_USING_CHIPSET']):
    # Chipset choice
    if GetDepend(['HM_USING_CHIPSET_BCM']):
        src += ['porting/chipset/bcm.c']

    elif GetDepend(['HM_USING_CHIPSET_CSR8311']):
        src += ['porting/chipset/csr8311.c']

    elif GetDepend(['HM_USING_CHIPSET_ZEPHYR']):
        src += ['porting/chipset/zephyr.c']


# For test.
if GetDepend(['HM_USING_TEST']):
    src += [
        # 'tests/hci_transport_h4.c',
        # 'tests/chipset.c'
        # 'tests/npl_os.c'
    ]

group = DefineGroup('hm', src, depend = [''], CPPPATH = CPPPATH)
Return ('group')