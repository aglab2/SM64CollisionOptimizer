#include <stdlib.h>
#include <stdio.h>

#define N64CKSUM_VERSION "0.1"

#define MB 1024*1024

static void print_usage(void)
{
   printf("Usage: n64cksum ROM [ROM_OUT]\n"
         "\n"
         "n64cksum v" N64CKSUM_VERSION ": N64 ROM checksum calculator\n"
         "\n"
         "File arguments:\n"
         " ROM          input ROM file\n"
         " ROM_OUT      output ROM file (default: overwrites input ROM)\n");
}


long read_file(const char *file_name, unsigned char **data)
{
   FILE *in;
   unsigned char *in_buf = NULL;
   long file_size;
   long bytes_read;
   in = fopen(file_name, "rb");
   if (in == NULL) {
      return -1;
   }

   // allocate buffer to read from offset to end of file
   fseek(in, 0, SEEK_END);
   file_size = ftell(in);

   // sanity check
   if (file_size > 256*MB) {
      return -2;
   }

   in_buf = malloc(file_size);
   fseek(in, 0, SEEK_SET);

   // read bytes
   bytes_read = fread(in_buf, 1, file_size, in);
   if (bytes_read != file_size) {
      return -3;
   }

   fclose(in);
   *data = in_buf;
   return bytes_read;
}

long write_file(const char *file_name, unsigned char *data, long length)
{
   FILE *out;
   long bytes_written;
   // open output file
   out = fopen(file_name, "wb");
   if (out == NULL) {
      perror(file_name);
      return -1;
   }
   bytes_written = fwrite(data, 1, length, out);
   fclose(out);
   return bytes_written;
}

#define read_u32_be(buf) (unsigned int)(((buf)[0] << 24) + ((buf)[1] << 16) + ((buf)[2] << 8) + ((buf)[3]))
#define write_u32_be(buf, val) do { \
   (buf)[0] = ((val) >> 24) & 0xFF; \
   (buf)[1] = ((val) >> 16) & 0xFF; \
   (buf)[2] = ((val) >> 8) & 0xFF; \
   (buf)[3] = (val) & 0xFF; \
} while(0)

