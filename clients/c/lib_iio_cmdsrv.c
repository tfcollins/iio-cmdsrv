/*
 * lib IIO command server
 *
 * Copyright 2012 Analog Devices Inc.
 *
 * Licensed under the GPL-2.
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <linux/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <syslog.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <termios.h>
#include <math.h>
#include <time.h>
#include <netdb.h>
#include <stdarg.h>

#include "lib_iio_cmdsrv.h"

// get sockaddr, IPv4 or IPv6:
static void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void iio_cmdsrv_disconnect(struct iio_cmdsrv *handle)
{
	if (handle && handle->sockfd != -1) {
		iio_cmd_send(handle, "quit\n");
		close(handle->sockfd);
		handle->sockfd = -1;
	}
}

int iio_cmdsrv_connect(const char *addr, const char *port, const char protocol, struct iio_cmdsrv *handle)
{
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	struct timeval timeout;
	struct gaicb *in[1];

	if (addr && port) {
		strcpy(handle->addr, (char *)addr);
		strcpy(handle->port, (char *)port);
		handle->sockfd = -1;
	} else {
		close(handle->sockfd);
	}

	memset(&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_INET;
	if(protocol == TCP) {
	hints.ai_socktype = SOCK_STREAM;
	} else if(protocol == UDP) {	
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
	}

	in[0] = malloc(sizeof(*in[0]));
	memset(in[0], 0, sizeof (*in[0]));
	in[0]->ar_name = handle->addr;
	in[0]->ar_service = handle->port;
	in[0]->ar_request = &hints;

	/* Get ready to make this time out */
	rv = getaddrinfo_a(GAI_WAIT, in, 1, NULL);
	if (rv != 0) {
		syslog(LOG_ERR, "%s: getaddrinfo_a failed\n", __func__);
		return 1;
	}
	servinfo = in[0]->ar_result;

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		handle->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (handle->sockfd == -1) {
			syslog(LOG_ERR, "%s: socket failed\n", __func__);
			continue;
		}

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		setsockopt(handle->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
		setsockopt(handle->sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));

		if (connect(handle->sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(handle->sockfd);
			handle->sockfd = -1;
			syslog(LOG_ERR, "%s: connection failed\n", __func__);
			continue;
		}

		break;
	}

	if (p == NULL) {
		syslog(LOG_ERR, "%s: connection failed\n", __func__);
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
		s, sizeof s);

	freeaddrinfo(servinfo);

	return 0;
}

static int srv_receive(struct iio_cmdsrv *s, char *rbuf, unsigned rlen,
			char *rbuf2, unsigned *rlen2, unsigned is_str)
{
	int rx_len, term = 1, retry = 1;
	unsigned i = 0, len = 0;

	do {
		rx_len = recv(s->sockfd, &rbuf[len], rlen - len, 0);

		if (rx_len == -1) {
			if (errno == EAGAIN) {
				if (retry--)
					continue;
			}
			return -errno;
		}
		len += rx_len;

		if (is_str) {
			for (i = 0; i < len; i++) {
				if (rbuf[i] == '\n') {
					term = 0;
					rbuf[i] = 0;
					break;
				}
			}
		}
		/* The return value will be 0 when the peer has performed an orderly shutdown. */
		if (rx_len == 0)
			term = 0;
	} while ((len < rlen) && term);


	if (rbuf2 && rlen2)
		for (++i; i < len; i++) {
			rbuf2[(*rlen2)++] = rbuf[i];
		}

	if(rbuf2 && rlen2 && (*rlen2 == 0)) {
		while(rbuf[*rlen2]) {
			rbuf2[*rlen2] = rbuf[*rlen2];
			(*rlen2)++;
		}
		rbuf2[*rlen2] = 0;
		(*rlen2)++;
	}

	return term;
}

static int iio_cmd_send_va(struct iio_cmdsrv *s, const char *str, va_list args)
{
	char buf[IIO_CMDSRV_MAX_STRINGVAL];
	char retval[IIO_CMDSRV_MAX_RETVAL + 1];
	int len, ret, ret_target;

	if (s->sockfd < 0) {
		syslog(LOG_ERR, "%s: not connected\n", __func__);
		return -1;
	}

	len = vsprintf(buf, str, args);

	if (len < 0)
		syslog(LOG_ERR, "%s: vsprintf\n", __func__);

	fprintf(stderr, "iio_cmd_send_va: %s\n", buf);

	ret = send(s->sockfd, buf, len, 0);
	if (ret < 0)
		syslog(LOG_ERR, "%s: send failed (%d)\n", __func__, ret);

	ret = srv_receive(s, retval, IIO_CMDSRV_MAX_RETVAL, NULL, NULL, 1);
	if (ret >= 0)
		if (sscanf(retval, "%d\n", &ret_target) == 1)
			return ret_target;

	return ret;
}

int iio_cmd_send(struct iio_cmdsrv *s, const char *str, ...)
{
	va_list args;
	int ret;

	va_start(args, str);
	ret = iio_cmd_send_va(s, str, args);
	if (ret < 0) {
		iio_cmdsrv_connect(NULL, NULL, -1, s);
		ret = iio_cmd_send_va(s, str, args);
	}
	va_end(args);

	return ret;
}


