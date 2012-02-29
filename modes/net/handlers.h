#ifndef __XBEE_MODE_HANDLERS_H
#define __XBEE_MODE_HANDLERS_H

/*
  libxbee - a C library to aid the use of Digi's XBee wireless modules
            running in API mode (AP=2).

  Copyright (C) 2009  Attie Grande (attie@attie.co.uk)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

xbee_err xbee_net_frontchannel_tx_func(struct xbee *xbee, struct xbee_con *con, void *arg, unsigned char identifier, unsigned char frameId, struct xbee_conAddress *address, struct xbee_conSettings *settings, unsigned char *buf, int len, struct xbee_buf **oBuf);
xbee_err xbee_net_frontchannel_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_buf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt);

extern const struct xbee_modeConType xbee_net_frontchannel_template;
extern const struct xbee_modeConType xbee_net_frontchannel_template_fid;

extern const struct xbee_modeConType xbee_net_backchannel;

#endif /* __XBEE_MODE_HANDLERS_H */
