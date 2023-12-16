#include <PalmOS.h>
#include "comms.h"
#include "res.h"


#define POKEWALKER_BAUDRATE				115200

typedef UInt16 SerialPort;

struct Comms {
	uint8_t session[4];
	SerialPort sp;
	bool connected;
};



#define POKEWALKER_DATA_MAX_LEN			128		//data per packet max, else we overflow pokewalker's buffer
#define POKEWALKER_CRC_START			0x0002
#define POKEWALKER_KEY					0xAA

#define DETAIL_DIR_TO_WALKER			0x01
#define DETAIL_DIR_FROM_WALKER			0x02


#define CMD_POKEWALKER_ADV				0xfc
#define CMD_POKEWALKER_SYN				0xfa
#define CMD_POKEWALKER_SYNACK			0xf8
#define CMD_POKEWALKER_DIS_REQ			0x66
#define CMD_POKEWALKER_DIS_RSP			0x68
#define CMD_EEPROM_READ_REQ				0x0c
#define CMD_EEPROM_READ_RSP				0x0e
#define CMD_EEPROM_WRITE_REQ			0x0a
#define CMD_EEPROM_WRITE_RSP			0x04
#define CMD_EVENT_POKE_RXED				0xc2
#define CMD_EVENT_ITEM_RXED				0xc4
#define CMD_EVENT_ROUTE_RXED			0xc6
#define CMD_WRITE						0x06










static void commsPrvError(Err err, const char *str)
{
	char x[16];
	
	x[0] = ' ';
	x[1] = '(';
	x[2] = '0';
	x[3] = 'x';
	StrIToH(x + 4, err);
	StrCat(x,")");

	FrmCustomAlert(ALERT_ID_ERROR, str, err ? x : "", "");
}

//serial port abstraction
static bool commsPrvSerialOpen(SerialPort *spP)
{
	uint32_t val, bufSz = 0xd000;
	uint16_t len;
	void *buf;
	Err e;
	
	//open port at proper baud rate
	e = SrmOpen(serPortIrPort, POKEWALKER_BAUDRATE, spP);
	if (e != errNone) {
		commsPrvError(e, "Cannot open the serial port");
		return false;
	}
	
	//config 8n1, no flow control
	len = sizeof(val);
	val = srmSettingsFlagStopBits1 | srmSettingsFlagBitsPerChar8;
	e = SrmControl(*spP, srmCtlSetFlags, &val, &len);
	if (e != errNone) {
		commsPrvError(e, "Cannot configure the serial port");
		(void)SrmClose(*spP);
		return false;
	}
	
	e = SrmControl(*spP, srmCtlIrDAEnable, NULL, NULL);
	if (e != errNone) {
		commsPrvError(e, "Cannot configure the IR port 1");
		(void)SrmClose(*spP);
		return false;
	}

	buf = MemPtrNew(bufSz);
	if (!buf)
		commsPrvError(0, "Cannot allocate a biger RX buffer");
	else {
		FtrSet('wPkr', 0, (UInt32)buf);
		SrmSetReceiveBuffer(*spP, buf, bufSz);
	}
	(void)SrmReceiveFlush(*spP, 0);
	
	return true;
}

static void commsPrvSerialClose(SerialPort sp)
{
	void *buf;
	
	SrmSetReceiveBuffer(sp, NULL, 0);
	
	
	if (errNone == FtrGet('wPkr', 0, (UInt32*)&buf) && buf)
		MemPtrFree(buf);
	FtrUnregister('wPkr', 0);

	(void)SrmClose(sp);
}

static int16_t commsPrvSerialRx(SerialPort sp, void* dstP, uint16_t expectedLen, uint16_t timeout)
{
	uint8_t *dst = (uint8_t*)dstP;
	uint16_t i, now;
	Err err;
	
	err = SrmControl(sp, srmCtlRxEnable, NULL, NULL);
	if (err != errNone)
		return -1;
	
	now = SrmReceive(sp, dstP, expectedLen, timeout, &err);
	if (dst[0] >= 0xfe) {		//no packet begins with 0x55 or 0x54
		MemMove(dst + 0, dst + 1, expectedLen - 1);
		(void)SrmReceive(sp, dst + expectedLen - 1, 1, 2, &err);	//crc will tell if it happened
	}
	
	for (i = 0; i < now; i++)
		dst[i] ^= POKEWALKER_KEY;
	
	return now;
}