// compute N64 ROM checksums
// buf: buffer with extended SM64 data
// cksum: two element array to write CRC1 and CRC2 to
// TODO: this could be hand optimized
static void sm64_calc_checksums(unsigned char *buf, unsigned int cksum[]) {
   unsigned int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
   unsigned int s0, s6;
   unsigned int a0, a1, a2, a3, at;
   unsigned int lo;
   unsigned int v0, v1;
   unsigned int ra;

   // derived from the SM64 boot code
   s6 = 0x3f;
   a0 = 0x1000;     // 59c:   8d640008    lw a0,8(t3)
   a1 = s6;         // 5a0:   02c02825    move  a1,s6
   at = 0x5d588b65; // 5a4:   3c015d58    lui   at,0x5d58
                    // 5a8:   34218b65    ori   at,at,0x8b65
   lo = a1 * at;    // 5ac:   00a10019    multu a1,at    16 F8CA 4DDB

   ra = 0x100000; // 5bc:  3c1f0010    lui   ra,0x10
   v1 = 0;  // 5c0:  00001825    move  v1,zero
   t0 = 0;  // 5c4:  00004025    move  t0,zero
   t1 = a0; // 5c8:  00804825    move  t1,a0
   t5 = 32; // 5cc:  240d0020    li t5,32
   v0 = lo; // 5d0:  00001012    mflo  v0
   v0++;    // 5d4:  24420001    addiu v0,v0,1
   a3 = v0; // 5d8:  00403825    move  a3,v0
   t2 = v0; // 5dc:  00405025    move  t2,v0
   t3 = v0; // 5e0:  00405825    move  t3,v0
   s0 = v0; // 5e4:  00408025    move  s0,v0
   a2 = v0; // 5e8:  00403025    move  a2,v0
   t4 = v0; // 5ec:  00406025    move  t4,v0

   do {
      v0 = read_u32_be(&buf[t1]);   // 5f0: 8d220000    lw v0,0(t1)
      v1 = a3 + v0;   // 5f4: 00e21821    addu  v1,a3,v0
      at = (v1 < a3); // 5f8: 0067082b    sltu  at,v1,a3
      a1 = v1;        // 600: 00602825    move  a1,v1 branch delay slot
      if (at) {       // 5fc: 10200002    beqz  at,0x608
         t2++;        // 604: 254a0001    addiu t2,t2,1
      }
      v1 = v0 & 0x1F;  // 608: 3043001f    andi  v1,v0,0x1f
      t7 = t5 - v1;    // 60c: 01a37823    subu  t7,t5,v1
      t8 = v0 >> t7;   // 610: 01e2c006    srlv  t8,v0,t7
      t6 = v0 << v1;   // 614: 00627004    sllv  t6,v0,v1
      a0 = t6 | t8;    // 618: 01d82025    or a0,t6,t8
      at = (a2 < v0);  // 61c: 00c2082b    sltu  at,a2,v0
      a3 = a1;         // 620: 00a03825    move  a3,a1
      t3 ^= v0;        // 624: 01625826    xor   t3,t3,v0
      s0 += a0;        // 62c: 02048021    addu  s0,s0,a0 branch delay slot
      if (at) {        // 628: 10200004    beqz  at,0x63c
         t9 = a3 ^ v0; // 630: 00e2c826    xor   t9,a3,v0
                       // 634: 10000002    b  0x640
         a2 ^= t9;     // 638: 03263026    xor   a2,t9,a2 branch delay
      } else {
         a2 ^= a0;     // 63c: 00c43026    xor   a2,a2,a0
      }
      t0 += 4;         // 640: 25080004    addiu t0,t0,4
      t7 = v0 ^ s0;    // 644: 00507826    xor   t7,v0,s0
      t1 += 4;         // 648: 25290004    addiu t1,t1,4
      t4 += t7;        // 650: 01ec6021    addu  t4,t7,t4 branch delay
   } while (t0 != ra); // 64c: 151fffe8    bne   t0,ra,0x5f0
   t6 = a3 ^ t2;       // 654: 00ea7026    xor   t6,a3,t2
   a3 = t6 ^ t3;       // 658: 01cb3826    xor   a3,t6,t3
   t8 = s0 ^ a2;       // 65c: 0206c026    xor   t8,s0,a2
   s0 = t8 ^ t4;       // 660: 030c8026    xor   s0,t8,t4
   
   cksum[0] = a3;
   cksum[1] = s0;
}

void sm64_update_checksums(unsigned char *buf)
{
   unsigned int cksum_offsets[] = {0x10, 0x14};
   unsigned int read_cksum[2];
   unsigned int calc_cksum[2];
   int i;

   // calculate new N64 header checksum
   sm64_calc_checksums(buf, calc_cksum);

   // mimic the n64sums output
   for (i = 0; i < 2; i++) {
      read_cksum[i] = read_u32_be(&buf[cksum_offsets[i]]);
      printf("CRC%d: 0x%08X ", i+1, read_cksum[i]);
      printf("Calculated: 0x%08X ", calc_cksum[i]);
      if (calc_cksum[i] == read_cksum[i]) {
         printf("(Good)\n");
      } else {
         printf("(Bad)\n");
      }
   }

   // write checksums into header
   printf("Writing back calculated Checksum\n");
   write_u32_be(&buf[cksum_offsets[0]], calc_cksum[0]);
   write_u32_be(&buf[cksum_offsets[1]], calc_cksum[1]);
}

int main(int argc, char *argv[])
{
   unsigned char *rom_data;
   char *file_in;
   char *file_out;
   long length;
   long write_length;
   if (argc < 2) {
      print_usage();
      return 1;
   }

   file_in = argv[1];
   if (argc > 2) {
      file_out = argv[2];
   } else {
      file_out = argv[1];
   }

   length = read_file(file_in, &rom_data);
   if (length < 0) {
      printf("Error reading input file \"%s\"\n", file_in);
      return 1;
   }

   sm64_update_checksums(rom_data);

   write_length = write_file(file_out, rom_data, length);

   free(rom_data);

   if (write_length != length) {
      printf("Error writing to output file \"%s\"\n", file_out);
      return 1;
   }

   return 1;
}