/*
	libxbee - a C library to aid the use of Digi's XBee wireless modules
	          running in API mode.

	Copyright (C) 2009 onwards  Attie Grande (attie@attie.co.uk)

	libxbee is free software: you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	libxbee is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../../internal.h"
#include "../../xbee_int.h"
#include "../../log.h"
#include "../../mode.h"
#include "../../frame.h"
#include "../../pkt.h"
#include "../common.h"
#include "mode.h"
#include "at.h"
#include "data.h"
#include "dataExp.h"
#include "io.h"
#include "sensor.h"
#include "identify.h"
#include "ota.h"
#include "zdo.h"

static xbee_err init(struct xbee *xbee, va_list ap);
static xbee_err mode_shutdown(struct xbee *xbee);

/* ######################################################################### */

static xbee_err init(struct xbee *xbee, va_list ap) {
	xbee_err ret;
	char *t;
	struct xbee_modeData *data;
	
	if (!xbee) return XBEE_EMISSINGPARAM;
	
	if ((data = malloc(sizeof(*data))) == NULL) return XBEE_ENOMEM;
	memset(data, 0, sizeof(*data));
	xbee->modeData = data;
	
	ret = XBEE_ENONE;
	
	/* currently I don't see a better way than this - using va_arg()... which is gross */	
	t = va_arg(ap, char*);
	if ((data->serialInfo.device = malloc(strlen(t) + 1)) == NULL) { ret = XBEE_ENOMEM; goto die; }
	strcpy(data->serialInfo.device, t);
	
	data->serialInfo.baudrate = va_arg(ap, int);
	
	if ((ret = xsys_serialSetup(&data->serialInfo)) != XBEE_ENONE) goto die;
	
	if ((ret = xbee_sZB_zdo_init(xbee)) != XBEE_ENONE) goto die;
	
	return XBEE_ENONE;
die:
	mode_shutdown(xbee);
	return ret;
}

static xbee_err mode_shutdown(struct xbee *xbee) {
	struct xbee_modeData *data;
	
	if (!xbee) return XBEE_EMISSINGPARAM;
	if (!xbee->mode || !xbee->modeData) return XBEE_EINVAL;
	
	data = xbee->modeData;
	
	xbee_sZB_zdo_shutdown(xbee);
	
	xsys_serialShutdown(&data->serialInfo);
	if (data->serialInfo.device) free(data->serialInfo.device);
	if (data->serialInfo.txBuf) free(data->serialInfo.txBuf);
	free(xbee->modeData);
	xbee->modeData = NULL;
	
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_sZB_transmitStatus_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_buf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	xbee_err ret;
	
	if (!xbee || !frameInfo || !buf || !address || !pkt) return XBEE_EMISSINGPARAM;
	
	ret = XBEE_ENONE;
	
	if (buf->len != 7) {
		ret = XBEE_ELENGTH;
		goto die1;
	}
	
#warning TODO - currently missing out on the resolved network address, retry count, and discovery status
	frameInfo->active = 1;
	frameInfo->id = buf->data[1];
	frameInfo->retVal = buf->data[5];
	
	goto done;
die1:
done:
	return ret;
}

/* ######################################################################### */

struct xbee_modeDataHandlerRx xbee_sZB_transmitStatus_rx  = {
	.identifier = 0x8B,
	.func = xbee_sZB_transmitStatus_rx_func,
};
struct xbee_modeConType xbee_sZB_transmitStatus = {
	.name = "Transmit Status",
	.allowFrameId = 1,
	.useTimeout = 0,
	.addressRules = ADDR_NONE,
	.rxHandler = &xbee_sZB_transmitStatus_rx,
	.txHandler = NULL,
};

/* ######################################################################### */

xbee_err xbee_sZB_modemStatus_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_buf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	struct xbee_pkt *iPkt;
	xbee_err ret;

	if (!xbee || !frameInfo || !buf || !address || !pkt) return XBEE_EMISSINGPARAM;
	
	if (buf->len != 2) return XBEE_ELENGTH;
	
	if ((ret = xbee_pktAlloc(&iPkt, NULL, 1)) != XBEE_ENONE) return ret;
	
	iPkt->dataLen = 1;
	iPkt->data[0] = buf->data[1];
	iPkt->data[iPkt->dataLen] = '\0';
	
	*pkt = iPkt;
	
	return 0;
}

/* ######################################################################### */

struct xbee_modeDataHandlerRx xbee_sZB_modemStatus_rx  = {
	.identifier = 0x8A,
	.func = xbee_sZB_modemStatus_rx_func,
};
struct xbee_modeConType xbee_sZB_modemStatus = {
	.name = "Modem Status",
	.allowFrameId = 0,
	.useTimeout = 0,
	.addressRules = ADDR_NONE,
	.rxHandler = &xbee_sZB_modemStatus_rx,
	.txHandler = NULL,
};

/* ######################################################################### */

static const struct xbee_modeConType *conTypes[] = {
	&xbee_sZB_transmitStatus,
	&xbee_sZB_modemStatus,
	&xbee_sZB_localAt,
	&xbee_sZB_remoteAt,
	&xbee_sZB_data,
	&xbee_sZB_dataExp,
	&xbee_sZB_io,
	&xbee_sZB_sensor,
	&xbee_sZB_identify,
	&xbee_sZB_ota,
	NULL
};

const struct xbee_mode mode_xbeeZB = {
	.name = "xbeeZB",
	
	.conTypes = conTypes,
	
	.init = init,
	.prepare = NULL,
	.shutdown = mode_shutdown,
	
	.rx_io = xbee_xbeeRxIo,
	.tx_io = xbee_xbeeTxIo,
	
	.thread = NULL,
};