static bool commsPrvSendPiece(SerialPort sp, const void *data, uint8_t len)
{
	const uint8_t *ptr = (const uint8_t*)data;
	uint8_t txBuf[64], i;
	Err err;
	
	if (!len)
		return true;

	while (len) {
	
		uint32_t now;
		
		now = len;
		if (now > sizeof(txBuf))
			now = sizeof(txBuf);
		
		for (i = 0; i < now; i++)
			txBuf[i] = ptr[i] ^ POKEWALKER_KEY;
		
		now = SrmSend(sp, txBuf, now, &err);
		if (err != errNone)
			return false;

		ptr += now;
		len -= now;
	}
	
	return true;
}

static bool commsPrvSendCompleted(SerialPort sp)
{
	(void)SrmSendWait(sp);
	(void)SrmReceiveFlush(sp, 0);
	
	return true;
}


//very low level
static uint16_t commsPrvCrc(const void *data, uint8_t len, uint16_t crcStart)
{
	const uint8_t *src = (const uint8_t*)data;
	uint32_t crc = crcStart;
	uint8_t i;
	
	for (i = 0; i < len; i++) {
		uint16_t v = src[i];
		
		if (!(i & 1))
			v <<= 8;
		
		crc += v;
	}
	
	while (crc >> 16)
		crc = (uint16_t)crc + (crc >> 16);
	
	return crc;
}

static uint16_t commsPrvChecksumPacket(const struct PokePacket *pkt, const void *data, uint8_t len)		//assumes pkt->crc == 0
{
	return commsPrvCrc(data, len, commsPrvCrc(pkt, sizeof(struct PokePacket), POKEWALKER_CRC_START));
}

static void commsPrvChecksumPacketAndRecordSum(struct PokePacket *pkt, const void *data, uint8_t len)
{
	uint16_t crc;
	
	pkt->crc[0] = 0;
	pkt->crc[1] = 0;
	
	crc = commsPrvChecksumPacket(pkt, data, len);
	
	pkt->crc[0] = crc;
	pkt->crc[1] = crc >> 8;
}

static bool commsPrvChecksumPacketAndCheck(struct PokePacket *pkt, const void *data, uint8_t len)
{
	uint16_t calcedCrc, rxedCrc = (((uint16_t)pkt->crc[1]) << 8) + pkt->crc[0];
	
	pkt->crc[0] = 0;
	pkt->crc[1] = 0;
	
	return rxedCrc == commsPrvChecksumPacket(pkt, data, len);
}

static bool commsPrvPacketSendLL(SerialPort sp, struct PokePacket *pkt, const void *data, uint8_t len)
{
	commsPrvChecksumPacketAndRecordSum(pkt, data ,len);
	
	return commsPrvSendPiece(sp, pkt, sizeof(struct PokePacket)) && commsPrvSendPiece(sp, data, len) && commsPrvSendCompleted(sp);;
}

static int16_t commsPrvPacketRxLL(SerialPort sp, struct PokePacket *pkt, void *buf, uint8_t expectedLen)	//negative on error. data size on no error
{
	uint8_t rxBuf[sizeof(struct PokePacket) + POKEWALKER_DATA_MAX_LEN];
	int16_t pos;
	
	if (expectedLen > POKEWALKER_DATA_MAX_LEN)
		return 1;
	
	pos = commsPrvSerialRx(sp, rxBuf, expectedLen + sizeof(struct PokePacket), 50);
	if (pos < (int16_t)sizeof(struct PokePacket)) {
		ErrAlertCustom(0, "not enough data", NULL, NULL);
		return -1;
	}
	pos -= sizeof(struct PokePacket);

	//faster than memmove
	MemMove(pkt, rxBuf, sizeof(struct PokePacket));
	MemMove(buf, rxBuf + sizeof(struct PokePacket), pos);
	
	return pos;
}


//pokewalker comms

