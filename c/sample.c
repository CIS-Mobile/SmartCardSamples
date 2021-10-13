#ifdef WIN31
#undef UNICODE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#endif

#ifdef WIN32
static char *pcsc_stringify_error(LONG rv) {
  static char out[20];
  sprintf_s(out, sizeof(out), "0x%08X", rv);

  return out;
}
#endif

#define BUF_SIZE 65538

#define CHECK(f, rv)                              \
  if (SCARD_S_SUCCESS != rv) {                    \
    printf(f ": %s\n", pcsc_stringify_error(rv)); \
    return -1;                                    \
  }

int hex_to_int(char c) {
  int first = c / 16 - 3;
  int second = c % 16;
  int result = first * 10 + second;
  if (result > 9)
    result--;
  return result;
}

int hex_to_ascii(char c, char d) {
  int high = hex_to_int(c) * 16;
  int low = hex_to_int(d);
  return high + low;
}

int main(void) {
  LONG rv;
  char hexString[BUF_SIZE];

  SCARDCONTEXT hContext;
  LPTSTR mszReaders;
  SCARDHANDLE hCard;
  DWORD dwReaders, dwActiveProtocol, dwRecvLength;

  SCARD_IO_REQUEST pioSendPci;
  BYTE pbRecvBuffer[BUF_SIZE];
  BYTE cmd[] = {0x00, 0xA4, 0x04, 0x00,				// SELECT_APDU Header
                0x07,						// Size of data following
                0xA0, 0x00, 0x00, 0x02, 0x47, 0x10, 0x01};	// AID

  unsigned int i;

  rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
  CHECK("SCardEstablishContext", rv)

#ifdef SCARD_AUTOALLOCATE
  dwReaders = SCARD_AUTOALLOCATE;

  rv = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);
  CHECK("SCardListReaders", rv)
#else
  rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
  CHECK("SCardListReaders", rv)

  mszReaders = calloc(dwReaders, sizeof(char));
  rv = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
  CHECK("SCardListReaders", rv)
#endif
  printf("reader name: %s\n", mszReaders);

  rv = SCardConnect(hContext, mszReaders, SCARD_SHARE_SHARED,
                    SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard,
                    &dwActiveProtocol);
  CHECK("SCardConnect", rv)

  switch (dwActiveProtocol) {
    case SCARD_PROTOCOL_T0:
      pioSendPci = *SCARD_PCI_T0;
      break;

    case SCARD_PROTOCOL_T1:
      pioSendPci = *SCARD_PCI_T1;
      break;
  }
  dwRecvLength = sizeof(pbRecvBuffer);
  rv = SCardTransmit(hCard, &pioSendPci, cmd, sizeof(cmd), NULL, pbRecvBuffer,
                     &dwRecvLength);
  CHECK("SCardTransmit", rv)

  // check for response code 90 00
  printf("response code: %02X %02X\n", pbRecvBuffer[dwRecvLength - 2], pbRecvBuffer[dwRecvLength - 1]);
  char responseCode[4];
  sprintf(responseCode, "%02X", pbRecvBuffer[dwRecvLength - 2]);
  sprintf(responseCode + strlen(responseCode), "%02X", pbRecvBuffer[dwRecvLength - 1]);
  if (strcmp(responseCode, "9000")) {
    printf("Card returned %s not 9000\n", responseCode);
    return 1;
  }

  printf("response: ");
  for (i = 0; i < dwRecvLength; i++)
    printf("%02X ", pbRecvBuffer[i]);
  printf("\n");
  // Write the response to a string and ignore the status code (2 bytes)
  for (i = 0; i < dwRecvLength - 2; i++)
    sprintf(hexString + strlen(hexString), "%02X", pbRecvBuffer[i]);

  int responseLength = strlen(hexString);
  char strBuf = 0;
  char responseString[BUF_SIZE];
  for (i = 0; i < responseLength; i++) {
    if (i % 2 != 0) {
      sprintf(responseString + strlen(responseString), "%c", hex_to_ascii(strBuf, hexString[i]));
    } else {
      strBuf = hexString[i];
    }
  }
  printf("%s\n", responseString);

  rv = SCardDisconnect(hCard, SCARD_LEAVE_CARD);
  CHECK("SCardDisconnect", rv)

#ifdef SCARD_AUTOALLOCATE
  rv = SCardFreeMemory(hContext, mszReaders);
  CHECK("SCardFreeMemory", rv)
#else
  free(mszReaders);
#endif

  rv = SCardReleaseContext(hContext);

  CHECK("SCardReleaseContext", rv)

  return 0;
}
