/*
    File: iGetKeys.c
    
    Description:
       Internationally Savy GetKeys test type routines for your entertainment
       and enjoyment.

    
    Disclaimer:
        IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
        ("Apple") in consideration of your agreement to the following terms, and your
        use, installation, modification or redistribution of this Apple software
        constitutes acceptance of these terms.  If you do not agree with these terms,
        please do not use, install, modify or redistribute this Apple software.

        In consideration of your agreement to abide by the following terms, and subject
        to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
        reproduce, modify and redistribute the Apple Software, with or without
        modifications, in source and/or binary forms; provided that if you redistribute
        the Apple Software in its entirety and without modifications, you must retain
        this notice and the following text and disclaimers in all such redistributions of
        the Apple Software.  Neither the name, trademarks, service marks or logos of
        Apple Computer, Inc. may be used to endorse or promote products derived from the
        Apple Software without specific prior written permission from Apple.  Except as
        expressly stated in this notice, no other rights or licenses, express or implied,
        are granted by Apple herein, including but not limited to any patent rights that
        may be infringed by your derivative works or by other works in which the Apple
        Software may be incorporated.

        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
        WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
        WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
        PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
        COMBINATION WITH YOUR PRODUCTS.

        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
        ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
        OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
        (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
        ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Change History (most recent first):
        Mon, Jan 15, 2001 -- created
*/


#include "iGetKeys.h"

#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
//#include <Events.h>
//#include <Script.h>
//#include <Resources.h>
#include <StdArg.h>
#endif




enum {
	kTableCountOffset = 256+2,
	kFirstTableOffset = 256+4,
	kTableSize = 128
};

OSStatus InitAscii2KeyCodeTable(Ascii2KeyCodeTable *ttable) {
	unsigned char *theCurrentKCHR, *ithKeyTable;
	short count, i, j, resID;
	Handle theKCHRRsrc;
        ResType rType;
		/* set up our table to all minus ones */
	for (i=0;i<256; i++) ttable->transtable[i] = -1;
		/* find the current kchr resource ID */
	ttable->kchrID = (short) GetScriptVariable(smCurrentScript, smScriptKeys);
		/* get the current KCHR resource */
	theKCHRRsrc = GetResource('KCHR', ttable->kchrID);
	if (theKCHRRsrc == NULL) return resNotFound;
        GetResInfo(theKCHRRsrc,&resID,&rType,ttable->KCHRname);
		/* dereference the resource */
	theCurrentKCHR = (unsigned char *)  (*theKCHRRsrc);
		/* get the count from the resource */
	count = * (short *) (theCurrentKCHR + kTableCountOffset);
		/* build inverse table by merging all key tables */
	for (i=0; i<count; i++) {
		ithKeyTable = theCurrentKCHR + kFirstTableOffset + (i * kTableSize);
		for (j=0; j<kTableSize; j++) {
			if ( ttable->transtable[ ithKeyTable[j] ] == -1)
				ttable->transtable[ ithKeyTable[j] ] = j;
		}
	}
		/* all done */
	return noErr;
}


OSStatus ValidateAscii2KeyCodeTable(Ascii2KeyCodeTable *ttable,Boolean *wasChanged) {
	short theID;
	theID = (short) GetScriptVariable(smCurrentScript, smScriptKeys);
	if (theID != ttable->kchrID)
        {
            *wasChanged=true;
            return InitAscii2KeyCodeTable(ttable);
        }
	else
        {
            *wasChanged=false;
            return noErr;
        }
}

StringPtr GetKchrName(Ascii2KeyCodeTable *ttable) {
    return ttable->KCHRname;
}


short AsciiToKeyCode(Ascii2KeyCodeTable *ttable, short asciiCode) {
	if (asciiCode >= 0 && asciiCode <= 255)
		return ttable->transtable[asciiCode];
	else return false;
}

char KeyCodeToAscii(short virtualKeyCode) {
	static unsigned long state = 0;
	long keyTrans;
	char charCode;
	Ptr kchr;
	kchr = (Ptr) GetScriptVariable(smCurrentScript, smKCHRCache);
	keyTrans = KeyTranslate(kchr, virtualKeyCode, &state);
	charCode = keyTrans;
	if (!charCode) charCode = (keyTrans>>16);
	return charCode;
}

