use crate::checksum;
use crate::embedded_data::{PATCHES};
use crate::patcher::Error::{Mismatch, TooShort};
use std::result;

use thiserror::Error;

#[derive(Error, Debug)]
pub enum Error {
    #[error("ROM file is too short, need at least {size}")]
    TooShort{
        size: u32,
    },

    #[error("Patch {name} is mismatching after application")]
    Mismatch{
        name: &'static str
    },
}
type Result<T> = result::Result<T, Error>;

/// Patch a ROM image using embedded bin data.
/// Zeros out regions for functions with .bin files or hardcoded_zero entries,
/// then writes the binary data into those regions.
pub fn patch_rom(rom: &mut [u8]) -> Result<()> {
    let rom_size = rom.len();

    // Step 1: Zero out all function regions (use JSON length for zeroing)
    for patch in PATCHES {
        if patch.rom_addr as usize + patch.length > rom_size {
            return Err(TooShort{ size: patch.rom_addr + patch.length as u32 });
        }

        let start = patch.rom_addr as usize;
        for i in 0..patch.length {
            rom[start + i] = 0x00;
        }
    }

    // Step 2: Write bin data over the zeros (only for patches with .bin files)
    for patch in PATCHES {
        if !patch.has_bin {
            continue;
        }
        let start = patch.rom_addr as usize;
        for (i, &byte) in patch.data.iter().enumerate() {
            rom[start + i] = byte;
        }
    }

    // Step 3: Verify written content matches source data
    // Only verify patches that have actual .bin files
    for patch in PATCHES {
        if !patch.has_bin {
            continue;
        }
        let start = patch.rom_addr as usize;
        let rom_region = &rom[start..start + patch.data.len()];
        if rom_region != patch.data {
            return Err(Mismatch{ name: patch.name });
        }
    }

    // Step 4: Calculate and write N64 header checksums
    checksum::update_header_checksums(rom);

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_patch_rom_small() {
        // Create a minimal ROM image (just big enough for the patches)
        let max_addr: u32 = PATCHES.iter().map(|p| p.rom_addr + p.length as u32).max().unwrap();
        let mut rom = vec![0xFFu8; (max_addr * 2) as usize];

        patch_rom(&mut rom).unwrap();

        // Verify patches with .bin files were written correctly
        for patch in PATCHES {
            if !patch.has_bin {
                continue;
            }
            let start = patch.rom_addr as usize;
            assert_eq!(&rom[start..start + patch.data.len()], patch.data);
        }

        // Verify checksums are now valid (recalculate and compare)
        let (crc1, crc2) = checksum::calculate_checksums(&rom);
        let stored_crc1 = ((rom[0x10] as u32) << 24) | ((rom[0x11] as u32) << 16) | ((rom[0x12] as u32) << 8) | (rom[0x13] as u32);
        let stored_crc2 = ((rom[0x14] as u32) << 24) | ((rom[0x15] as u32) << 16) | ((rom[0x16] as u32) << 8) | (rom[0x17] as u32);
        assert_eq!(crc1, stored_crc1);
        assert_eq!(crc2, stored_crc2);
    }
}
