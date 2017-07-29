/* Implementation of ARCFOUR extended to 16 bits. */
static char const help_usage[]= {
   /* Maintain the special indentation of the following block for a
    * target line width of 66 columns. */
         /* SPECIAL INDENDATION START { */
         "This program reads from standard input, encrypts/decrypts it, and\n"
         "writes the result to standard output.\n"
         "\n"
         "A single argument is required: A file containing the binary\n"
         "encryption key.\n"
         "\n"
         "This key file must not be used for encrypting more than one\n"
         "single message, or an attacker might be able to crack all\n"
         "messages encrypted with that key file.\n"
         "\n"
         "In order to allow re-using the same password for multiple\n"
         "messages safely, create this key file as follows before sending\n"
         "every message:\n"
         "\n"
         "$ { LC_ALL=C date; dd if=/dev/urandom bs=1 count=8 2> /dev/null \\\n"
         "; } > nonce.bin \\\n"
         "; cat nonce.bin pass_phrase.txt nonce.bin > key_file.bin\n"
         "\n"
         "In this example, 'key_file.bin' will be the name of the binary\n"
         "key file which has been created by the commands.\n"
         "\n"
         "Then use this key file to encrypt the message and send it\n"
         "together with 'nonce.bin' to the receiver, who must already have\n"
         "your secret password or -phrase in 'pass_phrase.txt'.\n"
         "\n"
         "The receiver then uses the last line of the above multi-line\n"
         "command sequence in order to recreate 'key_file.bin' for\n"
         "decryption of the current message.\n"
         "\n"
         "Encryption and decryption work exactly the same with this\n"
         "program. It will always know what you want and automatically do\n"
         "the right thing.\n"
         "\n"
         "Bug: There is no integrity protection at all. An active attacker\n"
         "can change any part of the encrypted file and you will never\n"
         "know. When in doubt, send a cryptographically signed\n"
         "cryptographic hash of the encrypted file along with it.\n"
         /* } SPECIAL INDENDATION END */
   /* End of specially indented text. */
   "\n"
   "version 2017.210\n"
   "\n"
   "Copyright (c) 2017 Guenther Brunthaler. All rights reserved.\n"
   "\n"
   "This program is free software.\n"
   "Distribution is permitted under the terms of the GPLv3."
};

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#define DIM(array) (sizeof(array) / sizeof *(array))

#define SWAP(type, var1, var2) { \
   type tmp= var1; \
   var1= var2; \
   var2= tmp; \
}

int main(int argc, char **argv) {
   struct {
      unsigned keybuf: 1;
      unsigned kfile: 1;
      unsigned needs2run: 1;
      unsigned flushed: 1;
   } have= {0};
   char const *error= 0;
   FILE *kfile;
   static unsigned short sbox[1 << 16];
   unsigned short (*keybuf)[DIM(sbox)];
   unsigned i, j;
   if (argc != 2) {
      error= help_usage;
      fail:
      (void)fputs(error, stderr);
      (void)putc('\n', stderr);
      goto cleanup;
   }
   if (!(keybuf= malloc(sizeof *keybuf))) {
      error= "Memory allocation failure!";
      goto fail;
   }
   have.keybuf= 1;
   if (!(kfile= fopen(argv[1], "rb"))) {
      error= "Could not open key file!";
      goto fail;
   }
   have.kfile= 1;
   #define SBOX_MASK ((unsigned short)(DIM(sbox) - 1))
   /* Read the key into memory. */
   {
      unsigned keylength;
      for (keylength= 0; keylength < DIM(*keybuf); ++keylength) {
         int low, high;
         if ((high= fgetc(kfile)) != EOF) {
            /* Big endian rulez: Important things should always come first! */
            if ((low= fgetc(kfile)) == EOF) {
               if (ferror(kfile)) {
                  krerr:
                  error= "Error reading key file!";
                  goto fail;
               }
               assert(feof(kfile));
               /* Silently pad the binary key with one zero byte rather than
                * complaining about it being not a multiple of 16 bits. */
               low= 0;
            }
            (*keybuf)[keylength]=
                  (unsigned short)((unsigned)high << 8)
               |  (unsigned short)(unsigned)low
            ;
         } else {
            if (ferror(kfile)) goto krerr;
            assert(feof(kfile));
            break;
         }
      }
      /* Run key setup. */
      for (i= DIM(sbox); i--; ) sbox[i]= (unsigned short)i;
      j= 0;
      {
         unsigned imodklen;
         for (i= imodklen= 0; i < DIM(sbox); ++i) {
            j= j + sbox[i] + (*keybuf)[imodklen] & SBOX_MASK;
            SWAP(unsigned short, sbox[i], sbox[j]);
            if (++imodklen == keylength) imodklen= 0;
         }
      }
      have.needs2run= 1;
   }
   cleanup:
   if (have.kfile) {
      have.kfile= 0;
      if (fclose(kfile)) {
         error= "Internal error. Should not normally occur.";
         goto fail;
      }
   }
   if (have.keybuf) { have.keybuf= 0; free(keybuf); }
   if (have.needs2run) {
      have.needs2run= 0;
      i= j= 0;
      #define DO_ROUND do { \
         i= i + 1 & SBOX_MASK; \
         j= sbox[i] + j & SBOX_MASK; \
         SWAP(unsigned short, sbox[i], sbox[j]); \
      } while (0)
      {
         unsigned drop;
         for (drop= 786432; drop--; ) DO_ROUND;
      }
      {
         int c;
         while ((c= getchar()) != EOF) {
            DO_ROUND;
            c^= (int)(
               sbox[sbox[i] + sbox[j] & SBOX_MASK] >> (16 - 8 >> 1) & UCHAR_MAX
            );
            if (putchar(c) != c) {
               assert(ferror(stdout));
               error= "Error writing to standard output!";
               goto fail;
            }
         }
         if (ferror(stdin)) {
            error= "Error reading standard input!";
            goto fail;
         }
         assert(feof(stdin));
      }
      #undef DO_ROUND
   }
   #undef SBOX_MASK
   if (!have.flushed) {
      have.flushed= 1;
      if (fflush(0)) {
         error= "Error writing remaining output.";
         goto fail;
      }
   }
   return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
