arcfour16-drop786432-middle8
============================

arcfour16-drop786432-middle8 is a minimalistic encryption
utility, implementing a 16-bit analogue of the 8-bit ARCFOUR
algorithm.

In fact it uses exactly the same algorithm, except that all
variables are 16 bits wide except of 8 bits, and the SBOX has
2**16 instead of 2**8 entries.

Usage
-----

	arcfour16-drop786432-middle8 key_file < infile > outfile

is the basic usage, but more practically

	$ { LC_ALL=C date; dd if=/dev/urandom bs=1 count=8 2> /dev/null \
	; } > nonce.bin \
	; cat nonce.bin pass_phrase.txt nonce.bin > key_file.bin \
	; arcfour16-drop786432-middle8 key_file.bin < infile > outfile

where key_file is the name of a file containing the
encryption/decryption key.

When run without any arguments, the above commands will be
displayed as part of the error message for the missing argument.
So your don't need to remember them.

The key can be up to 128 KiB in size, and every byte of it will
be significant (have an effect on the encryption).

This means very long keys are supported and will be honored as
such.

The binary key file containing the key can be longer than the
maximum key size, but no more than its first 128 KiB will be read
from it as the key.


Mode of operation
-----------------

There is no difference between encryption and decryption: If the
input is already encrypted it will be decrypted, otherwise it
will be encrypted.


Implementation details
----------------------

The encryption key is interpreted as binary data. But text works
as well, because everything including text can be interpreted as
binary data.

The binary data of the encryption key will be read as a sequence
of 16-bit units from the key file in big-endian byte order (pairs
of high-order byte, low-order byte).

As a convenience, if the key file is not actually made of an even
number of bytes, it will be processed as if another byte with
value zero would have been appended to its end.

ARCFOUR works by creating a pseudorandom stream which is
dependent on the key and uses the generated pseudorandom data to
encrypt/decrypt the input data.

Analyses have shown that the initial pseudorandom bytes generated
by ARCFOUR are not "random" enough, a fact which has allowed some
attacks against the original unmodified ARCFOUR algorithm.

In order to eliminate those problems, SCAN recommends that at
least the first 768 pseudorandom bytes should be dropped, or even
3072 bytes to be more conservative. (The latter being 12 times
the size of the SBOX.)

arcfour16-drop786432-middle8 follows that recommendation,
dropping the first (12 times SBOX size) 16-bit pseudorandom
units. Only the remaining pseudorandom units are actually used
for encryption. This means the first 786432 generated 16-bit
pseudorandom units will be dropped.

As an additional safety measure, arcfour16-drop786432-middle8
only uses the middle 8 bits of every generated 16-bit output for
actual encryption.

This also exposes less of the internal state to attackers for
analysis.


Security
--------

The internal state of ARCFOUR consists of a counter and the
SBOX contents.

The SBOX always contains a permutation of all possible integer
values that fit into an SBOX entry.

The initial contents of both SBOX and the counter are derived
from the key by a similar operation like the actual encryption.

The number of possible internal states of ARCFOUR is therefore
the number of possible counter values times the number of
possible SBOX permutations.

If the number of bits needed to represent the number of possible
internal states of ARCFOUR is considered its "maximal security",
then the following figures apply to ARCFOUR:

ARCFOUR-8: floor(log2(256 * 256!)) = 1691 bit  
ARCFOUR-16: floor(log2(65536 * 65536!)) = ??? bit

Unfortunately, my CAS software was not able to calculate the
maximal security fot ARCFOUR-16 because the numbers got too big.
Anyway, it is *huge*.

However, there is no guarantee that all possible internal states
will actually be generated durung operation: Depending on the
key, there could be loops through which the state cycles.

But this has not be an problem with ARCFOUR-8 so far, and will
even be less likely a problem with ARCFOUR-16 because of its much
larger internal state.

Most security problems with ARCFOUR have come from improper usage
or weak keys; see the next section how to avoid this.

-----

See the source file for version and copyright information.

The UUID of this project is 35944240-73a2-11e7-86f3-b827eb0d201c.
