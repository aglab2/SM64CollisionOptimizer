/// N64 ROM checksum calculator — port of chksum.c
/// Computes CRC1 and CRC2 exactly as SM64 boot code does.

pub fn calculate_checksums(data: &[u8]) -> (u32, u32) {
    let mut cksum = [0u32; 2];
    sm64_calc_checksums_impl(data, &mut cksum);
    (cksum[0], cksum[1])
}

/// Update checksum fields in the N64 ROM header at offsets 0x10 and 0x14.
pub fn update_header_checksums(rom: &mut [u8]) {
    let (crc1, crc2) = calculate_checksums(rom);
    write_u32_be(&mut rom[0x10..], crc1);
    write_u32_be(&mut rom[0x14..], crc2);
}

/// Derived from SM64 boot code (file.c at 0x59C).
fn sm64_calc_checksums_impl(buf: &[u8], cksum: &mut [u32; 2]) {
    let s6 = 0x3Fu32;
    let a0 = 0x1000u32;
    let at_val = 0x5D58_8B65u32;
    let lo = ((s6 as u64) * (at_val as u64)) as u32;

    let ra_limit: u32 = 0x100_000; // 1MB ROM region to process

    let v0 = lo + 1;
    let mut a3 = v0;
    let mut t2 = v0;
    let mut t3 = v0;
    let mut s0 = v0;
    let mut a2 = v0;
    let mut t4 = v0;
    let mut t0: u32 = 0;
    let mut t1: usize = a0 as usize;
    let t5: u32 = 32;

    loop {
        if t0 >= ra_limit {
            break;
        }

        let v0_raw = read_u32_be(&buf[t1..]);

        // v1 = a3 + v0 (with carry detection)
        let v1 = a3.wrapping_add(v0_raw);
        let carry = v1 < a3;
        if carry {
            t2 += 1;
        }

        // Rotate v0: split at bit position (v0 & 0x1F), shift right by (32 - shift) and left by shift
        let shift = v0_raw & 0x1F;
        let t7 = t5.wrapping_sub(shift); // 32 - shift
        // Handle edge case: shifting u32 by 32 is UB, treat as 0
        let t8 = if t7 >= 32 { 0u32 } else { v0_raw >> t7 }; // high bits
        let t6 = v0_raw << shift;        // low bits (overflow discarded)
        let a0_rot = t6 | t8;            // rotated value

        // XOR carry detection
        let xor_carry = a2 < v0_raw;

        // Delay slot operations
        s0 = s0.wrapping_add(a0_rot);

        // Main operations
        t3 ^= v0_raw;

        let (a2_new, _a3_tmp) = if xor_carry {
            let t9 = a3 ^ v0_raw;
            (t9 ^ a2, a3)
        } else {
            (a2 ^ a0_rot, a3)
        };
        a2 = a2_new;

        // Delay slot operations
        t0 += 4;
        let t7_xor = v0_raw ^ s0;
        t1 += 4;
        t4 = t4.wrapping_add(t7_xor);

        a3 = _a3_tmp;
    }

    // Final reduction (post-loop)
    let t6_final = a3 ^ t2;
    let result_a3 = t6_final ^ t3;
    let t8_final = s0 ^ a2;
    let result_s0 = t8_final ^ t4;

    cksum[0] = result_a3;
    cksum[1] = result_s0;
}

fn read_u32_be(buf: &[u8]) -> u32 {
    ((buf[0] as u32) << 24)
        | ((buf[1] as u32) << 16)
        | ((buf[2] as u32) << 8)
        | (buf[3] as u32)
}

fn write_u32_be(buf: &mut [u8], val: u32) {
    buf[0] = ((val >> 24) & 0xFF) as u8;
    buf[1] = ((val >> 16) & 0xFF) as u8;
    buf[2] = ((val >> 8) & 0xFF) as u8;
    buf[3] = (val & 0xFF) as u8;
}
