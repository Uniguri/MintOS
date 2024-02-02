import gdb

def GetPrettyStrFromBytes(val: bytes):
    assert(len(val)%16 == 0)
    val = val.hex()
    return ' '.join([val[i*16:(i+1)*16] for i in range(len(val)//16)])
    

def LoadDataOnMemory():
    ret: dict[int:bytes] = {}
    inf = gdb.inferiors()[0]
    for i in range(0x400):
        now_addr = 0x10000 + 0x200*i
        val = inf.read_memory(now_addr, 512)
        ret[now_addr] = bytes(val)
    return ret

def ReadDiskImage(image_path: str) -> bytes:
    with open(image_path, 'rb') as f:
       return f.read()

def SeperateDiskImage(img: bytes) -> list[bytes]:
    assert(len(img)%512 == 0)
    return [img[i*512:(i+1)*512] for i in range(len(img)//512)]

if(__name__ == '__main__'):
    raw_img = ReadDiskImage('./Disk.img')
    # Ignore first sector
    sectors = SeperateDiskImage(raw_img)[1:]
    
    loaded_data = LoadDataOnMemory()
    for i, data in enumerate(sectors):
        addr = 0x10000 + 0x200*i
        assert(addr in loaded_data)
        if(data == loaded_data[addr]):
            print(f'{hex(addr)} : OK')
        else:
            pretty_data_from_img = GetPrettyStrFromBytes(data)
            pretty_data_on_memory = GetPrettyStrFromBytes(loaded_data[addr])
            print(f'{hex(addr)} : \n\tDisk.img : {pretty_data_from_img}\n\t  Loaded : {pretty_data_on_memory}')