static int iio_cmd_read_va(struct iio_cmdsrv *s, char *rbuf, unsigned rlen,
		const char *str, va_list args)
{
	char buf[IIO_CMDSRV_MAX_STRINGVAL];
	char retval[IIO_CMDSRV_MAX_RETVAL + 1];
	int len, ret;
	unsigned rx_len = 0;

	/* Make sure things are null terminated */
//	bzero(buf, IIO_CMDSRV_MAX_STRINGVAL);
//	bzero(rbuf, rlen);

	if (s->sockfd < 0) {
		syslog(LOG_ERR, "%s: not connected\n", __func__);
		return -1;
	}

	len = vsprintf(buf, str, args);
	if (len < 0)
		perror("iio_cmd_send");

	ret = send(s->sockfd, buf, len, MSG_DONTWAIT);
	if (ret < 0) {
		perror("iio_cmd_send:send");
		return ret;
	}

	ret = srv_receive(s, retval, IIO_CMDSRV_MAX_RETVAL, rbuf, &rx_len, 1);

	if ((ret >= 0) && (sscanf(retval, "%d\n", &ret) == 1)) {
		if (ret >= 0) {
			/* Already received the entire response ? */
			if (rbuf[rx_len - 1] == 0 || rbuf[rx_len - 1] == '\n') {
				rbuf[rx_len - 1] = 0;
				return ret;
			}
			ret = srv_receive(s, &rbuf[rx_len], rlen - rx_len, NULL, NULL, 1);
		} else {
			return ret;
		}
	}

	return ret;
}

int iio_cmd_read(struct iio_cmdsrv *s, char *rbuf, unsigned rlen,
		 const char *str, ...)
{
	va_list args;
	int ret;

	va_start(args, str);
	ret = iio_cmd_read_va(s, rbuf, rlen, str, args);
	if (ret < 0) {
		iio_cmdsrv_connect(NULL, NULL, -1, s);
		ret = iio_cmd_read_va(s, rbuf, rlen, str, args);
	}
	va_end(args);

	return ret;
}

int iio_cmd_regread(struct iio_cmdsrv *s, char *name, unsigned reg, unsigned *val)
{
	char buf[IIO_CMDSRV_MAX_STRINGVAL];
	int ret;

	ret = iio_cmd_read(s, buf, IIO_CMDSRV_MAX_STRINGVAL, "regread %s %d\n", name, reg);
	if (ret >= 0)
		if (sscanf(buf, "%i\n", val) == 1)
			return 0;
		else
			return -EINVAL;
	else
		return ret;
}

int iio_cmd_regwrite(struct iio_cmdsrv *s, char *name, unsigned reg, unsigned val)
{
	return iio_cmd_send(s, "regwrite %s %d %d\n", name, reg, val);
}

int iio_cmd_sample(struct iio_cmdsrv *s, const char *name,char *rbuf,
		unsigned count, unsigned bytes_per_sample)
{
	char buf[IIO_CMDSRV_MAX_STRINGVAL];
	int len = 0, ret, retval;
	unsigned rlen = 0;

	if (s->sockfd < 0) {
		syslog(LOG_ERR, "%s: not connected\n", __func__);
		return -1;
	}

	len = sprintf(buf, "sample %s %d %d\n", name, count, bytes_per_sample);
	if (len < 0)
		perror("iio_cmd_send");

	ret = send(s->sockfd, buf, len, 0);
	if (ret < 0) {
		perror("iio_cmd_send:send");
		return ret;
	}

	ret = srv_receive(s, buf, IIO_CMDSRV_MAX_RETVAL, rbuf, &rlen, 1);

	if ((ret >= 0) && (sscanf(buf, "%d\n", &retval) == 1)) {
		if (retval >= 0) {
			ret = srv_receive(s, &rbuf[rlen], (retval), NULL, NULL, 0);
			if (ret >= 0)
				ret = retval;
		} else {
			return ret;
		}
	}

	return ret;
}

int iio_cmd_bufwrite(struct iio_cmdsrv *s, const char *name, char *wbuf,
		unsigned count)
{
	char buf[IIO_CMDSRV_MAX_STRINGVAL];
	int ret;
	unsigned int len;


	if (s->sockfd < 0) {
		syslog(LOG_ERR, "%s: not connected\n", __func__);
		return -1;
	}

	ret = sprintf(buf, "bufwrite %s %d\n", name, count);
	if (ret < 0)
		perror("iio_cmd_send");

	len = ret;
	ret = send(s->sockfd, buf, len, 0);
	if (ret < 0) {
		perror("iio_cmd_send:send");
		return ret;
	}

	len = 0;
	do {
		ret = send(s->sockfd, wbuf + len, count - len, 0);
		if (ret > 0) {
			len += ret;
		} else if (errno == EAGAIN) {
			continue;
		} else {
			return -errno;
		}
	} while (len < count);

	return ret;
}
