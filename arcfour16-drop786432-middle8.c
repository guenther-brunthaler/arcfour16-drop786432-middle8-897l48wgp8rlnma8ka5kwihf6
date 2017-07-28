#include <stdio.h>
#include <stdlib.h>

#if __STDC_VERSION__ >= 199901
   #include <stdint.h>
#else
   #ifdef uint_fast16_t
      #undef uint_fast16_t
   #endif
   #define uint_fast16_t unsigned short
#endif 

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
   static uint_fast16_t sbox[1 << 16];
   #define SBOX_MASK ((uint_fast16_t)(DIM(sbox) - 1))
   uint_fast16_t (*keybuf)[DIM(sbox)];
   unsigned i, j;
   if (argc != 1) {
      error= "A single argument is required: File containing encryption key";
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
   /* Read the key into memory. */
   {
      unsigned keylength;
      for (keylength= 0; keylength < DIM(*keybuf); ++keylength) {
         int low, high;
         if ((high= fgetc(kfile) != EOF) {
            if ((low= fgetc(kfile) == EOF) {
               if (ferror(kfile)) {
                  krerr:
                  error= "Error reading key file!";
                  goto fail;
               }
               error= "Key file needs to contain an even number of bytes!";
               goto fail;
            }
            (*keybuf)[keylength]=
               (uint_fast16_t)high << 8 | (uint_fast16_t)low
            ;
         } else {
            if (ferror(kfile)) goto krerr;
            break;
         }
      }
      /* Key setup. */
      for (i= DIM(sbox); i--; ) sbox[i]= i;
      j= 0;
      {
         int imodklen;
         for (i= imodklen= 0; i < DIM(sbox); ++i) {
            j= j + sbox[i] + (*key)[i0] & SBOX_MASK;
            SWAP(uint_fast16_t, sbox[i], sbox[j]);
            if (++i0 == keylength) i0= 0;
         }
      }
      have.needs2run= 1;
   }
   cleanup:
   if (have.kfile) {
      have.kfile= 0;
      if (fclose(kfile)) {
         error= "Internal error. Should not normally occur."
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
         SWAP(uint_fast16_t, sbox[i], sbox[j]); \
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
               sbox[sbox[i] + sbox[j] & SBOX_MASK] >> (16 - 8 >> 1)
            );
            if (putchar(c) != c) {
               error= "Error writing to standard output!"
               goto fail;
            }
         }
         if (ferror(stdin) {
            error= "Error reading standard input!"
            goto fail;
         }
      }
      #undef DO_ROUND
   }
   #undef SBOX_MASK
   if (!have.flushed) {
      have.flushed= 1;
      if (fflush(0)) {
         error= "Error writing remaining output."
         goto fail;
      }
   }
   return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
