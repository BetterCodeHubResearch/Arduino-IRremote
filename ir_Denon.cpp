#include "IRremote.h"
#include "IRremoteInt.h"

// Reverse Engineerd by looking at RAW dumps generated by IRremote

//==============================================================================
//                    DDDD   EEEEE  N   N   OOO   N   N
//                     D  D  E      NN  N  O   O  NN  N
//                     D  D  EEE    N N N  O   O  N N N
//                     D  D  E      N  NN  O   O  N  NN
//                    DDDD   EEEEE  N   N   OOO   N   N
//==============================================================================

#define BITS          14  // The number of bits in the command

#define HDR_MARK     300  // The length of the Header:Mark
#define HDR_SPACE    750  // The lenght of the Header:Space

#define BIT_MARK     300  // The length of a Bit:Mark
#define ONE_SPACE   1800  // The length of a Bit:Space for 1's
#define ZERO_SPACE   750  // The length of a Bit:Space for 0's

//+=============================================================================
//
#if SEND_DENON
void  IRsend::sendDenon (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark (HDR_MARK);
	space(HDR_SPACE);

	// Data
	for (unsigned long  mask = 1 << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark (BIT_MARK);
			space(ONE_SPACE);
		} else {
			mark (BIT_MARK);
			space(ZERO_SPACE);
		}
	}

	// Footer
	mark(BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
//
#if DECODE_DENON
bool  IRrecv::decodeDenon (decode_results *results)
{
	unsigned long  data   = 0;  // Somewhere to build our code
	int            offset = 1;  // Skip the Gap reading

	// Check we have the right amount of data
	if (irparams.rawlen != 1 + 2 + (2 * BITS) + 1)  return false ;

	// Check initial Mark+Space match
	if (!MATCH_MARK (results->rawbuf[offset++], HDR_MARK ))  return false ;
	if (!MATCH_SPACE(results->rawbuf[offset++], HDR_SPACE))  return false ;

	// Read the bits in
	for (int i = 0;  i < BITS;  i++) {
		// Each bit looks like: MARK + SPACE_1 -> 1
		//                 or : MARK + SPACE_0 -> 0
		if (!MATCH_MARK(results->rawbuf[offset++], BIT_MARK))  return false ;

		// IR data is big-endian, so we shuffle it in from the right:
		if      (MATCH_SPACE(results->rawbuf[offset], ONE_SPACE))   data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], ZERO_SPACE))  data = (data << 1) | 0 ;
		else                                                        return false ;
		offset++;
	}

	// Success
	results->bits        = BITS;
	results->value       = data;
	results->decode_type = DENON;
	return true;
}
#endif
