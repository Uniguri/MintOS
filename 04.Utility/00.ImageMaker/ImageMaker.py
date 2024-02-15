import sys
import struct

SECTOR_SIZE = 512


def calculate_sector_count(data: bytes) -> int:
    data_size = len(data)
    return (data_size + SECTOR_SIZE - 1) // SECTOR_SIZE


def modify_bytes(source: bytes, location: int, data: bytes) -> bytes:
    assert location < len(source)
    assert len(source) >= len(data) + location

    bytearray_source = bytearray(source)
    for idx, byte in enumerate(data):
        bytearray_source[location + idx] = byte

    return bytes(bytearray_source)


def pad_with_zeros(data: bytes, size_after_padding: int) -> bytes:
    return data.ljust(size_after_padding, b"\x00")


def make_disk_img(bootloader_path: str, kernel32_path: str) -> bytes:
    with open(bootloader_path, "rb") as f:
        bootloader_data = f.read()
    with open(kernel32_path, "rb") as f:
        kernel32_data = f.read()

    sector_count = calculate_sector_count(kernel32_data)

    padded_kernel32_data = pad_with_zeros(
        kernel32_data, SECTOR_SIZE * sector_count
    )
    actual_bootloader_data = modify_bytes(
        bootloader_data, 2, struct.pack("<H", sector_count)
    )

    disk_data = actual_bootloader_data + padded_kernel32_data
    return disk_data


if __name__ == "__main__":
    argc: int = len(sys.argv)
    argv: list[str] = sys.argv

    if argc < 3:
        print(
            f"Usage: {argv[0]} [bootloader path] [kernel32 path] {'{Disk Path}'}"
        )
        exit()

    if argc > 3:
        disk_path = argv[3]
    else:
        disk_path = "./Disk.img"

    disk_data = make_disk_img(argv[1], argv[2])

    with open(disk_path, "wb+") as f:
        f.write(disk_data)