static int16_t commsPrvPacketRx(struct Comms *comms, struct PokePacket *pkt, void *buf, uint8_t rxBufSz)
{
	int16_t ret = commsPrvPacketRxLL(comms->sp, pkt, buf, rxBufSz);
	uint_fast8_t i;
	
	if (ret < 0)
		return -1;
	
	for (i = 0; i < sizeof(pkt->session); i++) {
	
		if (pkt->session[i] != comms->session[i]) {
		
			char x[128];
			
			StrPrintF(x, "bad session at byte %u pkt{ %02x %02x %02x %02x} exp {%02x %02x %02x %02x}. 1st byte %02x", i,
				pkt->session[0], pkt->session[1], pkt->session[2], pkt->session[3],
				comms->session[0], comms->session[1], comms->session[2], comms->session[3], pkt->cmd);
		
			ErrAlertCustom(0, x, NULL, NULL);
			return -1;
		}
	}
	
	if (!commsPrvChecksumPacketAndCheck(pkt, buf, ret)) {
		ErrAlertCustom(0, "bad crc", NULL, NULL);
		return -1;
	}
	
	return ret;
}

static bool commsPrvPacketTx(struct Comms *comms, struct PokePacket *pkt, const void *data, uint8_t len)
{
	uint_fast8_t i;
	
	for (i = 0; i < sizeof(pkt->session); i++)
		pkt->session[i] = comms->session[i];
	
	return commsPrvPacketSendLL(comms->sp, pkt, data, len);
}

bool commsPacketRawTx(struct Comms *comms, struct PokePacket *pkt, const void *data, uint8_t len)
{
	return commsPrvPacketTx(comms, pkt, data, len);
}

int16_t commsPacketRawRx(struct Comms *comms, struct PokePacket *pkt, void *buf, uint8_t rxBufSz)
{
	return commsPrvPacketRx(comms, pkt, buf, rxBufSz);
}

struct Comms* commsOpen(void)
{
	struct Comms *comms = MemPtrNew(sizeof(struct Comms));
	uint32_t val32;
	uint16_t len;
	Err e;
	
	if (!comms)
		goto fail_alloc;
	
	MemSet(comms, sizeof(*comms), 0);
	
	if (!commsPrvSerialOpen(&comms->sp))
		goto serial_open_fail;
	
	return comms;

serial_open_fail:
	MemPtrFree(comms);
	
fail_alloc:
	return NULL;
}

void commsClose(struct Comms *comms)
{
	commsPrvSerialClose(comms->sp);
	MemPtrFree(comms);
}

bool commsTryAcceptWalkerConnection(struct Comms *comms)
{
	uint32_t ourSeed = (((uint32_t)SysRandom(0)) << 16) + SysRandom(0);
	struct PokePacket pkt;
	uint8_t adv;
	int16_t i;
	
	if (comms->connected)
		return false;
	
	if (sizeof(adv) != commsPrvSerialRx(comms->sp, &adv, sizeof(adv), 0) || adv != CMD_POKEWALKER_ADV)
		return false;
	
	pkt.cmd = CMD_POKEWALKER_SYN;
	pkt.detail = DETAIL_DIR_TO_WALKER;
	
	for (i = 0; i < 4; i++, ourSeed >>= 8)
		comms->session[i] = ourSeed;
	
	if (!commsPrvPacketTx(comms, &pkt,  NULL, 0))
		return false;
	
	if (commsPrvPacketRxLL(comms->sp, &pkt, NULL, 0) != 0 || pkt.cmd != CMD_POKEWALKER_SYNACK || pkt.detail != DETAIL_DIR_FROM_WALKER || !commsPrvChecksumPacketAndCheck(&pkt, NULL, 0))
		return false;
	
	for (i = 0; i < 4; i++)
		comms->session[i] ^= pkt.session[i];
	
	comms->connected = true;
	
	return true;
}

static bool commsPrvSimplePacketTx(struct Comms *comms, uint32_t cmd)
{
	struct PokePacket pkt;
	
	pkt.cmd = cmd;
	pkt.detail = DETAIL_DIR_TO_WALKER;
	
	return commsPrvPacketTx(comms, &pkt, NULL, 0);
}

static bool commsPrvSimplePacketRx(struct Comms *comms, uint8_t cmd)
{
	struct PokePacket pkt;
	
	return commsPrvPacketRx(comms, &pkt, NULL, 0) == 0 && pkt.cmd == cmd && pkt.detail == DETAIL_DIR_FROM_WALKER;
}