Boolean TestForKeyDown(short virtualKeyCode) {
	KeyMap theKeys;
	unsigned char *keybytes;
	if (virtualKeyCode >= 0 && virtualKeyCode <= 127) {
		GetKeys(theKeys);
		keybytes = (unsigned char *) theKeys;
		return ((keybytes[virtualKeyCode>>3] & (1 << (virtualKeyCode&7))) != 0);
	} else return false;
}

Boolean TestForMultipleKeysDown(short nkeys, ...) {
	KeyMap theKeys;
	va_list args;
	long i, virtualKeyCode;
	Boolean product;
	unsigned char *keybytes;
	va_start(args, nkeys);
	if (nkeys == 0) {
		product = false;
	} else {
		product = true;
		GetKeys(theKeys);
		keybytes = (unsigned char *) theKeys;
		for (i=0; i<nkeys; i++) {
			virtualKeyCode = va_arg(args, long);
			if (virtualKeyCode < 0 || virtualKeyCode > 127
			|| (keybytes[virtualKeyCode>>3] & (1 << (virtualKeyCode&7))) == 0) {
				product = false;
				break;
			}
		}
	}
	va_end(args);
	return product;
}


Boolean TestForArrayOfKeysDown(short nkeys, short *keyvec) {
	KeyMap theKeys;
	long i, virtualKeyCode;
	Boolean product;
	unsigned char *keybytes;
	if (nkeys == 0) {
		product = false;
	} else {
		product = true;
		GetKeys(theKeys);
		keybytes = (unsigned char *) theKeys;
		for (i=0; i<nkeys; i++) {
			virtualKeyCode = keyvec[i];
			if (virtualKeyCode < 0 || virtualKeyCode > 127
			|| (keybytes[virtualKeyCode>>3] & (1 << (virtualKeyCode&7))) == 0) {
				product = false;
				break;
			}
		}
	}
	return product;
}


Boolean TestForAsciiKeyDown(Ascii2KeyCodeTable *ttable, short asciiCode) {
	short virtualKeyCode;
	if (asciiCode < 0 || asciiCode > 255
	|| (virtualKeyCode = ttable->transtable[asciiCode]) == -1)
		return false;
	else return TestForKeyDown(virtualKeyCode);
}


Boolean TestForMultipleAsciiKeysDown(Ascii2KeyCodeTable *ttable, short nkeys, ...) {
	KeyMap theKeys;
	va_list args;
	long i, virtualKeyCode, asciiCode;
	Boolean product;
	unsigned char *keybytes;
	va_start(args, nkeys);
	if (nkeys == 0) {
		product = false;
	} else {
		product = true;
		GetKeys(theKeys);
		keybytes = (unsigned char *) theKeys;
		for (i=0; i<nkeys; i++) {
			asciiCode = va_arg(args, long);
			if (asciiCode < 0 || asciiCode > 255)  {
				product = false;
				break;
			} else {
				virtualKeyCode = ttable->transtable[asciiCode];
				if (virtualKeyCode == -1
				|| (keybytes[virtualKeyCode>>3] & (1 << (virtualKeyCode&7))) == 0) {
					product = false;
					break;
				}
			}
		}
	}
	va_end(args);
	return product;
}

Boolean TestForArrayOfAsciiKeysDown(Ascii2KeyCodeTable *ttable, short nkeys, unsigned char* asciiVec) {
	KeyMap theKeys;
	long i, virtualKeyCode, asciiCode;
	Boolean product;
	unsigned char *keybytes;
	if (nkeys == 0) {
		product = false;
	} else {
		product = true;
		GetKeys(theKeys);
		keybytes = (unsigned char *) theKeys;
		for (i=0; i<nkeys; i++) {
			asciiCode = asciiVec[i];
			if (asciiCode < 0 || asciiCode > 255)  {
				product = false;
				break;
			} else {
				virtualKeyCode = ttable->transtable[asciiCode];
				if (virtualKeyCode == -1
				|| (keybytes[virtualKeyCode>>3] & (1 << (virtualKeyCode&7))) == 0) {
					product = false;
					break;
				}
			}
		}
	}
	return product;
}


