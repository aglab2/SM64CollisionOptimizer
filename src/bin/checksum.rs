#![allow(dead_code)]

#[path = "../checksum.rs"]
mod checksum;

use std::env;
use std::fs;
use std::process;

#[allow(dead_code)]
fn read_u32_be(buf: &[u8]) -> u32 {
    ((buf[0] as u32) << 24)
        | ((buf[1] as u32) << 16)
        | ((buf[2] as u32) << 8)
        | (buf[3] as u32)
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() != 2 {
        eprintln!("Usage: {} <rom_path>", args[0]);
        process::exit(1);
    }

    let rom_path = &args[1];
    let data = match fs::read(rom_path) {
        Ok(d) => d,
        Err(e) => {
            eprintln!("Failed to read '{}': {}", rom_path, e);
            process::exit(1);
        }
    };

    if data.len() < 0x80 {
        eprintln!("ROM too small (need at least 0x80 bytes for header)");
        process::exit(1);
    }

    let (computed_crc1, computed_crc2) = checksum::calculate_checksums(&data);

    // Read the stored checksums from the ROM header
    let stored_crc1 = read_u32_be(&data[0x10..]);
    let stored_crc2 = read_u32_be(&data[0x14..]);

    println!("ROM: {}", rom_path);
    println!("Size: 0x{:X} bytes ({} MB)", data.len(), data.len() / 1024 / 1024);
    println!();
    println!("CRC1: computed = 0x{:08X}, stored = 0x{:08X}", computed_crc1, stored_crc1);
    println!("CRC2: computed = 0x{:08X}, stored = 0x{:08X}", computed_crc2, stored_crc2);
    println!();

    if computed_crc1 == stored_crc1 && computed_crc2 == stored_crc2 {
        println!("Checksums MATCH — ROM is valid.");
    } else {
        println!("Checksums DO NOT MATCH — ROM has been modified or is corrupt.");
        process::exit(2);
    }
}
