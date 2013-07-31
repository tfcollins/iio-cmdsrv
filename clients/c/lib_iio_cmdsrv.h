/*
 * lib IIO command server
 *
 * Copyright 2012 Analog Devices Inc.
 *
 * Licensed under the GPL-2.
 */

#ifndef LIB_IIO_CMDSRV_
#define LIB_IIO_CMDSRV_

#include <netinet/in.h>

#define IIO_CMDSRV_MAX_RETVAL	13
#define IIO_CMDSRV_MAX_STRINGVAL 512

struct iio_cmdsrv {
	int sockfd;
	char addr[INET6_ADDRSTRLEN + 1];
	char port[33];
};

int iio_cmdsrv_connect(const char *addr, const char *port,
		struct iio_cmdsrv *handle);

int iio_cmd_send(struct iio_cmdsrv *s, const char *str, ...);

int iio_cmd_read(struct iio_cmdsrv *s, char *rbuf, unsigned rlen,
		const char *str, ...);

int iio_cmd_sample(struct iio_cmdsrv *s, const char *name, char *rbuf,
		unsigned count, unsigned bytes_per_sample);

int iio_cmd_regread(struct iio_cmdsrv *s, char *name, unsigned reg,
		unsigned *val);

int iio_cmd_regwrite(struct iio_cmdsrv *s, char *name, unsigned reg,
		unsigned val);

int iio_cmd_bufwrite(struct iio_cmdsrv *s, const char *name, char *wbuf,
		unsigned count);

#endif /* LIB_IIO_CMDSRV_ */
