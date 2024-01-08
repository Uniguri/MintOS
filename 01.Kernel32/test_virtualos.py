import gdb

inf = gdb.inferiors()[0]
for i in range(0x400):
    offset = 0x200*i
    addr = 0x10000 + offset
    val = inf.read_memory(addr, 0x19).hex()
    pretty_val = f'{val[:8*1*2]} {val[8*1*2:8*2*2]} {val[8*2*2:8*3*2]} {val[8*3*2:]}'
    if(i%10 == 0):
        print('')
    print(f'{hex(addr)} : {pretty_val}')