bool commsDisconnect(struct Comms *comms)
{
	if (!comms->connected)
		return false;
	
	comms->connected = false;

	return commsPrvSimplePacketTx(comms, CMD_POKEWALKER_DIS_REQ) && commsPrvSimplePacketRx(comms, CMD_POKEWALKER_DIS_RSP);
}

bool commsEepromRead(struct Comms *comms, void *dstP, uint16_t addr, uint16_t len)
{
	uint8_t *dst = (uint8_t*)dstP;
	struct PokePacket pkt;
	
	if (!comms->connected)
		return false;
	
	while (len) {
		
		uint16_t now = (len > POKEWALKER_DATA_MAX_LEN) ? POKEWALKER_DATA_MAX_LEN : len;
		uint8_t cmd[3] = {addr >> 8, addr, now};
		int16_t got;
		
		pkt.cmd = CMD_EEPROM_READ_REQ;
		pkt.detail = DETAIL_DIR_TO_WALKER;
		
		if (!commsPrvPacketTx(comms, &pkt, cmd, sizeof(cmd)))
			return false;
		
		got = commsPrvPacketRx(comms, &pkt, dst, now);
		
		if (got < 0 || (uint16_t)got != now || pkt.cmd != CMD_EEPROM_READ_RSP || pkt.detail != DETAIL_DIR_FROM_WALKER)
			return false;
		
		dst += now;
		len -= now;
		addr += now;
	}
	
	return true;
}

bool commsEepromWrite(struct Comms *comms, const void *srcP, uint16_t addr, uint16_t len)
{
	const uint8_t *src = (const uint8_t*)srcP;
	uint8_t txBuf[POKEWALKER_DATA_MAX_LEN];
	struct PokePacket pkt;
	
	if (!comms->connected)
		return false;
	
	while (len) {
		uint16_t now = (len > POKEWALKER_DATA_MAX_LEN - 1) ? POKEWALKER_DATA_MAX_LEN - 1 : len;
		
		pkt.cmd = CMD_EEPROM_WRITE_REQ;
		pkt.detail = addr >> 8;
		txBuf[0] = addr;
		
		MemMove(txBuf + 1, src, now);
		
		if (!commsPrvPacketTx(comms, &pkt, txBuf, now + 1))
			return false;
		
		if (0 != commsPrvPacketRx(comms, &pkt, NULL, 0) || pkt.cmd != CMD_EEPROM_WRITE_RSP || pkt.detail != (addr >> 8)) {
			ErrAlertCustom(0, "bad hi level", NULL, NULL);
			
			return false;
		}
		
		src += now;
		len -= now;
		addr += now;
	}
	
	return true;
}

bool commsEventPokeRxed(struct Comms *comms)
{
	if (!comms->connected)
		return false;
	
	comms->connected = false;
	
	return commsPrvSimplePacketTx(comms, CMD_EVENT_POKE_RXED) && commsPrvSimplePacketRx(comms, CMD_EVENT_POKE_RXED);
}

bool commsEventItemRxed(struct Comms *comms)
{
	if (!comms->connected)
		return false;
	
	comms->connected = false;
	
	return commsPrvSimplePacketTx(comms, CMD_EVENT_ITEM_RXED) && commsPrvSimplePacketRx(comms, CMD_EVENT_ITEM_RXED);
}

bool commsEventRouteRxed(struct Comms *comms)
{
	if (!comms->connected)
		return false;
	
	comms->connected = false;
	
	return commsPrvSimplePacketTx(comms, CMD_EVENT_ROUTE_RXED) && commsPrvSimplePacketRx(comms, CMD_EVENT_ROUTE_RXED);
}

bool commsDeviceReset(struct Comms *comms)
{
	uint8_t resetPkt[3] = {0xe0, 0x5b, 0x94};
	struct PokePacket pkt;
	
	pkt.cmd = CMD_WRITE;
	pkt.detail = 0xf7;
	
	if (!comms->connected)
		return false;
	
	comms->connected = false;
	
	return commsPrvPacketTx(comms, &pkt, resetPkt, sizeof(resetPkt));
}





