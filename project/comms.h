#include <stdbool.h>
#include <stdint.h>


struct Comms;

struct PokePacket {
	uint8_t cmd;
	uint8_t detail;
	uint8_t crc[2];
	uint8_t session[4];
	//data here
};

struct Comms* commsOpen(void);
void commsClose(struct Comms *comms);

bool commsTryAcceptWalkerConnection(struct Comms *comms);
bool commsDisconnect(struct Comms *comms);					//some other commands disconnect as is, they will be noted

bool commsEepromRead(struct Comms *comms, void *dst, uint16_t addr, uint16_t len);
bool commsEepromWrite(struct Comms *comms, const void *src, uint16_t addr, uint16_t len);





//triggers
bool commsEventPokeRxed(struct Comms *comms);				//implies disconnect
bool commsEventItemRxed(struct Comms *comms);				//implies disconnect
bool commsEventRouteRxed(struct Comms *comms);				//not yet understood. implies disconnect

//scary low level things
bool commsPacketRawTx(struct Comms *comms, struct PokePacket *pkt, const void *data, uint8_t len);
int16_t commsPacketRawRx(struct Comms *comms, struct PokePacket *pkt, void *buf, uint8_t rxBufSz);
bool commsDeviceReset(struct Comms *comms);					//implies disconnect

