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


def make_disk_img(
    bootloader_path: str, kernel32_path: str, kernel64_path: str
) -> bytes:
    with open(bootloader_path, "rb") as f:
        bootloader_data = f.read()
    with open(kernel32_path, "rb") as f:
        kernel32_data = f.read()
    with open(kernel64_path, "rb") as f:
        kernel64_data = f.read()

    # Calculate the number of sectors after padding
    kernel32_sector_count = calculate_sector_count(kernel32_data)
    kernel64_sector_count = calculate_sector_count(kernel64_data)
    total_sector_count = kernel32_sector_count + kernel64_sector_count

    # Resize each kernel's image to a multiple of SECTOR_SIZE(512)
    padded_kernel32_data = pad_with_zeros(
        kernel32_data, SECTOR_SIZE * kernel32_sector_count
    )
    padded_kernel64_data = pad_with_zeros(
        kernel64_data, SECTOR_SIZE * kernel64_sector_count
    )

    # Set BootLoader's TOTAL_SELECTOR_COUNT
    actual_bootloader_data = modify_bytes(
        bootloader_data, 2, struct.pack("<H", total_sector_count)
    )
    # Set BootLoader's KERNEL32_SECTOR_COUNT
    actual_bootloader_data = modify_bytes(
        actual_bootloader_data, 4, struct.pack("<H", kernel32_sector_count)
    )

    disk_data = (
        actual_bootloader_data + padded_kernel32_data + padded_kernel64_data
    )
    return disk_data


def main(argc: int, argv: list[str]) -> None:
    if argc < 4:
        print(
            f"Usage: {argv[0]} [bootloader path] [kernel32 path] [kernel64 path] {'{Disk Path}'}"
        )
        exit()

    if argc > 4:
        disk_path = argv[3]
    else:
        disk_path = "./Disk.img"

    disk_data = make_disk_img(argv[1], argv[2], argv[3])

    with open(disk_path, "wb+") as f:
        f.write(disk_data)


if __name__ == "__main__":
    main(len(sys.argv), sys.argv)
