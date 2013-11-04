#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lib_iio_cmdsrv.h"

#if 1

void check (char * cmd, int ret)
{
	if (ret < 0) {
		printf("when excuting '");
		while(*cmd) {
			if (*cmd == '\n')
				printf("\\n");
			else
				printf("%c", *cmd);
			cmd++;
		}
		printf("'\n");
		if (IS_ERR_LOCAL(ret))
			printf("Local error ret : %d (%s)\n", ret + ERRNO_BASE_LOCAL,
				strerror(abs(ret + ERRNO_BASE_LOCAL)));
		else
			printf("ret : %d (%s)\n", ret, strerror(abs(ret)));
		printf("Exiting with error: %s\n", strerror(errno));
		exit(-1);
	}

	if (ret != 0)
		printf("return code %i\n", ret);
}

static void dump_str(char *string, size_t max)
{
	if (strlen(string) > max)
		printf("'%.*s[[..%d..]]%.*s'\n", max / 2, string,
				strlen (string) - max,
				max / 2, string + strlen (string) - max / 2);
	else
		printf("'%s'\n", string);
}

#define BUFSIZE 400000
#define NAMSIZE 2048

static void ad9643(struct iio_cmdsrv srv)
{
	short *buf2;
	char *buf;
	size_t i, ret;

	buf2 = malloc(BUFSIZE * 2);
	if (!buf2) {
		printf("malloc error\n");
		exit(-1);
	}
	buf = (char *)buf2;

	ret = iio_cmd_read(&srv, buf, 1000, "read cf-ad9643-core-lpc in_voltage_scale_available\n", NULL);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_read(&srv, buf, 200, "read cf-ad9643-core-lpc in_voltage_scale\n", NULL);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_send(&srv, "write cf-ad9643-core-lpc in_voltage_scale %f\n", 0.025024);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_read(&srv, buf, 200, "read cf-ad9643-core-lpc in_voltage_scale\n", NULL);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_send(&srv, "write cf-ad9643-core-lpc in_voltage_scale %f\n", 0.030060);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_read(&srv, buf, 200, "read cf-ad9643-core-lpc in_voltage_scale\n", NULL);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_regread(&srv, "cf-ad9643-core-lpc", 1, &i);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regread(&srv, "cf-ad9643-core-lpc", 2, &i);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regread(&srv, "cf-ad9643-core-lpc", 1, &i);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regwrite(&srv, "cf-ad9643-core-lpc", 0x20, 0xAB);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regread(&srv, "cf-ad9643-core-lpc", 0x20, &i);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regwrite(&srv, "cf-ad9643-core-lpc", 0x20, 0);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regread(&srv, "cf-ad9643-core-lpc", 0x20, &i);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_bufwrite(&srv, "cf-ad9643-core-lpc", buf, 400000);
	printf("main:retval (ret %d)\n", ret);
}

static int network_test(const char *ipnum, const char *port, const char protocol)
{

	struct iio_cmdsrv srv;
	int ret;
	size_t i;
	short *buf2;
	char *data;
	char *tmp, *tmp1, *tmp2, *tmp3, *buf;
	char *command, *device;

	buf2 = malloc(BUFSIZE * 2);
	tmp1 = malloc(NAMSIZE);
	buf = malloc(NAMSIZE);
	command = malloc(256);
	device = malloc(256);

	if(!buf2 || !tmp1 || !buf || !command || !device) {
		printf("malloc error\n");
		exit(-1);
	}
	data = (char *)buf2;

	printf("trying to connect to %s:%s over ", ipnum, port);
	if (protocol == TCP)
		printf("TCP\n");
	else if (protocol == UDP)
		printf("UDP\n");
	else
		printf("Unknown protocol\n");

	ret = iio_cmdsrv_connect(ipnum, port, protocol, &srv);
	if (ret) {
		perror("connection failed");
		exit(-1);
	}
	printf("Connected to %s %s\n", srv.addr, srv.port);

	sprintf(command, "version\n");
	ret = iio_cmd_read(&srv, buf, NAMSIZE - 1, command, NULL);
	check(command, ret);
	printf("version returned '%s'\n", buf);

	sprintf(command, "show\n");
	ret = iio_cmd_read(&srv, buf, NAMSIZE - 1, command, NULL);
	check(command, ret);
	printf("devices installed: '%s'\n", buf);

	tmp = buf;
	while (*tmp) {
		printf("\n");

		tmp2 = strchr(tmp, ' ');
		if (!tmp2)
			tmp2 = tmp + strlen(tmp);

		sprintf(device, "%.*s", (int)(tmp2 - tmp), tmp);

		sprintf(command, "read %s name\n", device);
		ret = iio_cmd_read(&srv, tmp1, NAMSIZE - 1, command, NULL);
		check(command, ret);
		printf("name of '%s' is '%s'\n", device, tmp1);

		sprintf(command, "show %s .\n", device);
		ret = iio_cmd_read(&srv, tmp1, NAMSIZE - 1, command, NULL);
		check(command, ret);
		printf("attributes of '%s' are ", device);
		dump_str(tmp1, 60);

		tmp3 = strstr(tmp1, "_test_mode");
		if (tmp3) {
			printf("in '%s', test mode exists\n", device);
			ret = iio_cmd_sample(&srv, device, data, 40, 4);
			printf("main:retval (ret %d)\n", ret);
			for (i = 0; i < 40; i++)
				printf("[%d] = %d\n", i, data[i]);
		}

		if (!strcmp(device, "cf-ad9643-core-lpc"))
			ad9643(srv);


		sprintf(command, "register 0x0 read of %s", device);
		ret = iio_cmd_regread(&srv, device, 0x00, &i);
		if (ret != -111) {
			check(command, ret);
			printf("register 0 = %x\n", i);
		}


		tmp = tmp2 + 1;

	}
	iio_cmdsrv_disconnect(&srv);

	free(buf2);
	free(tmp1);
	free(buf);
	free(command);
	free(device);

	return 0;
}

int main (int argc, const char* argv[])
{
	if (argc == 1) {
		printf("No IP number\n");
		exit(-1);
	}

	network_test(argv[1], "1234", TCP);
	network_test(argv[1], "1236", UDP);
	return 0;

}
#endif
